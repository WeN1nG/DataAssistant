#include "ScheduleDialog.h"
#include "./ui_ScheduleDialog.h"
#include <QMessageBox>

ScheduleDialog::ScheduleDialog(QWidget *parent, Schedule *schedule) :
    QDialog(parent),
    ui(new Ui::ScheduleDialog),
    currentSchedule(schedule),
    isEditing(schedule != nullptr)
{
    ui->setupUi(this);
    
    // 设置窗口标题
    setWindowTitle(isEditing ? "编辑日程" : "添加日程");
    
    // 如果是编辑模式，填充现有数据
    if (isEditing) {
        ui->titleLineEdit->setText(currentSchedule->title);
        ui->descriptionTextEdit->setText(currentSchedule->description);
        ui->dateTimeEdit->setDateTime(currentSchedule->datetime);
        ui->priorityComboBox->setCurrentIndex(currentSchedule->priority);
        
        // 根据reminderMinutes设置提醒时间
        switch (currentSchedule->reminderMinutes) {
        case 0: ui->reminderComboBox->setCurrentIndex(0); break;
        case 5: ui->reminderComboBox->setCurrentIndex(1); break;
        case 15: ui->reminderComboBox->setCurrentIndex(2); break;
        case 30: ui->reminderComboBox->setCurrentIndex(3); break;
        case 60: ui->reminderComboBox->setCurrentIndex(4); break;
        case 1440: ui->reminderComboBox->setCurrentIndex(5); break;
        default: ui->reminderComboBox->setCurrentIndex(0); break;
        }
    } else {
        // 新建模式，设置默认时间为今天的8点
        ui->dateTimeEdit->setDateTime(QDateTime(QDate::currentDate(), QTime(8, 0, 0)));
    }
}

ScheduleDialog::~ScheduleDialog()
{
    delete ui;
}

void ScheduleDialog::on_saveButton_clicked() {
    QString title = ui->titleLineEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入日程标题");
        return;
    }
    
    DatabaseManager dbManager;
    Schedule schedule;
    
    if (isEditing) {
        schedule = *currentSchedule;
    }
    
    schedule.title = title;
    schedule.description = ui->descriptionTextEdit->toPlainText();
    schedule.datetime = ui->dateTimeEdit->dateTime();
    schedule.priority = ui->priorityComboBox->currentIndex();
    
    // 根据选择的提醒时间设置reminderMinutes
    switch (ui->reminderComboBox->currentIndex()) {
    case 0: schedule.reminderMinutes = 0; break;
    case 1: schedule.reminderMinutes = 5; break;
    case 2: schedule.reminderMinutes = 15; break;
    case 3: schedule.reminderMinutes = 30; break;
    case 4: schedule.reminderMinutes = 60; break;
    case 5: schedule.reminderMinutes = 1440; break;
    default: schedule.reminderMinutes = 0; break;
    }
    
    // 根据priority设置日程颜色
    schedule.color = DatabaseManager::getColorByPriority(schedule.priority);
    
    bool success;
    if (isEditing) {
        success = dbManager.updateSchedule(schedule);
    } else {
        schedule.id = -1;
        schedule.completed = false;
        success = dbManager.addSchedule(schedule);
    }
    
    if (success) {
        QMessageBox::information(this, "成功", isEditing ? "日程更新成功" : "日程添加成功");
        accept();
    } else {
        QMessageBox::warning(this, "错误", "操作失败，请重试");
    }
}

void ScheduleDialog::on_cancelButton_clicked() {
    reject();
}
