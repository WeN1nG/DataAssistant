#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QColor>
#include <QSqlDatabase>

struct Schedule {
    int id;
    QString title;
    QString description;
    QDateTime datetime;
    QDateTime endDatetime;      // 批量添加的结束日期时间（单日程时与datetime相同）
    bool isBatch;               // 是否为批量添加的日程
    int priority; // 0: 一般, 1: 重要, 2: 紧急
    int reminderMinutes; // 提前提醒分钟数
    bool completed;
    QColor color; // 日程颜色，根据priority自动设置
};

struct DeleteResult {
    int totalSchedules;      // 将要/实际删除的日程总数
    int batchCount;         // 涉及的批量日程批次数
    int batchDaysCount;     // 涉及的批量日程总天数
    int scheduleCount;      // 涉及的普通日程数
    int actualDeleted;      // 实际删除的日程数
    QStringList batchDetails; // 每个批量日程的详细信息
};

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();
    
    static QSqlDatabase getDatabaseConnection(const QString& connectionName = QString());

    bool addSchedule(const Schedule& schedule);
    bool addBatchSchedules(const QVector<Schedule>& schedules);
    bool updateSchedule(const Schedule& schedule);
    
    // ============ 传统删除函数（向后兼容）============
    bool deleteSchedule(int id);
    bool deleteBatchSchedules(const QString& batchGroupId);
    bool deleteScheduleWithBatch(int id);
    bool deleteAllSchedules();
    int deleteSchedulesByIds(const QVector<int>& ids);
    DeleteResult deleteSchedulesWithDetails(const QVector<int>& ids);
    DeleteResult getDeletePreview(const QVector<int>& ids);
    
    // ============ 新删除函数体系（基于需求文档实现）============
    // 基本删除函数1-单日程删除函数：只删除一个日程
    bool deleteSingleScheduleById(int id);
    
    // 基本删除函数2-单批量日程删除函数：通过循环删除整个批量日程
    bool deleteSingleBatchSchedule(const QString& title, const QDateTime& startDatetime, const QDateTime& endDatetime, int priority);
    
    // 单删除函数：先确定要删除的日程是单日程还是批量日程，根据情况调用不同的基本删除函数
    bool deleteSingleSchedule(int id);
    
    // 批量删除函数：通过循环调用单删除函数，实现批量删除
    int deleteMultipleSchedules(const QVector<int>& ids);
    
    QVector<Schedule> getSchedules();
    QVector<Schedule> getSchedulesByDate(const QDate& date);
    Schedule getScheduleById(int id);
    QVector<Schedule> getBatchSchedulesByGroupId(const QString& groupId);
    
    bool backupDatabase(const QString& backupPath);
    bool restoreDatabase(const QString& backupPath);
    
    static QColor getColorByPriority(int priority);

private:
    QString dbPath;
    bool initializeDatabase();
};

#endif // DATABASEMANAGER_H