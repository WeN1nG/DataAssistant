#include "DatabaseManager.h"
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>

DatabaseManager::DatabaseManager() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    dbPath = appDataPath + "/schedules.db";
    initializeDatabase();
}

DatabaseManager::~DatabaseManager() {
    QSqlDatabase::removeDatabase("QSQLITE");
}

bool DatabaseManager::initializeDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    // 添加color字段（如果不存在）
    query.exec("CREATE TABLE IF NOT EXISTS schedules (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT NOT NULL, description TEXT, datetime TEXT NOT NULL, priority INTEGER DEFAULT 0, reminderMinutes INTEGER DEFAULT 0, completed INTEGER DEFAULT 0, color TEXT DEFAULT '')");
    
    // 检查color字段是否存在，如果不存在则添加
    if (!query.exec("SELECT color FROM schedules LIMIT 1")) {
        query.exec("ALTER TABLE schedules ADD COLUMN color TEXT DEFAULT ''");
    }

    db.close();
    return true;
}

QColor DatabaseManager::getColorByPriority(int priority) {
    switch (priority) {
    case 2: return Qt::red;      // 紧急：红色
    case 1: return QColor(255, 165, 0);  // 重要：橙色
    default: return Qt::blue;    // 一般：蓝色
    }
}

bool DatabaseManager::addSchedule(const Schedule& schedule) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO schedules (title, description, datetime, priority, reminderMinutes, completed, color) VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(schedule.title);
    query.addBindValue(schedule.description);
    query.addBindValue(schedule.datetime.toString(Qt::ISODate));
    query.addBindValue(schedule.priority);
    query.addBindValue(schedule.reminderMinutes);
    query.addBindValue(schedule.completed ? 1 : 0);
    query.addBindValue(schedule.color.isValid() ? schedule.color.name() : getColorByPriority(schedule.priority).name());

    bool success = query.exec();
    db.close();
    return success;
}

bool DatabaseManager::updateSchedule(const Schedule& schedule) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE schedules SET title = ?, description = ?, datetime = ?, priority = ?, reminderMinutes = ?, completed = ?, color = ? WHERE id = ?");
    query.addBindValue(schedule.title);
    query.addBindValue(schedule.description);
    query.addBindValue(schedule.datetime.toString(Qt::ISODate));
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
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

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

QVector<Schedule> DatabaseManager::getSchedules() {
    QVector<Schedule> schedules;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        return schedules;
    }

    QSqlQuery query("SELECT * FROM schedules ORDER BY datetime ASC");
    while (query.next()) {
        Schedule schedule;
        schedule.id = query.value(0).toInt();
        schedule.title = query.value(1).toString();
        schedule.description = query.value(2).toString();
        schedule.datetime = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        schedule.priority = query.value(4).toInt();
        schedule.reminderMinutes = query.value(5).toInt();
        schedule.completed = query.value(6).toInt() == 1;
        
        // 读取颜色
        QString colorStr = query.value(7).toString();
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
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        return schedules;
    }

    QDateTime startOfDay = QDateTime(date, QTime(0, 0, 0));
    QDateTime endOfDay = QDateTime(date, QTime(23, 59, 59));

    QSqlQuery query;
    query.prepare("SELECT * FROM schedules WHERE datetime >= ? AND datetime <= ? ORDER BY datetime ASC");
    query.addBindValue(startOfDay.toString(Qt::ISODate));
    query.addBindValue(endOfDay.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            Schedule schedule;
            schedule.id = query.value(0).toInt();
            schedule.title = query.value(1).toString();
            schedule.description = query.value(2).toString();
            schedule.datetime = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
            schedule.priority = query.value(4).toInt();
            schedule.reminderMinutes = query.value(5).toInt();
            schedule.completed = query.value(6).toInt() == 1;
            
            // 读取颜色
            QString colorStr = query.value(7).toString();
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

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        return schedule;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM schedules WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        schedule.id = query.value(0).toInt();
        schedule.title = query.value(1).toString();
        schedule.description = query.value(2).toString();
        schedule.datetime = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        schedule.priority = query.value(4).toInt();
        schedule.reminderMinutes = query.value(5).toInt();
        schedule.completed = query.value(6).toInt() == 1;
        
        // 读取颜色
        QString colorStr = query.value(7).toString();
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
    
    // 关闭数据库连接
    QSqlDatabase::removeDatabase("QSQLITE");
    
    // 删除原数据库文件
    if (sourceFile.exists()) {
        if (!sourceFile.remove()) {
            return false;
        }
    }
    
    // 复制备份文件到原数据库位置
    return backupFile.copy(sourceFile.fileName());
}