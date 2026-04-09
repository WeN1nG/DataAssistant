#include "DatabaseManager.h"
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>

static QSqlDatabase sharedDb;
static bool dbInitialized = false;

QSqlDatabase DatabaseManager::getDatabaseConnection(const QString& connectionName) {
    Q_UNUSED(connectionName);
    
    if (!dbInitialized) {
        sharedDb = QSqlDatabase::addDatabase("QSQLITE");
        sharedDb.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/schedules.db");
        
        if (!sharedDb.open()) {
            qWarning() << "Failed to open database:" << sharedDb.lastError().text();
        }
        
        dbInitialized = true;
    }
    
    return sharedDb;
}

DatabaseManager::DatabaseManager() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    dbPath = appDataPath + "/schedules.db";
    initializeDatabase();
}

DatabaseManager::~DatabaseManager() {
    
}

bool DatabaseManager::initializeDatabase() {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    // 创建支持批量添加的数据库结构
    query.exec("CREATE TABLE IF NOT EXISTS schedules (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT NOT NULL, description TEXT, datetime TEXT NOT NULL, endDatetime TEXT DEFAULT '', isBatch INTEGER DEFAULT 0, priority INTEGER DEFAULT 0, reminderMinutes INTEGER DEFAULT 0, completed INTEGER DEFAULT 0, color TEXT DEFAULT '')");
    
    // 检查并添加新字段（向后兼容�?
    if (!query.exec("SELECT color FROM schedules LIMIT 1")) {
        query.exec("ALTER TABLE schedules ADD COLUMN color TEXT DEFAULT ''");
    }
    
    if (!query.exec("SELECT endDatetime FROM schedules LIMIT 1")) {
        query.exec("ALTER TABLE schedules ADD COLUMN endDatetime TEXT DEFAULT ''");
    }
    
    if (!query.exec("SELECT isBatch FROM schedules LIMIT 1")) {
        query.exec("ALTER TABLE schedules ADD COLUMN isBatch INTEGER DEFAULT 0");
    }

    db.close();
    return true;
}

QColor DatabaseManager::getColorByPriority(int priority) {
    switch (priority) {
    case 2: return Qt::red;      // 紧急：红色
    case 1: return QColor(255, 165, 0);  // 重要：橙�?
    default: return Qt::blue;    // 一般：蓝色
    }
}

bool DatabaseManager::addSchedule(const Schedule& schedule) {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO schedules (title, description, datetime, endDatetime, isBatch, priority, reminderMinutes, completed, color) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(schedule.title);
    query.addBindValue(schedule.description);
    query.addBindValue(schedule.datetime.toString(Qt::ISODate));
    query.addBindValue(schedule.endDatetime.toString(Qt::ISODate));
    query.addBindValue(schedule.isBatch ? 1 : 0);
    query.addBindValue(schedule.priority);
    query.addBindValue(schedule.reminderMinutes);
    query.addBindValue(schedule.completed ? 1 : 0);
    query.addBindValue(schedule.color.isValid() ? schedule.color.name() : getColorByPriority(schedule.priority).name());

    bool success = query.exec();
    db.close();
    return success;
}

bool DatabaseManager::addBatchSchedules(const QVector<Schedule>& schedules) {
    if (schedules.isEmpty()) {
        return false;
    }
    
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    bool success = true;
    
    db.transaction();
    
    for (const Schedule& schedule : schedules) {
        query.prepare("INSERT INTO schedules (title, description, datetime, endDatetime, isBatch, priority, reminderMinutes, completed, color) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(schedule.title);
        query.addBindValue(schedule.description);
        query.addBindValue(schedule.datetime.toString(Qt::ISODate));
        query.addBindValue(schedule.endDatetime.toString(Qt::ISODate));
        query.addBindValue(schedule.isBatch ? 1 : 0);
        query.addBindValue(schedule.priority);
        query.addBindValue(schedule.reminderMinutes);
        query.addBindValue(schedule.completed ? 1 : 0);
        query.addBindValue(schedule.color.isValid() ? schedule.color.name() : getColorByPriority(schedule.priority).name());
        
        if (!query.exec()) {
            success = false;
            break;
        }
    }
    
    if (success) {
        db.commit();
    } else {
        db.rollback();
    }
    
    db.close();
    return success;
}

bool DatabaseManager::updateSchedule(const Schedule& schedule) {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE schedules SET title = ?, description = ?, datetime = ?, endDatetime = ?, isBatch = ?, priority = ?, reminderMinutes = ?, completed = ?, color = ? WHERE id = ?");
    query.addBindValue(schedule.title);
    query.addBindValue(schedule.description);
    query.addBindValue(schedule.datetime.toString(Qt::ISODate));
    query.addBindValue(schedule.endDatetime.toString(Qt::ISODate));
    query.addBindValue(schedule.isBatch ? 1 : 0);
    query.addBindValue(schedule.priority);
    query.addBindValue(schedule.reminderMinutes);
    query.addBindValue(schedule.completed ? 1 : 0);
    query.addBindValue(schedule.color.isValid() ? schedule.color.name() : getColorByPriority(schedule.priority).name());
    query.addBindValue(schedule.id);

    bool success = query.exec();
    db.close();
    return success;
}

bool DatabaseManager::deleteSchedule(int id) {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM schedules WHERE id = ?");
    query.addBindValue(id);

    bool success = query.exec();
    db.close();
    return success;
}

bool DatabaseManager::deleteBatchSchedules(const QString& batchGroupId) {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM schedules WHERE isBatch = 1 AND datetime LIKE ?");
    query.addBindValue(batchGroupId + "%");

    bool success = query.exec();
    db.close();
    return success;
}

bool DatabaseManager::deleteScheduleWithBatch(int id) {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    // 首先查询该日程的详细信息
    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT isBatch, title, datetime, endDatetime, priority FROM schedules WHERE id = ?");
    selectQuery.addBindValue(id);

    if (!selectQuery.exec() || !selectQuery.next()) {
        db.close();
        return false;
    }

    bool isBatch = selectQuery.value(0).toInt() == 1;
    QString title = selectQuery.value(1).toString();
    QString datetime = selectQuery.value(2).toString();
    QString endDatetime = selectQuery.value(3).toString();
    int priority = selectQuery.value(4).toInt();

    // 使用新的query对象执行删除操作
    QSqlQuery deleteQuery;
    if (isBatch) {
        deleteQuery.prepare("DELETE FROM schedules WHERE isBatch = 1 AND title = ? AND datetime = ? AND endDatetime = ? AND priority = ?");
        deleteQuery.addBindValue(title);
        deleteQuery.addBindValue(datetime);
        deleteQuery.addBindValue(endDatetime);
        deleteQuery.addBindValue(priority);
    } else {
        deleteQuery.prepare("DELETE FROM schedules WHERE id = ?");
        deleteQuery.addBindValue(id);
    }

    bool success = deleteQuery.exec();
    db.close();
    return success;
}

bool DatabaseManager::deleteAllSchedules() {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    bool success = query.exec("DELETE FROM schedules");
    db.close();
    return success;
}

int DatabaseManager::deleteSchedulesByIds(const QVector<int>& ids) {
    if (ids.isEmpty()) {
        return 0;
    }

    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return 0;
    }

    db.transaction();
    int deletedCount = 0;

    for (int id : ids) {
        // 先查询该日程的详细信息（在同一事务中）
        QSqlQuery selectQuery;
        selectQuery.prepare("SELECT isBatch, title, datetime, endDatetime, priority FROM schedules WHERE id = ?");
        selectQuery.addBindValue(id);

        if (selectQuery.exec() && selectQuery.next()) {
            bool isBatch = selectQuery.value(0).toInt() == 1;
            QString title = selectQuery.value(1).toString();
            QString datetime = selectQuery.value(2).toString();
            QString endDatetime = selectQuery.value(3).toString();
            int priority = selectQuery.value(4).toInt();

            // 使用新的query对象执行删除操作
            QSqlQuery deleteQuery;
            if (isBatch) {
                deleteQuery.prepare("DELETE FROM schedules WHERE isBatch = 1 AND title = ? AND datetime = ? AND endDatetime = ? AND priority = ?");
                deleteQuery.addBindValue(title);
                deleteQuery.addBindValue(datetime);
                deleteQuery.addBindValue(endDatetime);
                deleteQuery.addBindValue(priority);
            } else {
                deleteQuery.prepare("DELETE FROM schedules WHERE id = ?");
                deleteQuery.addBindValue(id);
            }

            if (deleteQuery.exec()) {
                deletedCount++;
            }
        }
    }

    db.commit();
    db.close();
    return deletedCount;
}

// 基本删除函数1-单日程删除函数：只删除一个日�?
bool DatabaseManager::deleteSingleScheduleById(int id) {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        qDebug() << "[ERROR] deleteSingleScheduleById: Failed to open database";
        return false;
    }
    
    QSqlQuery query;
    query.prepare("DELETE FROM schedules WHERE id = ?");
    query.addBindValue(id);
    
    bool success = query.exec();
    if (!success) {
        qDebug() << "[ERROR] deleteSingleScheduleById: Failed to delete schedule" << id
                 << "Error:" << query.lastError().text();
    } else {
        qDebug() << "[INFO] deleteSingleScheduleById: Successfully deleted schedule" << id;
    }
    
    db.close();
    return success;
}

// 基本删除函数2-单批量日程删除函数：通过循环删除整个批量日程
bool DatabaseManager::deleteSingleBatchSchedule(const QString& title, const QDateTime& startDatetime, const QDateTime& endDatetime, int priority) {
    if (!startDatetime.isValid()) {
        qDebug() << "[ERROR] deleteSingleBatchSchedule: Invalid startDatetime";
        return false;
    }
    
    QDateTime actualEndDatetime = endDatetime.isValid() ? endDatetime : startDatetime;
    
    qDebug() << "[INFO] deleteSingleBatchSchedule: Starting batch deletion"
             << "Title:" << title
             << "From:" << startDatetime.toString(Qt::ISODate)
             << "To:" << actualEndDatetime.toString(Qt::ISODate)
             << "Priority:" << priority;
    
    // 首先查询数据库中实际存在的记录
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        qDebug() << "[ERROR] deleteSingleBatchSchedule: Failed to open database";
        return false;
    }
    
    // 查询将要删除的记录数
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM schedules WHERE isBatch = 1 AND title = ? AND datetime >= ? AND datetime <= ? AND priority = ?");
    checkQuery.addBindValue(title);
    checkQuery.addBindValue(startDatetime.toString(Qt::ISODate));
    checkQuery.addBindValue(actualEndDatetime.toString(Qt::ISODate));
    checkQuery.addBindValue(priority);
    
    if (checkQuery.exec() && checkQuery.next()) {
        int foundCount = checkQuery.value(0).toInt();
        qDebug() << "[INFO] deleteSingleBatchSchedule: Found" << foundCount << "schedules to delete";
        if (foundCount == 0) {
            qDebug() << "[WARNING] deleteSingleBatchSchedule: No schedules found matching criteria!";
            db.close();
            return false;
        }
    }
    
    db.transaction();
    bool allSuccess = true;
    
    QDateTime currentDatetime = startDatetime;
    int dayCount = 0;
    
    while (currentDatetime <= actualEndDatetime) {
        QSqlQuery query;
        query.prepare("DELETE FROM schedules WHERE isBatch = 1 AND title = ? AND datetime = ? AND priority = ?");
        query.addBindValue(title);
        query.addBindValue(currentDatetime.toString(Qt::ISODate));
        query.addBindValue(priority);
        
        if (!query.exec()) {
            qDebug() << "[WARNING] deleteSingleBatchSchedule: Failed to delete schedule at" 
                     << currentDatetime.toString(Qt::ISODate)
                     << "Error:" << query.lastError().text();
            allSuccess = false;
        } else {
            int rowsAffected = query.numRowsAffected();
            dayCount++;
            qDebug() << "[DEBUG] deleteSingleBatchSchedule: Deleted" << rowsAffected << "schedule(s) at" 
                     << currentDatetime.toString(Qt::ISODate);
        }
        
        currentDatetime = currentDatetime.addDays(1);
    }
    
    if (allSuccess) {
        db.commit();
        qDebug() << "[INFO] deleteSingleBatchSchedule: Successfully deleted" << dayCount << "schedules";
    } else {
        db.rollback();
        qDebug() << "[WARNING] deleteSingleBatchSchedule: Rolled back due to errors";
    }
    
    db.close();
    return allSuccess;
}

// 单删除函数：先确定要删除的日程是单日程还是批量日程，根据情况调用不同的基本删除函�?
bool DatabaseManager::deleteSingleSchedule(int id) {
    qDebug() << "[INFO] deleteSingleSchedule: Starting deletion for schedule id" << id;
    
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        qDebug() << "[ERROR] deleteSingleSchedule: Failed to open database";
        return false;
    }
    
    QSqlQuery query;
    query.prepare("SELECT isBatch, title, datetime, endDatetime, priority FROM schedules WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec() || !query.next()) {
        qDebug() << "[ERROR] deleteSingleSchedule: Schedule not found, id =" << id;
        db.close();
        return false;
    }
    
    bool isBatch = query.value(0).toInt() == 1;
    QString title = query.value(1).toString();
    QDateTime datetime = QDateTime::fromString(query.value(2).toString(), Qt::ISODate);
    QDateTime endDatetime = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
    int priority = query.value(4).toInt();
    
    db.close();
    
    qDebug() << "[INFO] deleteSingleSchedule: Processing schedule id =" << id
             << "isBatch =" << isBatch
             << "title =" << title
             << "datetime =" << datetime.toString(Qt::ISODate)
             << "endDatetime =" << endDatetime.toString(Qt::ISODate)
             << "priority =" << priority;
    
    if (isBatch) {
        qDebug() << "[INFO] deleteSingleSchedule: Calling deleteSingleBatchSchedule for batch schedule";
        return deleteSingleBatchSchedule(title, datetime, endDatetime, priority);
    } else {
        qDebug() << "[INFO] deleteSingleSchedule: Calling deleteSingleScheduleById for single schedule";
        return deleteSingleScheduleById(id);
    }
}

// 批量删除函数：通过循环调用单删除函数，实现批量删除
int DatabaseManager::deleteMultipleSchedules(const QVector<int>& ids) {
    if (ids.isEmpty()) {
        qDebug() << "[INFO] deleteMultipleSchedules: No IDs provided";
        return 0;
    }
    
    qDebug() << "[INFO] deleteMultipleSchedules: Deleting" << ids.size() << "schedules";
    
    int successCount = 0;
    
    for (int id : ids) {
        if (deleteSingleSchedule(id)) {
            successCount++;
        } else {
            qDebug() << "[WARNING] deleteMultipleSchedules: Failed to delete schedule" << id;
        }
    }
    
    qDebug() << "[INFO] deleteMultipleSchedules: Successfully deleted" << successCount 
             << "out of" << ids.size() << "schedules";
    
    return successCount;
}

::DeleteResult DatabaseManager::getDeletePreview(const QVector<int>& ids) {
    ::DeleteResult result;
    result.totalSchedules = 0;
    result.batchCount = 0;
    result.batchDaysCount = 0;
    result.scheduleCount = 0;
    result.actualDeleted = 0;
    
    if (ids.isEmpty()) {
        qDebug() << "[DEBUG] getDeletePreview: ids is empty!";
        return result;
    }
    
    qDebug() << "[DEBUG] getDeletePreview called with" << ids.size() << "IDs:" << ids;
    
    QSqlDatabase db = getDatabaseConnection();
    
    if (!db.isOpen()) {
        qDebug() << "[DEBUG] Database not open, trying to open...";
        if (!db.open()) {
            qDebug() << "[DEBUG] Failed to open database in getDeletePreview";
            return result;
        }
    }
    
    QSet<QString> processedBatches;  // 用于避免重复计算同一批次
    
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("SELECT isBatch, title, datetime, endDatetime, priority FROM schedules WHERE id = ?");
        query.addBindValue(id);
        
        if (query.exec() && query.next()) {
            bool isBatch = query.value(0).toInt() == 1;
            QString title = query.value(1).toString();
            QString datetime = query.value(2).toString();
            QString endDatetime = query.value(3).toString();
            int priority = query.value(4).toInt();
            
            // 增强识别逻辑：如�?endDatetime > datetime，也视为批量日程
            QDateTime startDate = QDateTime::fromString(datetime, Qt::ISODate);
            QDateTime endDate = QDateTime::fromString(endDatetime, Qt::ISODate);
            bool hasEndDate = endDate.isValid() && endDate > startDate;
            
            // 生成唯一标识�?- 对于批量日程，不包含datetime
            QString batchKey = QString("%1|%2|%3").arg(title).arg(endDatetime).arg(priority);
            
            // 如果 isBatch = 1 或者有结束日期（endDatetime > datetime），视为批量日程
            if (isBatch || hasEndDate) {
                // 只计算一次同一批次
                if (!processedBatches.contains(batchKey)) {
                    processedBatches.insert(batchKey);
                    
                    // 查询该批次实际包含多少条记录
                    QSqlQuery countQuery;
                    if (isBatch) {
                        countQuery.prepare("SELECT COUNT(*) FROM schedules WHERE isBatch = 1 AND title = ? AND endDatetime = ? AND priority = ?");
                        countQuery.addBindValue(title);
                        countQuery.addBindValue(endDatetime);
                        countQuery.addBindValue(priority);
                    } else {
                        countQuery.prepare("SELECT COUNT(*) FROM schedules WHERE title = ? AND endDatetime = ? AND priority = ?");
                        countQuery.addBindValue(title);
                        countQuery.addBindValue(endDatetime);
                        countQuery.addBindValue(priority);
                    }
                    
                    int actualDays = 0;
                    if (countQuery.exec() && countQuery.next()) {
                        actualDays = countQuery.value(0).toInt();
                    }
                    
                    if (actualDays < 1) actualDays = 1;
                    
                    result.batchCount++;
                    result.batchDaysCount += actualDays;
                    result.totalSchedules += actualDays;
                    
                    // 添加批次详情
                    QString detail = QString("%1 (%2 天, %3 至 %4)")
                        .arg(title)
                        .arg(actualDays)
                        .arg(startDate.toString("MM/dd"))
                        .arg(endDate.toString("MM/dd"));
                    result.batchDetails.append(detail);
                }
            } else {
                // 普通日程
                result.scheduleCount++;
                result.totalSchedules++;
            }
        }
    }
    
    qDebug() << "[DEBUG] getDeletePreview result - totalSchedules:" << result.totalSchedules 
             << "batchCount:" << result.batchCount;
    
    db.close();
    
    return result;
}



::DeleteResult DatabaseManager::deleteSchedulesWithDetails(const QVector<int>& ids) {
    // �Ȼ�ȡԤ����Ϣ������ͳ�����飩
    ::DeleteResult result = getDeletePreview(ids);
    
    if (result.totalSchedules == 0) {
        qDebug() << "[DEBUG] deleteSchedulesWithDetails: No schedules to delete";
        return result;
    }
    
    qDebug() << "[DEBUG] deleteSchedulesWithDetails: Will delete" << result.totalSchedules
             << " schedules (batchCount:" << result.batchCount << ")";
    
    // ִ������ɾ��������ͳһ��ɾ��������
    int actualDeleted = deleteMultipleSchedules(ids);
    
    qDebug() << "[DEBUG] deleteSchedulesWithDetails: Actually deleted" << actualDeleted << " schedules";
    
    // ����ʵ��ɾ������
    result.actualDeleted = actualDeleted;
    
    return result;
}


QVector<Schedule> DatabaseManager::getBatchSchedulesByGroupId(const QString& groupId) {
    QVector<Schedule> schedules;
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return schedules;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM schedules WHERE isBatch = 1 AND datetime LIKE ? ORDER BY datetime ASC");
    query.addBindValue(groupId + "%");

    if (query.exec()) {
        while (query.next()) {
            Schedule schedule;
            // 使用列名代替索引，避免字段顺序问�?
            schedule.id = query.value("id").toInt();
            schedule.title = query.value("title").toString();
            schedule.description = query.value("description").toString();
            schedule.datetime = QDateTime::fromString(query.value("datetime").toString(), Qt::ISODate);
            
            // 读取批量添加字段
            QString endDatetimeStr = query.value("endDatetime").toString();
            schedule.endDatetime = endDatetimeStr.isEmpty() ? schedule.datetime : QDateTime::fromString(endDatetimeStr, Qt::ISODate);
            schedule.isBatch = query.value("isBatch").toInt() == 1;
            
            // 读取其他字段
            schedule.priority = query.value("priority").toInt();
            schedule.reminderMinutes = query.value("reminderMinutes").toInt();
            schedule.completed = query.value("completed").toInt() == 1;
            
            // 读取颜色
            QString colorStr = query.value("color").toString();
            if (!colorStr.isEmpty()) {
                schedule.color = QColor(colorStr);
            } else {
                schedule.color = getColorByPriority(schedule.priority);
            }
            
            schedules.append(schedule);
        }
    }

    db.close();
    return schedules;
}

QVector<Schedule> DatabaseManager::getSchedules() {
    QVector<Schedule> schedules;
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return schedules;
    }

    QSqlQuery query("SELECT * FROM schedules ORDER BY datetime ASC");
    while (query.next()) {
        Schedule schedule;
        // 使用列名代替索引，避免字段顺序问�?
        schedule.id = query.value("id").toInt();
        schedule.title = query.value("title").toString();
        schedule.description = query.value("description").toString();
        schedule.datetime = QDateTime::fromString(query.value("datetime").toString(), Qt::ISODate);
        
        // 读取批量添加字段
        QString endDatetimeStr = query.value("endDatetime").toString();
        schedule.endDatetime = endDatetimeStr.isEmpty() ? schedule.datetime : QDateTime::fromString(endDatetimeStr, Qt::ISODate);
        schedule.isBatch = query.value("isBatch").toInt() == 1;
        
        // 读取其他字段
        schedule.priority = query.value("priority").toInt();
        schedule.reminderMinutes = query.value("reminderMinutes").toInt();
        schedule.completed = query.value("completed").toInt() == 1;
        
        // 读取颜色
        QString colorStr = query.value("color").toString();
        if (!colorStr.isEmpty()) {
            schedule.color = QColor(colorStr);
        } else {
            schedule.color = getColorByPriority(schedule.priority);
        }
        
        schedules.append(schedule);
    }

    db.close();
    return schedules;
}

QVector<Schedule> DatabaseManager::getSchedulesByDate(const QDate& date) {
    QVector<Schedule> schedules;
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return schedules;
    }

    QDateTime startOfDay = QDateTime(date, QTime(0, 0, 0));
    QDateTime endOfDay = QDateTime(date, QTime(23, 59, 59));

    QSqlQuery query;
    // 查询在该日期范围内或结束日期在该日期范围内的日程
    query.prepare("SELECT * FROM schedules WHERE (datetime >= ? AND datetime <= ?) OR (endDatetime >= ? AND endDatetime <= ?) OR (datetime <= ? AND endDatetime >= ?) ORDER BY datetime ASC");
    query.addBindValue(startOfDay.toString(Qt::ISODate));
    query.addBindValue(endOfDay.toString(Qt::ISODate));
    query.addBindValue(startOfDay.toString(Qt::ISODate));
    query.addBindValue(endOfDay.toString(Qt::ISODate));
    query.addBindValue(startOfDay.toString(Qt::ISODate));
    query.addBindValue(endOfDay.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            Schedule schedule;
            // 使用列名代替索引，避免字段顺序问�?
            schedule.id = query.value("id").toInt();
            schedule.title = query.value("title").toString();
            schedule.description = query.value("description").toString();
            schedule.datetime = QDateTime::fromString(query.value("datetime").toString(), Qt::ISODate);
            
            // 读取批量添加字段
            QString endDatetimeStr = query.value("endDatetime").toString();
            schedule.endDatetime = endDatetimeStr.isEmpty() ? schedule.datetime : QDateTime::fromString(endDatetimeStr, Qt::ISODate);
            schedule.isBatch = query.value("isBatch").toInt() == 1;
            
            // 读取其他字段
            schedule.priority = query.value("priority").toInt();
            schedule.reminderMinutes = query.value("reminderMinutes").toInt();
            schedule.completed = query.value("completed").toInt() == 1;
            
            // 读取颜色
            QString colorStr = query.value("color").toString();
            if (!colorStr.isEmpty()) {
                schedule.color = QColor(colorStr);
            } else {
                schedule.color = getColorByPriority(schedule.priority);
            }
            
            schedules.append(schedule);
        }
    }

    db.close();
    return schedules;
}

Schedule DatabaseManager::getScheduleById(int id) {
    Schedule schedule;
    schedule.id = -1;

    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return schedule;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM schedules WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        // 使用列名代替索引，避免字段顺序问�?
        schedule.id = query.value("id").toInt();
        schedule.title = query.value("title").toString();
        schedule.description = query.value("description").toString();
        schedule.datetime = QDateTime::fromString(query.value("datetime").toString(), Qt::ISODate);
        
        // 读取批量添加字段
        QString endDatetimeStr = query.value("endDatetime").toString();
        schedule.endDatetime = endDatetimeStr.isEmpty() ? schedule.datetime : QDateTime::fromString(endDatetimeStr, Qt::ISODate);
        schedule.isBatch = query.value("isBatch").toInt() == 1;
        
        // 读取其他字段
        schedule.priority = query.value("priority").toInt();
        schedule.reminderMinutes = query.value("reminderMinutes").toInt();
        schedule.completed = query.value("completed").toInt() == 1;
        
        // 读取颜色
        QString colorStr = query.value("color").toString();
        if (!colorStr.isEmpty()) {
            schedule.color = QColor(colorStr);
        } else {
            schedule.color = getColorByPriority(schedule.priority);
        }
    }

    db.close();
    return schedule;
}

bool DatabaseManager::backupDatabase(const QString& backupPath) {
    QFile sourceFile(dbPath);
    QFile backupFile(backupPath);
    
    if (!sourceFile.exists()) {
        return false;
    }
    
    return sourceFile.copy(backupFile.fileName());
}

bool DatabaseManager::restoreDatabase(const QString& backupPath) {
    QFile backupFile(backupPath);
    QFile sourceFile(dbPath);
    
    if (!backupFile.exists()) {
        return false;
    }
    
    // 关闭数据库连�?
    
    
    // 删除原数据库文件
    if (sourceFile.exists()) {
        if (!sourceFile.remove()) {
            return false;
        }
    }
    
    // 复制备份文件到原数据库位�?
    return backupFile.copy(sourceFile.fileName());
}
