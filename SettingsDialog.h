#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QStandardPaths>
#include "WeatherManager.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr, WeatherManager *weatherManager = nullptr);
    ~SettingsDialog();

private slots:
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void on_weatherEnabledCheckBox_stateChanged(int arg1);
    void on_backupButton_clicked();
    void on_restoreButton_clicked();

private:
    void loadConfigDisplay();

    Ui::SettingsDialog *ui;
    WeatherManager *weatherManager;
};

#endif // SETTINGSDIALOG_H