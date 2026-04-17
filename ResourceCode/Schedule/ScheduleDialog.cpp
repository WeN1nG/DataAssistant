#include "ScheduleDialog.h"
#include "./ui_ScheduleDialog.h"
#include <QMessageBox>
#include <QDateTime>

ScheduleDialog::ScheduleDialog(QWidget *parent, Schedule *schedule) :
    QWidget(parent),
    ui(new Ui::ScheduleDialog),
    currentSchedule(schedule),
    isEditing(schedule != nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    ui->setupUi(this);
    
    // 设置窗口标题
    setWindowTitle(isEditing ? "编辑日程" : "添加日程");
    
    // 连接批量添加复选框信号
    connect(ui->batchAddCheckBox, &QCheckBox::checkStateChanged, 
            this, &ScheduleDialog::on_batchAddCheckBox_stateChanged);
    
    // 如果是编辑模式，填充现有数据
    if (isEditing) {
        ui->titleLineEdit->setText(currentSchedule->title);
        ui->descriptionTextEdit->setText(currentSchedule->description);
        ui->dateTimeEdit->setDateTime(currentSchedule->datetime);
        ui->priorityComboBox->setCurrentIndex(currentSchedule->priority);
        
        // 如果是批量日程，设置批量添加界面
        if (currentSchedule->isBatch) {
            ui->batchAddCheckBox->setChecked(true);
            ui->startDateTimeEdit->setDateTime(currentSchedule->datetime);
            ui->endDateTimeEdit->setDateTime(currentSchedule->endDatetime);
        }
        
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
        QDateTime defaultDateTime = QDateTime(QDate::currentDate(), QTime(8, 0, 0));
        ui->dateTimeEdit->setDateTime(defaultDateTime);
        ui->startDateTimeEdit->setDateTime(defaultDateTime);
        ui->endDateTimeEdit->setDateTime(defaultDateTime);
    }
}

ScheduleDialog::~ScheduleDialog()
{
    delete ui;
}

void ScheduleDialog::on_batchAddCheckBox_stateChanged(int state) {
    bool enabled = (state == Qt::Checked);
    ui->dateRangeFrame->setEnabled(enabled);
    ui->dateTimeEdit->setVisible(!enabled);
    
    if (enabled) {
        // 批量添加模式：显示日期范围框架
        ui->dateRangeFrame->show();
        ui->dateTimeEdit->hide();
    } else {
        // 单日期模式：隐藏日期范围框架
        ui->dateRangeFrame->hide();
        ui->dateTimeEdit->show();
    }
}

void ScheduleDialog::on_saveButton_clicked() {
    QString title = ui->titleLineEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入日程标题");
        return;
    }
    
    DatabaseManager dbManager;
    
    // 检查是否是批量添加模式
    if (ui->batchAddCheckBox->isChecked()) {
        // 批量添加模式
        if (!validateBatchDates()) {
            return;
        }
        
        QDateTime startDate = ui->startDateTimeEdit->dateTime();
        QDateTime endDate = ui->endDateTimeEdit->dateTime();
        
        // 创建批量日程数组
        QVector<Schedule> schedules;
        QDateTime currentDate = startDate;
        
        while (currentDate <= endDate) {
            Schedule schedule;
            schedule.id = -1;
            schedule.completed = false;
            schedule.title = title;
            schedule.description = ui->descriptionTextEdit->toPlainText();
            schedule.datetime = currentDate;
            schedule.endDatetime = endDate;
            schedule.isBatch = true;
            schedule.priority = ui->priorityComboBox->currentIndex();
            schedule.color = DatabaseManager::getColorByPriority(schedule.priority);
            
            // 设置提醒时间
            switch (ui->reminderComboBox->currentIndex()) {
            case 0: schedule.reminderMinutes = 0; break;
            case 1: schedule.reminderMinutes = 5; break;
            case 2: schedule.reminderMinutes = 15; break;
            case 3: schedule.reminderMinutes = 30; break;
            case 4: schedule.reminderMinutes = 60; break;
            case 5: schedule.reminderMinutes = 1440; break;
            default: schedule.reminderMinutes = 0; break;
            }
            
            schedules.append(schedule);
            currentDate = currentDate.addDays(1);
        }
        
        // 添加所有日程
        if (dbManager.addBatchSchedules(schedules)) {
            QString message = QString("成功添加 %1 个日程（%2 至 %3）")
                .arg(schedules.size())
                .arg(startDate.toString("yyyy-MM-dd"))
                .arg(endDate.toString("yyyy-MM-dd"));
            QMessageBox::information(this, "成功", message);
            close();
            emit scheduleSaved();
        } else {
            QMessageBox::warning(this, "错误", "批量添加日程失败，请重试");
        }
    } else {
        // 单日期添加模式
        Schedule schedule;
        
        if (isEditing) {
            schedule = *currentSchedule;
        }
        
        schedule.title = title;
        schedule.description = ui->descriptionTextEdit->toPlainText();
        schedule.datetime = ui->dateTimeEdit->dateTime();
        schedule.endDatetime = ui->dateTimeEdit->dateTime();
        schedule.isBatch = false;
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
            close();
            emit scheduleSaved();
        } else {
            QMessageBox::warning(this, "错误", "操作失败，请重试");
        }
    }
}

bool ScheduleDialog::validateBatchDates() {
    QDateTime startDate = ui->startDateTimeEdit->dateTime();
    QDateTime endDate = ui->endDateTimeEdit->dateTime();
    
    // 验证：结束日期必须晚于或等于开始日期
    if (endDate < startDate) {
        QMessageBox::warning(this, "日期错误", "结束日期必须晚于或等于开始日期");
        return false;
    }
    
    // 验证：日期范围不能超过30天
    int days = getDaysInRange(startDate, endDate);
    if (days > 30) {
        QMessageBox::warning(this, "日期范围过大", 
            QString("单次批量添加不能超过30天，当前选择为 %1 天\n请缩短日期范围后重试").arg(days));
        return false;
    }
    
    return true;
}

int ScheduleDialog::getDaysInRange(const QDateTime& start, const QDateTime& end) {
    return start.date().daysTo(end.date()) + 1;
}

void ScheduleDialog::on_cancelButton_clicked() {
    close();
}
