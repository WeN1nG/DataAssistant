#ifndef REMINDERMANAGER_H
#define REMINDERMANAGER_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include "DatabaseManager.h"

class ReminderManager : public QObject
{
    Q_OBJECT

public:
    explicit ReminderManager(QObject *parent = nullptr);
    ~ReminderManager();

    void startMonitoring();
    void stopMonitoring();

private slots:
    void checkReminders();

private:
    QTimer *timer;
    DatabaseManager *dbManager;
    void showReminder(const Schedule &schedule);
};

#endif // REMINDERMANAGER_H