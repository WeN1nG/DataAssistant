#include "ReminderManager.h"
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#include <mmsystem.h>
#endif

ReminderManager::ReminderManager(QObject *parent) :
    QObject(parent),
    timer(new QTimer(this)),
    dbManager(new DatabaseManager())
{
    connect(timer, &QTimer::timeout, this, &ReminderManager::checkReminders);
}

ReminderManager::~ReminderManager()
{
    stopMonitoring();
    qDeleteAll(activeNotifications);
    activeNotifications.clear();
    delete dbManager;
}

void ReminderManager::startMonitoring() {
    // 每10秒检查一次提醒
    timer->start(10000);
    // 立即检查一次
    checkReminders();
    
    qDebug() << "ReminderManager: Started monitoring at" << QDateTime::currentDateTime();
}

void ReminderManager::stopMonitoring() {
    timer->stop();
}

bool ReminderManager::markScheduleAsCompleted(int scheduleId) {
    try {
        Schedule schedule = dbManager->getScheduleById(scheduleId);
        if (schedule.id == -1) {
            qWarning() << "Schedule not found:" << scheduleId;
            return false;
        }

        Schedule updatedSchedule = schedule;
        updatedSchedule.completed = true;
        bool success = dbManager->updateSchedule(updatedSchedule);

        if (success) {
            qDebug() << "Schedule marked as completed:" << schedule.title;
            emit scheduleCompleted(scheduleId);
            cleanupNotification(scheduleId);
        } else {
            qWarning() << "Failed to mark schedule as completed:" << scheduleId;
        }

        return success;
    } catch (const std::exception &e) {
        qCritical() << "Exception marking schedule as completed:" << e.what();
        return false;
    } catch (...) {
        qCritical() << "Unknown exception marking schedule as completed";
        return false;
    }
}

void ReminderManager::checkReminders() {
    QDateTime now = QDateTime::currentDateTime();
    QVector<Schedule> schedules = dbManager->getSchedules();
    
    for (const Schedule &schedule : schedules) {
        if (!schedule.completed && schedule.reminderMinutes > 0) {
            QDateTime reminderTime = schedule.datetime.addSecs(-schedule.reminderMinutes * 60);
            
            // 避免重复提醒
            if (notifiedScheduleIds.contains(schedule.id)) {
                continue;
            }
            
            // 检查是否到达提醒时间（扩大窗口到5分钟内，或已过期但还未通知）
            bool shouldNotify = false;
            
            if (now >= reminderTime && now <= reminderTime.addSecs(300)) {
                // 在提醒时间点后5分钟内
                shouldNotify = true;
            } else if (now > reminderTime.addSecs(300) && now <= schedule.datetime) {
                // 已超过5分钟窗口但还未到日程时间（已过期提醒）
                // 计算已过期多久
                int overdueMinutes = reminderTime.secsTo(now) / 60;
                qDebug() << "Schedule reminder overdue by" << overdueMinutes << "minutes for:" << schedule.title;
                shouldNotify = true;
            }
            
            if (shouldNotify) {
                notifiedScheduleIds.insert(schedule.id);
                showReminder(schedule);
            }
        }
    }
}

void ReminderManager::showReminder(const Schedule &schedule) {
    try {
        QDateTime reminderTime = schedule.datetime.addSecs(-schedule.reminderMinutes * 60);
        qDebug() << "Showing reminder for schedule:" << schedule.title
                 << "at" << schedule.datetime.toString("yyyy-MM-dd HH:mm")
                 << "with" << schedule.reminderMinutes << "minutes advance notice"
                 << "reminder time was:" << reminderTime.toString("yyyy-MM-dd HH:mm")
                 << "current time:" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        
        playNotificationSound();
        showSystemTrayNotification(schedule);
        qDebug() << "System tray notification shown successfully for schedule:" << schedule.title;
    } catch (const std::exception &e) {
        qCritical() << "Failed to show system tray notification:" << e.what();
    } catch (...) {
        qCritical() << "Unknown error showing system tray notification";
    }
}

void ReminderManager::playNotificationSound() {
    qDebug() << "Playing notification sound...";

#ifdef Q_OS_WIN
    qDebug() << "Platform: Windows, using Windows Audio API";

    BOOL playResult = PlaySound(MAKEINTRESOURCE(SND_ALIAS_SYSTEMEXCLAMATION), NULL, SND_ALIAS_ID | SND_ASYNC);
    if (!playResult) {
        qDebug() << "PlaySound failed with error code:" << GetLastError();
        qDebug() << "Trying MessageBeep as fallback...";
        MessageBeep(MB_ICONEXCLAMATION);
    } else {
        qDebug() << "PlaySound executed successfully";
    }

#elif defined(Q_OS_MAC)
    qDebug() << "Platform: macOS";
    QApplication::beep();

#else
    qDebug() << "Platform: Other (Linux/Unix), using QApplication::beep()";
    QApplication::beep();
#endif
}

void ReminderManager::showSystemTrayNotification(const Schedule &schedule) {
    try {
        SystemTrayNotification *notification = new SystemTrayNotification(schedule);
        
        connect(notification, &SystemTrayNotification::notificationClicked,
                this, &ReminderManager::onNotificationClicked);
        connect(notification, &SystemTrayNotification::notificationClosed,
                this, &ReminderManager::onNotificationClosed);
        connect(notification, &SystemTrayNotification::scheduleMarkedAsCompleted,
                this, &ReminderManager::markScheduleAsCompleted);
        
        activeNotifications[schedule.id] = notification;
        notification->showNotification();
    } catch (const std::exception &e) {
        qCritical() << "Exception creating system tray notification:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception creating system tray notification";
    }
}

void ReminderManager::onNotificationClicked(int scheduleId) {
    try {
        Schedule schedule = dbManager->getScheduleById(scheduleId);
        if (schedule.id != -1) {
            qDebug() << "Notification clicked for schedule:" << schedule.title;
            emit scheduleClicked(scheduleId);
        }
        cleanupNotification(scheduleId);
    } catch (const std::exception &e) {
        qCritical() << "Exception handling notification click:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception handling notification click";
    }
}

void ReminderManager::onNotificationClosed() {
    try {
        // 通知已关闭，可能需要更新状态
    } catch (const std::exception &e) {
        qCritical() << "Exception handling notification close:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception handling notification close";
    }
}

void ReminderManager::cleanupNotification(int scheduleId) {
    try {
        if (activeNotifications.contains(scheduleId)) {
            SystemTrayNotification *notification = activeNotifications.take(scheduleId);
            if (notification) {
                if (!notification->isHidden()) {
                    notification->hideNotification();
                }
                QTimer::singleShot(500, notification, &QObject::deleteLater);
            }
        }
    } catch (const std::exception &e) {
        qCritical() << "Exception cleaning up notification:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception cleaning up notification";
    }
}

void ReminderManager::testNotification() {
    qDebug() << "=== Testing notification sound ===";
    playNotificationSound();
    
    Schedule testSchedule;
    testSchedule.id = -1;
    testSchedule.title = "测试提醒";
    testSchedule.description = "这是一个测试提醒，用于验证提醒功能是否正常工作";
    testSchedule.datetime = QDateTime::currentDateTime().addSecs(5);
    testSchedule.reminderMinutes = 0;
    testSchedule.completed = false;
    
    qDebug() << "Test notification created, showing now...";
    showSystemTrayNotification(testSchedule);
}
