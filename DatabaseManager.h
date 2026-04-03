#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QDateTime>
#include <QVector>

struct Schedule {
    int id;
    QString title;
    QString description;
    QDateTime datetime;
    int priority; // 0: 一般, 1: 重要, 2: 紧急
    int reminderMinutes; // 提前提醒分钟数
    bool completed;
};

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool addSchedule(const Schedule& schedule);
    bool updateSchedule(const Schedule& schedule);
    bool deleteSchedule(int id);
    QVector<Schedule> getSchedules();
    QVector<Schedule> getSchedulesByDate(const QDate& date);
    Schedule getScheduleById(int id);
    
    bool backupDatabase(const QString& backupPath);
    bool restoreDatabase(const QString& backupPath);

private:
    QString dbPath;
    bool initializeDatabase();
};

#endif // DATABASEMANAGER_H