#include "controller.h"
#include "initprogram.h"
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include "../model/database.h"
#include "../ui/searchbar.h"
#include <QSettings>
#include <QJsonObject>
#include <QFile>
#include <QMessageBox>
#include <QJsonParseError>
#include <windows.h>
#include <shlobj.h>
#include <QApplication>
#include "../controller/utils.h"
#include "../ui/settingwindow.h"
#include "../model/chineseconvertpinyin.h"

Controller::Controller() {
    SearchBar& searchBar = SearchBar::getInstance();
    ResultFrame& resultFrame = ResultFrame::getInstance();
    SettingWindow& settingWindow = SettingWindow::getInstance();

    QObject::connect(&settingWindow, &SettingWindow::confirmSetting, [this](SettingWindowConfigure configure){
        this->saveSetting(configure);
    });

    QObject::connect(&searchBar, &QLineEdit::textChanged, [this](const QString& inputText){
        this->inputText(inputText);
    });

    QObject::connect(&searchBar, &SearchBar::launchSelectedProgram, [this](){
        this->launchSelectedProgram();
    });

}

void Controller::loadConfigure()
{
    QString filePath = getConfigureFilePath();
    // 检查文件是否存在
    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << "File does not exist.";
        initConfigureFile();

    }

    // 打开文件
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open the file.";
        QMessageBox::critical(nullptr, "检测到错误", (QString)"请打开程序目录，删除configure文件后重新运行本程序, error:" + file.errorString());
        exit(1);
    }

    // 读取文件内容
    QByteArray fileContent = file.readAll();
    file.close();

    // 解析JSON
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileContent, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "Invalid JSON file:" << parseError.errorString();
        initConfigureFile();
        jsonDoc = QJsonDocument::fromJson(fileContent, &parseError);
    }
    // 正确的解析了文件，所以可以赋值
    configure = jsonDoc.object();
}

void Controller::initConfigureFile()
{
    QJsonObject jsonObject;

    jsonObject =  getDefaultSettingJson();

    QJsonDocument jsonDocument(jsonObject);

    QString filePath = getConfigureFilePath();
    QFile file(filePath);
    // 打开文件以进行写入
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::critical(nullptr, "检测到错误", (QString)"请打开程序目录，删除configure文件后重新运行本程序, error:" + file.errorString());
        exit(1);
    }
    // 将JSON文档写入文件
    file.write(jsonDocument.toJson());
    // 关闭文件
    file.close();
}

bool Controller::startProcessWithElevation(const std::wstring &programPath, const std::wstring &workingDirectory)
{
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = L"runas"; // Request elevation
    sei.lpFile = programPath.c_str();
    sei.lpDirectory = workingDirectory.c_str();
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&sei)) {
        DWORD error = GetLastError();
        if (error == ERROR_CANCELLED) {
            qDebug() << "User declined the elevation request.";
        } else {
            qDebug() << "Failed to start process with elevation. Error: " << error;
        }
        return false;
    }
    return true;
}

bool Controller::startProcessNormally(const std::wstring &programPath, const std::wstring &workingDirectory)
{
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = nullptr; // Use default verb
    sei.lpFile = programPath.c_str();
    sei.lpDirectory = workingDirectory.c_str();
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&sei)) {
        DWORD error = GetLastError();
        qDebug() << "Failed to start process normally. Error: " << error;
        return false;
    }
    return true;
}

void Controller::init()
{
    loadConfigure();
    //根据加载的配置来初始化程序

    InitProgram& init = InitProgram::getInstance();

    SettingWindowConfigure classConfig = buildClassWithJson(configure);

    uiController.initUI(classConfig);

    ChineseConvertPinyin& pinyin = ChineseConvertPinyin::getInstance();
    pinyin.init();

    init.clearStore();

    init.initCustomPath();
    if (configure["searchStartMenu"].toBool()) {
        init.initProgramWithStartMenu();
    }
    if (configure["searchRegistry"].toBool()) {
        init.initProgramWithRegistery();
    }
    if (configure["searchProgramFile"].toBool()) {
        init.initProgramWithProgramFileDir();
    }
    if (configure["preLoadResource"].toBool()) {
        uiController.preLoadProgramIcon();
    }

    setAutoStart(configure["isAutoStart"].toBool());

}

void Controller::launchSelectedProgram()
{
    ResultFrame& resultFrame = ResultFrame::getInstance();
    int index = resultFrame.getCurrentItemIndex();

    uiController.hide();
    runProgramWithIndex(index);
}

void Controller::runProgramWithIndex(int index) {
    Database& db = Database::getInstance();
    auto& programsFile = db.getProgramsFile();

    const std::wstring& programPath = programsFile[index].programPath;
    db.addLaunchTime(index);
    // Log the program path to ensure it is correct
    qDebug() << "Attempting to run program: " << QString::fromStdWString(programPath);
    // Extract the directory part of the path using std::filesystem
    std::filesystem::path path(programPath);
    std::wstring workingDirectory = path.parent_path().wstring();

    // Try to start the process normally
    if (!startProcessNormally(programPath, workingDirectory)) {
        qDebug() << "Attempting to run process with elevation.";
        if (!startProcessWithElevation(programPath, workingDirectory)) {
            qDebug() << "Failed to start process even with elevation.";
        }
    }

    /*
    // 以下是使用cmd.exe启动的代码，该代码作为启动程序没有问题
    Database& db = Database::getInstance();
    auto& programsFile = db.getProgramsFile();

    const std::wstring& programPath = programsFile[index].programPath;
    db.addLaunchTime(index);

    // Prepare the command line to run the program using cmd.exe
    std::wstring commandLine = L"cmd.exe /c start \"\" \"" + programPath + L"\"";

    // Log the command line to ensure it is correct
    qDebug() << "Attempting to run command: " << commandLine;

    // Prepare the STARTUPINFO structure
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create a writable copy of the command line
    std::vector<wchar_t> commandLineBuffer(commandLine.begin(), commandLine.end());
    commandLineBuffer.push_back(0);

    // Extract the directory part of the path using std::filesystem
    std::filesystem::path path(programPath);
    std::wstring workingDirectory = path.parent_path().wstring();

    // Create the process
    if (!CreateProcess(
            NULL,           // Module name (use command line directly)
            commandLineBuffer.data(), // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            CREATE_NO_WINDOW | DETACHED_PROCESS, // No console window, detached process
            NULL,           // Use parent's environment block
            workingDirectory.c_str(),           // Use parent's starting directory
            &si,            // Pointer to STARTUPINFO structure
            &pi)            // Pointer to PROCESS_INFORMATION structure
        ) {
        qDebug() << "Failed to create process. Error: " << GetLastError();
    } else {
        // Close process and thread handles to ensure they are completely detached
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        qDebug() << "Process created successfully: " << commandLine;
    }
    */
}

void Controller::inputText(const QString &text)
{
    if (!text.isEmpty()) {
        Database& db = Database::getInstance();
        db.updateProgramInfo(text.toStdWString());
    }
    uiController.updateResultFrame(text.isEmpty());
}
void Controller::setAutoStart(bool isAutoStart)
{
    QString  strApplicationName = QApplication::applicationName();//获取应用名称
    QSettings * settings = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    if(isAutoStart)
    {
        QString strApplicationFilePath = QApplication::applicationFilePath();//获取应用的目录
        settings->setValue(strApplicationName, strApplicationFilePath.replace("/", "\\"));//写入注册表
    }
    else
        settings->remove(strApplicationName);//移除注册表
}

const QJsonObject &Controller::getConfigure()
{
    return configure;
}

void Controller::saveSetting(SettingWindowConfigure configure)
{
    QJsonObject json;

    json = buildJsonWithClass(configure);

    QJsonDocument jsonDocument(json);

    QString filePath = getConfigureFilePath();

    QFile file(filePath);
    // 打开文件以进行写入
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::critical(nullptr, "检测到错误", "请打开程序目录，删除configure文件后重新运行本程序");
        exit(1);
    }
    // 将JSON文档写入文件
    file.write(jsonDocument.toJson());
    // 关闭文件
    file.close();

    // 保存好后，重新加载配置
    init();
}


