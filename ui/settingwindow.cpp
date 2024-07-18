#include "settingwindow.h"
#include "ui_settingwindow.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QJsonObject>
#include "../controller/utils.h"
#include <QCloseEvent>

SettingWindow::SettingWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("设置");
    // 设置窗口无法更改大小
    this->setFixedSize(this->size());

}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::initWindow(const SettingWindowConfigure &config)
{

    ui->boxSearchStartMenu->setChecked(config.isSearchStartMenu);
    ui->boxAutoStart->setChecked(config.isAutoStart);
    ui->boxPreLoadResource->setChecked(config.isPreLoadResource);
    ui->boxSearchProgramFile->setChecked(config.isSearchProgramFile);
    ui->boxSearchRegistry->setChecked(config.isSearchRegistry);
    ui->spResultItemNumber->setValue(config.resultItemNumber);
    ui->LEPlaceholderText->setText(config.searchBarPlaceholderText);
    ui->LEEmptyText->setText(config.resultFrameEmptyText);
    ui->boxSearchUWP->setChecked(config.isSearchUWP);
    ui->boxIgnoreUninstall->setChecked(config.isIgnoreUninstallApp);
}

void SettingWindow::show()
{
    QWidget::show();
    raise();
    activateWindow();
    setFocus();
}

void SettingWindow::closeEvent(QCloseEvent *event)
{
    this->hide();
    event->ignore();  // 忽略关闭事件
}

void SettingWindow::on_btnConfirm_clicked()
{
    SettingWindowConfigure configure;

    configure.isAutoStart = ui->boxAutoStart->isChecked();
    configure.isSearchStartMenu = ui->boxSearchStartMenu->isChecked();
    configure.isSearchRegistry = ui->boxSearchRegistry->isChecked();
    configure.isSearchProgramFile = ui->boxSearchProgramFile->isChecked();
    configure.isPreLoadResource = ui->boxPreLoadResource->isChecked();
    configure.resultItemNumber = ui->spResultItemNumber->value();
    configure.searchBarPlaceholderText = ui->LEPlaceholderText->text();
    configure.resultFrameEmptyText = ui->LEEmptyText->text();
    configure.isSearchUWP = ui->boxSearchUWP->isChecked();
    configure.isIgnoreUninstallApp = ui->boxIgnoreUninstall->isChecked();
    // 保存文件
    emit confirmSetting(configure);
    hide();
}

void SettingWindow::on_btnCustomDir_clicked()
{
    QString filePath = getCustomDirectoryPath();

    // 使用QDesktopServices打开该文件
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))) {
        QMessageBox::warning(nullptr, "Error", "Cannot open file with default application.");
    }

}


void SettingWindow::on_btnBannedDir_clicked()
{
    QString filePath = getBannedDirectoryPath();

    // 使用QDesktopServices打开该文件
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))) {
        QMessageBox::warning(nullptr, "Error", "Cannot open file with default application.");
    }

}

