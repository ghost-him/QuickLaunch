#include "initprogram.h"

#include <QString>
#include <QSettings>
#include <tchar.h>
#include <shlobj.h>
#include <QStandardPaths>
#include <iostream>
#include "../model/database.h"
#include <QApplication>
#include <QFile>
#include "utils.h"
#include "uwpapp.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>


InitProgram::InitProgram() {}

void InitProgram::initProgramWithRegistery()
{
    auto a = initTargetRegistery(HKEY_LOCAL_MACHINE);
    auto b = initTargetRegistery(HKEY_CURRENT_USER);

    for (auto& i : a) {
        initTargetDirectory(i, 1, 10);
    }
    for (auto& i : b) {
        initTargetDirectory(i, 1, 10);
    }
}

void InitProgram::initProgramWithStartMenu()
{
    QString commonStartMenuPath = GetShellDirectory(CSIDL_COMMON_STARTMENU);
    QString userStartMenuPath = GetShellDirectory(CSIDL_STARTMENU);

    qDebug() << commonStartMenuPath;
    qDebug() << userStartMenuPath;

    initTargetDirectory(commonStartMenuPath.toStdWString(), -1, 0);
    initTargetDirectory(userStartMenuPath.toStdWString(), -1, 0);
}

void InitProgram::initProgramWithProgramFileDir()
{
    QString programFilesPath64 = getDefaultProgramFilesPath(true);
    if (!programFilesPath64.isEmpty()) {
        initTargetDirectory(programFilesPath64.toStdWString(), 1, 0);
    }
    QString programFilesPath32 = getDefaultProgramFilesPath(false);
    if (!programFilesPath32.isEmpty()) {
        initTargetDirectory(programFilesPath32.toStdWString(), 1, 0);
    }
}

void InitProgram::initUWPProgram()
{
    UWPAppManager& app = UWPAppManager::getInstance();
    Database& db = Database::getInstance();
    auto uwps = app.getUWPApp();

    for (auto& uwp : uwps) {
        db.insertProgramInfo(uwp.name, uwp.appID, uwp.picturePath, 0, true);
    }
}

void InitProgram::initCustomPath(const std::vector<QString>& bannedPaths, const std::vector<QString>& searchPaths)
{
    // 先存储不遍历的路径
    for (const auto & i : bannedPaths) {
        QFileInfo info(i);
        if (info.exists()) {
            std::wstring wline = i.toStdWString();
            _bannedPaths.push_back(wline);
        }
    }

    for (const auto& i : searchPaths) {
        QFileInfo info(i);
        if (info.exists()) {
            std::wstring wline = i.toStdWString();
            initTargetDirectory(wline, 1, 0);
        }
    }
}

void InitProgram::clearStore()
{
    Database& db = Database::getInstance();
    db.clearProgramInfo();
    _bannedPaths.clear();
}

void InitProgram::initTargetDirectory(const std::wstring &path, unsigned int depth, int level_bias)
{
    static Database& db = Database::getInstance();
    try {
        std::function<void(const fs::path&, unsigned int)> traverse = [&](const fs::path& currentPath, unsigned int currentDepth) {
            if (currentDepth > depth) return;
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                if (isValidPath(entry) && entry.is_regular_file() && isValidFile(entry)) {
                    // 获得该程序的等级
                    int level = level_bias;
                    // 获得该程序的名字
                    std::wstring programName = entry.path().stem();
                    // 获取该程序的目录
                    std::wstring programPath = entry.path().wstring();

                    db.insertProgramInfo(programName, programPath, programPath, level, false);
                } else if (entry.is_directory()) {
                    traverse(entry.path(), currentDepth + 1);
                }
            }
        };
        traverse(path, 0);
    }catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "General error: " << e.what() << std::endl;
    }
}

std::unordered_set<std::wstring> InitProgram::initTargetRegistery(HKEY hKeyRoot)
{
        HKEY hKey;
        TCHAR subKey[MAX_PATH];
        DWORD subKeySize = MAX_PATH;
        DWORD index = 0;
        std::unordered_set<std::wstring> ret;
        if (RegOpenKeyEx(hKeyRoot, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            while (RegEnumKeyEx(hKey, index, subKey, &subKeySize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                HKEY hSubKey;
                if (RegOpenKeyEx(hKey, subKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    TCHAR installLocation[MAX_PATH];
                    DWORD installLocationSize = sizeof(installLocation);

                    if (RegQueryValueEx(hSubKey, _T("InstallLocation"), NULL, NULL, (LPBYTE)installLocation, &installLocationSize) == ERROR_SUCCESS) {
                        ret.insert(std::wstring(installLocation));
                    }
                    RegCloseKey(hSubKey);
                }
                subKeySize = MAX_PATH;
                index++;
            }
            RegCloseKey(hKey);
        }
        return ret;
}

bool InitProgram::isValidFile(const std::filesystem::path &path)
{
    if (path.extension() == ".lnk") return true;
    if (path.extension() == ".exe") return true;
    if (path.extension() == ".url") return true;
    return false;
}

bool InitProgram::isValidPath(const fs::directory_entry& entry) {
    for (const auto& i : _bannedPaths) {
        if (starts_with_directory(entry, i))
            return false;
    }
    return true;
}

QString InitProgram::getDefaultProgramFilesPath(bool is64Bit)
{
    TCHAR path[MAX_PATH];
    if (is64Bit) {
        // 获取 64 位程序文件路径
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, path))) {
            return QString::fromWCharArray(path);
        }
    } else {
        // 获取 32 位程序文件路径
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, path))) {
            return QString::fromWCharArray(path);
        }
    }
    return QString();
}

QString InitProgram::getSystemStartMenuPath()
{
    TCHAR path[MAX_PATH];
    if (SHGetSpecialFolderPath(NULL, path, CSIDL_COMMON_PROGRAMS, FALSE)) {
        return QString::fromWCharArray(path);
    }
    return QString();
}

QString InitProgram::getUserStartMenuPath()
{
    TCHAR path[MAX_PATH];
    if (SHGetSpecialFolderPath(NULL, path, CSIDL_PROGRAMS, FALSE)) {
        return QString::fromWCharArray(path);
    }
    return QString();
}

QString InitProgram::getUserHomePath()
{
    wchar_t userPath[MAX_PATH];
    // 获取用户目录路径
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, userPath))) {
        // 将 wchar_t* 转换为 QString 并返回
        return QString::fromWCharArray(userPath);
    } else {
        // 返回空字符串表示获取失败
        return QString();
    }
}

bool InitProgram::starts_with_directory(const fs::directory_entry& entry, const std::wstring& dir) {
    fs::path entry_path = entry.path();
    fs::path dir_path = fs::path(dir);

    // Convert both paths to absolute paths
    entry_path = fs::absolute(entry_path);
    dir_path = fs::absolute(dir_path);

    // Check if the entry path starts with the directory path
    auto entry_it = entry_path.begin();
    auto dir_it = dir_path.begin();

    while (dir_it != dir_path.end() && entry_it != entry_path.end() && *dir_it == *entry_it) {
        ++dir_it;
        ++entry_it;
    }

    return dir_it == dir_path.end();
}

