
#include "DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

bool DatabaseManager::saveImapAccount(const EmailAccount& account) {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    if (account.id > 0) {
        query.prepare("UPDATE imap_accounts SET emailAddress = ?, displayName = ?, imapServer = ?, imapPort = ?, username = ?, password = ?, isDefault = ?, isActive = ?, lastUsedAt = ? WHERE id = ?");
        query.addBindValue(account.emailAddress);
        query.addBindValue(account.displayName);
        query.addBindValue(account.imapServer);
        query.addBindValue(account.imapPort);
        query.addBindValue(account.username);
        query.addBindValue(account.password);
        query.addBindValue(account.isDefault ? 1 : 0);
        query.addBindValue(account.isActive ? 1 : 0);
        query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
        query.addBindValue(account.id);
    } else {
        query.prepare("INSERT INTO imap_accounts (emailAddress, displayName, imapServer, imapPort, username, password, isDefault, isActive, createdAt, lastUsedAt) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(account.emailAddress);
        query.addBindValue(account.displayName);
        query.addBindValue(account.imapServer);
        query.addBindValue(account.imapPort);
        query.addBindValue(account.username);
        query.addBindValue(account.password);
        query.addBindValue(account.isDefault ? 1 : 0);
        query.addBindValue(account.isActive ? 1 : 0);
        query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
        query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    }

    bool success = query.exec();
    db.close();
    return success;
}

bool DatabaseManager::saveSmtpAccount(const EmailAccount& account) {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    if (account.id > 0) {
        query.prepare("UPDATE smtp_accounts SET emailAddress = ?, displayName = ?, smtpServer = ?, smtpPort = ?, username = ?, password = ?, isDefault = ?, isActive = ?, lastUsedAt = ? WHERE id = ?");
        query.addBindValue(account.emailAddress);
        query.addBindValue(account.displayName);
        query.addBindValue(account.smtpServer);
        query.addBindValue(account.smtpPort);
        query.addBindValue(account.username);
        query.addBindValue(account.password);
        query.addBindValue(account.isDefault ? 1 : 0);
        query.addBindValue(account.isActive ? 1 : 0);
        query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
        query.addBindValue(account.id);
    } else {
        query.prepare("INSERT INTO smtp_accounts (emailAddress, displayName, smtpServer, smtpPort, username, password, isDefault, isActive, createdAt, lastUsedAt) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(account.emailAddress);
        query.addBindValue(account.displayName);
        query.addBindValue(account.smtpServer);
        query.addBindValue(account.smtpPort);
        query.addBindValue(account.username);
        query.addBindValue(account.password);
        query.addBindValue(account.isDefault ? 1 : 0);
        query.addBindValue(account.isActive ? 1 : 0);
        query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
        query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    }

    bool success = query.exec();
    db.close();
    return success;
}

QVector<EmailAccount> DatabaseManager::getImapAccounts() {
    QVector<EmailAccount> accounts;

    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        qDebug() << "[DatabaseManager] Failed to open database connection!";
        return accounts;
    }
    
    qDebug() << "[DatabaseManager] Database connected successfully";
    qDebug() << "[DatabaseManager] Querying imap_accounts table...";

    QSqlQuery query;
    query.exec("SELECT * FROM imap_accounts WHERE isActive = 1");
    
    qDebug() << "[DatabaseManager] Query executed, numRowsAffected:" << query.numRowsAffected();
    qDebug() << "[DatabaseManager] Query lastError:" << query.lastError().text();

    while (query.next()) {
        EmailAccount account;
        account.id = query.value("id").toInt();
        account.emailAddress = query.value("emailAddress").toString();
        account.displayName = query.value("displayName").toString();
        account.imapServer = query.value("imapServer").toString();
        account.imapPort = query.value("imapPort").toInt();
        account.username = query.value("username").toString();
        account.password = query.value("password").toString();
        account.isDefault = query.value("isDefault").toBool();
        account.isActive = query.value("isActive").toBool();
        
        qDebug() << "[DatabaseManager] Row read:";
        qDebug() << "  id:" << account.id;
        qDebug() << "  emailAddress:" << account.emailAddress;
        qDebug() << "  username:" << account.username;
        qDebug() << "  password length:" << account.password.length();
        qDebug() << "  imapServer:" << account.imapServer;
        
        accounts.append(account);
    }
    
    qDebug() << "[DatabaseManager] Total accounts read:" << accounts.size();

    db.close();
    return accounts;
}

QVector<EmailAccount> DatabaseManager::getSmtpAccounts() {
    QVector<EmailAccount> accounts;

    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return accounts;
    }

    QSqlQuery query;
    query.exec("SELECT * FROM smtp_accounts WHERE isActive = 1");

    while (query.next()) {
        EmailAccount account;
        account.id = query.value("id").toInt();
        account.emailAddress = query.value("emailAddress").toString();
        account.displayName = query.value("displayName").toString();
        account.smtpServer = query.value("smtpServer").toString();
        account.smtpPort = query.value("smtpPort").toInt();
        account.username = query.value("username").toString();
        account.password = query.value("password").toString();
        account.isDefault = query.value("isDefault").toBool();
        account.isActive = query.value("isActive").toBool();
        accounts.append(account);
    }

    db.close();
    return accounts;
}

bool DatabaseManager::deleteImapAccount(int id) {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE imap_accounts SET isActive = 0 WHERE id = ?");
    query.addBindValue(id);

    bool success = query.exec();
    db.close();
    return success;
}

bool DatabaseManager::deleteSmtpAccount(int id) {
    QSqlDatabase db = getDatabaseConnection();
    if (!db.open()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE smtp_accounts SET isActive = 0 WHERE id = ?");
    query.addBindValue(id);

    bool success = query.exec();
    db.close();
    return success;
}
