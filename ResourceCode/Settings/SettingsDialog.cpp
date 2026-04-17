#include "SettingsDialog.h"
#include "./ui_SettingsDialog.h"
#include "DatabaseManager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QStandardPaths>

SettingsDialog::SettingsDialog(QWidget *parent, WeatherManager *weatherManager) :
    QWidget(parent),
    ui(new Ui::SettingsDialog),
    weatherManager(weatherManager)
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    ui->setupUi(this);
    
    // 初始化设置
    ui->weatherEnabledCheckBox->setChecked(weatherManager->isEnabled());
    
    // 加载并显示当前配置信息
    loadConfigDisplay();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_saveButton_clicked() {
    bool enabled = ui->weatherEnabledCheckBox->isChecked();
    QString apiKey = ui->apiKeyLineEdit->text().trimmed();
    QString city = ui->cityLineEdit->text().trimmed();
    
    if (enabled) {
        if (apiKey.isEmpty() || city.isEmpty()) {
            QMessageBox::warning(this, "警告", "启用天气功能需要输入 API Key 和城市");
            return;
        }
        
        weatherManager->setApiKey(apiKey);
        weatherManager->setCity(city);
    }
    
    weatherManager->setEnabled(enabled);
    
    QMessageBox::information(this, "成功", "设置已保存");
    close();
}

void SettingsDialog::on_cancelButton_clicked() {
    close();
}

void SettingsDialog::on_weatherEnabledCheckBox_stateChanged(int arg1) {
    bool enabled = (arg1 == Qt::Checked);
    ui->apiKeyLineEdit->setEnabled(enabled);
    ui->cityLineEdit->setEnabled(enabled);
}

void SettingsDialog::loadConfigDisplay() {
    // 从 WeatherManager 获取当前配置
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/weather_settings.ini";
    QFile file(configPath);
    
    QString apiKey;
    QString city;
    bool enabled = false;
    
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("apiKey=")) {
                apiKey = line.mid(7);
            } else if (line.startsWith("city=")) {
                city = line.mid(5);
            } else if (line.startsWith("enabled=")) {
                enabled = (line.mid(8) == "1");
            }
        }
        file.close();
    }
    
    // 显示配置信息：只有启用且配置了值时才显示，否则显示默认提示
    ui->currentApiKeyDisplayLabel->setText(enabled && !apiKey.isEmpty() ? apiKey : "未配置");
    ui->currentCityDisplayLabel->setText(enabled && !city.isEmpty() ? city : "未设置");
}

void SettingsDialog::on_backupButton_clicked() {
    QString backupPath = QFileDialog::getSaveFileName(this, "备份数据", QDir::homePath() + "/schedules_backup.db", "数据库文件 (*.db)");
    if (!backupPath.isEmpty()) {
        DatabaseManager dbManager;
        if (dbManager.backupDatabase(backupPath)) {
            QMessageBox::information(this, "成功", "数据备份成功");
        } else {
            QMessageBox::warning(this, "错误", "数据备份失败");
        }
    }
}

void SettingsDialog::on_restoreButton_clicked() {
    QString backupPath = QFileDialog::getOpenFileName(this, "恢复数据", QDir::homePath(), "数据库文件 (*.db)");
    if (!backupPath.isEmpty()) {
        if (QMessageBox::question(this, "确认恢复", "恢复数据将覆盖当前数据，确定要继续吗？") == QMessageBox::Yes) {
            DatabaseManager dbManager;
            if (dbManager.restoreDatabase(backupPath)) {
                QMessageBox::information(this, "成功", "数据恢复成功，请重启应用");
            } else {
                QMessageBox::warning(this, "错误", "数据恢复失败");
            }
        }
    }
}
