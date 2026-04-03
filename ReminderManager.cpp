#include "ReminderManager.h"
#include <QMessageBox>
#include <QPushButton>
#include <QDateTime>

ReminderManager::ReminderManager(QObject *parent) :
    QObject(parent),
    timer(new QTimer(this)),
    dbManager(new DatabaseManager())
{
    connect(timer, &QTimer::timeout, this, &ReminderManager::checkReminders);
}

ReminderManager::~ReminderManager()
{
    delete dbManager;
    delete timer;
}

void ReminderManager::startMonitoring() {
    // 每60秒检查一次提醒
    timer->start(60000);
    // 立即检查一次
    checkReminders();
}

void ReminderManager::stopMonitoring() {
    timer->stop();
}

void ReminderManager::checkReminders() {
    QDateTime now = QDateTime::currentDateTime();
    QVector<Schedule> schedules = dbManager->getSchedules();
    
    for (const Schedule &schedule : schedules) {
        if (!schedule.completed && schedule.reminderMinutes > 0) {
            QDateTime reminderTime = schedule.datetime.addSecs(-schedule.reminderMinutes * 60);
            // 检查是否到达提醒时间（误差在1分钟内）
            if (now >= reminderTime && now <= reminderTime.addSecs(60)) {
                showReminder(schedule);
            }
        }
    }
}

void ReminderManager::showReminder(const Schedule &schedule) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("日程提醒");
    msgBox.setText(QString("您有一个日程即将到来：\n\n%1\n\n时间：%2\n\n描述：%3")
                   .arg(schedule.title)
                   .arg(schedule.datetime.toString("yyyy-MM-dd HH:mm"))
                   .arg(schedule.description.isEmpty() ? "无" : schedule.description));
    
    QPushButton *laterButton = msgBox.addButton("稍后提醒", QMessageBox::ActionRole);
    QPushButton *completedButton = msgBox.addButton("已完成", QMessageBox::ActionRole);
    QPushButton *closeButton = msgBox.addButton(QMessageBox::Close);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == completedButton) {
        // 标记日程为已完成
        Schedule updatedSchedule = schedule;
        updatedSchedule.completed = true;
        dbManager->updateSchedule(updatedSchedule);
    } else if (msgBox.clickedButton() == laterButton) {
        // 稍后提醒（暂时不做处理，等待下次检查）
    }
}
