#ifndef REMINDERMANAGER_H
#define REMINDERMANAGER_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QMap>
#include "DatabaseManager.h"
#include "SystemTrayNotification.h"

class ReminderManager : public QObject
{
    Q_OBJECT

public:
    explicit ReminderManager(QObject *parent = nullptr);
    ~ReminderManager();

    void startMonitoring();
    void stopMonitoring();
    bool markScheduleAsCompleted(int scheduleId);
    void testNotification();

signals:
    void scheduleClicked(int scheduleId);
    void scheduleCompleted(int scheduleId);

private slots:
    void checkReminders();
    void onNotificationClicked(int scheduleId);
    void onNotificationClosed();

private:
    QTimer *timer;
    DatabaseManager *dbManager;
    QMap<int, SystemTrayNotification*> activeNotifications;
    QSet<int> notifiedScheduleIds;

    void showReminder(const Schedule &schedule);
    void showSystemTrayNotification(const Schedule &schedule);
    void cleanupNotification(int scheduleId);
    void playNotificationSound();
};

#endif // REMINDERMANAGER_H