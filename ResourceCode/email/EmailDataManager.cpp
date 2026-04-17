#include "EmailDataManager.h"
#include "../Database/DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFile>
#include <QDir>
#include <QVariant>
#include <QDebug>
#include <QStandardPaths>

EmailDataManager::EmailDataManager() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    if (!dir.exists()) {
        dir.mkpath(appDataPath);
    }
    dbPath = appDataPath + "/email_database.db";
}

EmailDataManager::~EmailDataManager() {
}

bool EmailDataManager::initializeDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "email_connection");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qWarning() << "无法打开邮箱数据库:" << db.lastError().text();
        return false;
    }

    return initializeTables();
}

bool EmailDataManager::initializeTables() {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.exec("CREATE TABLE IF NOT EXISTS email_accounts ("
              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
              "email_address TEXT NOT NULL UNIQUE, "
              "display_name TEXT, "
              "smtp_server TEXT, "
              "smtp_port INTEGER DEFAULT 587, "
              "imap_server TEXT, "
              "imap_port INTEGER DEFAULT 993, "
              "username TEXT NOT NULL, "
              "password TEXT, "
              "is_default INTEGER DEFAULT 0, "
              "is_active INTEGER DEFAULT 1, "
              "created_at TEXT, "
              "last_used_at TEXT"
              ")");

    query.exec("CREATE TABLE IF NOT EXISTS emails ("
              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
              "account_id INTEGER NOT NULL, "
              "message_id TEXT, "
              "subject TEXT, "
              "from_address TEXT, "
              "from_name TEXT, "
              "to_address TEXT NOT NULL, "
              "cc_addresses TEXT, "
              "bcc_addresses TEXT, "
              "body TEXT, "
              "html_body TEXT, "
              "attachments TEXT, "
              "folder INTEGER DEFAULT 0, "
              "is_read INTEGER DEFAULT 0, "
              "is_starred INTEGER DEFAULT 0, "
              "is_deleted INTEGER DEFAULT 0, "
              "priority INTEGER DEFAULT 1, "
              "sent_at TEXT, "
              "received_at TEXT, "
              "created_at TEXT, "
              "modified_at TEXT, "
              "raw_json TEXT, "
              "FOREIGN KEY(account_id) REFERENCES email_accounts(id) ON DELETE CASCADE"
              ")");

    query.exec("CREATE INDEX IF NOT EXISTS idx_emails_account_folder ON emails(account_id, folder)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_emails_message_id ON emails(message_id)");
    
    query.exec("PRAGMA table_info(emails)");
    bool hasRawJson = false;
    bool hasIsDeleted = false;
    while (query.next()) {
        QString columnName = query.value("name").toString();
        if (columnName == "raw_json") {
            hasRawJson = true;
        }
        if (columnName == "is_deleted") {
            hasIsDeleted = true;
        }
    }
    
    if (!hasRawJson) {
        qDebug() << "[DatabaseManager] Adding raw_json column to emails table...";
        query.exec("ALTER TABLE emails ADD COLUMN raw_json TEXT");
    }
    
    if (!hasIsDeleted) {
        qDebug() << "[DatabaseManager] Adding is_deleted column to emails table...";
        query.exec("ALTER TABLE emails ADD COLUMN is_deleted INTEGER DEFAULT 0");
    }

    return true;
}

bool EmailDataManager::addEmailAccount(const EmailAccount& account) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("INSERT INTO email_accounts (email_address, display_name, smtp_server, smtp_port, "
                  "imap_server, imap_port, username, password, is_default, is_active, created_at, last_used_at) "
                  "VALUES (:email_address, :display_name, :smtp_server, :smtp_port, "
                  ":imap_server, :imap_port, :username, :password, :is_default, :is_active, :created_at, :last_used_at)");

    query.bindValue(":email_address", account.emailAddress);
    query.bindValue(":display_name", account.displayName);
    query.bindValue(":smtp_server", account.smtpServer);
    query.bindValue(":smtp_port", account.smtpPort);
    query.bindValue(":imap_server", account.imapServer);
    query.bindValue(":imap_port", account.imapPort);
    query.bindValue(":username", account.username);
    query.bindValue(":password", account.password);
    query.bindValue(":is_default", account.isDefault ? 1 : 0);
    query.bindValue(":is_active", account.isActive ? 1 : 0);
    query.bindValue(":created_at", account.createdAt.isNull() ? QDateTime::currentDateTime() : account.createdAt);
    query.bindValue(":last_used_at", account.lastUsedAt.isNull() ? QDateTime::currentDateTime() : account.lastUsedAt);

    if (!query.exec()) {
        qWarning() << "添加邮箱账户失败:" << query.lastError().text();
        return false;
    }

    if (account.isDefault) {
        query.exec("UPDATE email_accounts SET is_default = 0 WHERE email_address != '" + account.emailAddress + "'");
    }

    return true;
}

bool EmailDataManager::updateEmailAccount(const EmailAccount& account) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("UPDATE email_accounts SET email_address = :email_address, display_name = :display_name, "
                  "smtp_server = :smtp_server, smtp_port = :smtp_port, imap_server = :imap_server, "
                  "imap_port = :imap_port, username = :username, password = :password, "
                  "is_default = :is_default, is_active = :is_active, last_used_at = :last_used_at "
                  "WHERE id = :id");

    query.bindValue(":id", account.id);
    query.bindValue(":email_address", account.emailAddress);
    query.bindValue(":display_name", account.displayName);
    query.bindValue(":smtp_server", account.smtpServer);
    query.bindValue(":smtp_port", account.smtpPort);
    query.bindValue(":imap_server", account.imapServer);
    query.bindValue(":imap_port", account.imapPort);
    query.bindValue(":username", account.username);
    query.bindValue(":password", account.password);
    query.bindValue(":is_default", account.isDefault ? 1 : 0);
    query.bindValue(":is_active", account.isActive ? 1 : 0);
    query.bindValue(":last_used_at", QDateTime::currentDateTime());

    if (!query.exec()) {
        qWarning() << "更新邮箱账户失败:" << query.lastError().text();
        return false;
    }

    if (account.isDefault) {
        query.exec("UPDATE email_accounts SET is_default = 0 WHERE id != " + QString::number(account.id));
    }

    return true;
}

bool EmailDataManager::deleteEmailAccount(int accountId) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("DELETE FROM email_accounts WHERE id = :id");
    query.bindValue(":id", accountId);

    if (!query.exec()) {
        qWarning() << "删除邮箱账户失败:" << query.lastError().text();
        return false;
    }

    return true;
}

EmailAccount EmailDataManager::getEmailAccount(int accountId) {
    DatabaseManager dbManager;
    QVector<EmailAccount> imapAccounts = dbManager.getImapAccounts();
    
    for (const EmailAccount& account : imapAccounts) {
        if (account.id == accountId) {
            return account;
        }
    }
    
    QVector<EmailAccount> smtpAccounts = dbManager.getSmtpAccounts();
    for (const EmailAccount& account : smtpAccounts) {
        if (account.id == accountId) {
            return account;
        }
    }

    return EmailAccount();
}

QVector<EmailAccount> EmailDataManager::getAllEmailAccounts() {
    DatabaseManager dbManager;
    QVector<EmailAccount> accounts = dbManager.getImapAccounts();
    
    QVector<EmailAccount> smtpAccounts = dbManager.getSmtpAccounts();
    accounts.append(smtpAccounts);

    return accounts;
}

EmailAccount EmailDataManager::getDefaultEmailAccount() {
    DatabaseManager dbManager;
    QVector<EmailAccount> imapAccounts = dbManager.getImapAccounts();
    
    for (const EmailAccount& account : imapAccounts) {
        if (account.isDefault && account.isActive) {
            return account;
        }
    }
    
    QVector<EmailAccount> smtpAccounts = dbManager.getSmtpAccounts();
    for (const EmailAccount& account : smtpAccounts) {
        if (account.isDefault && account.isActive) {
            return account;
        }
    }
    
    for (const EmailAccount& account : imapAccounts) {
        if (account.isActive) {
            return account;
        }
    }
    
    for (const EmailAccount& account : smtpAccounts) {
        if (account.isActive) {
            return account;
        }
    }

    return EmailAccount();
}

bool EmailDataManager::addEmail(const Email& email) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("INSERT INTO emails (account_id, message_id, subject, from_address, from_name, "
                  "to_address, cc_addresses, bcc_addresses, body, html_body, attachments, "
                  "folder, is_read, is_starred, is_deleted, priority, sent_at, received_at, created_at, modified_at, raw_json) "
                  "VALUES (:account_id, :message_id, :subject, :from_address, :from_name, "
                  ":to_address, :cc_addresses, :bcc_addresses, :body, :html_body, :attachments, "
                  ":folder, :is_read, :is_starred, :is_deleted, :priority, :sent_at, :received_at, :created_at, :modified_at, :raw_json)");

    query.bindValue(":account_id", email.accountId);
    query.bindValue(":message_id", email.messageId);
    query.bindValue(":subject", email.subject);
    query.bindValue(":from_address", email.fromAddress);
    query.bindValue(":from_name", email.fromName);
    query.bindValue(":to_address", email.toAddress);
    query.bindValue(":cc_addresses", email.ccAddresses.join(";"));
    query.bindValue(":bcc_addresses", email.bccAddresses.join(";"));
    query.bindValue(":is_deleted", email.isDeleted ? 1 : 0);
    query.bindValue(":body", email.body);
    query.bindValue(":html_body", email.htmlBody);
    
    QStringList attachmentFilenames;
    for (const EmailAttachment& att : email.attachments) {
        attachmentFilenames.append(att.filename);
    }
    query.bindValue(":attachments", attachmentFilenames.join("|"));
    query.bindValue(":folder", static_cast<int>(email.folder));
    query.bindValue(":is_read", email.isRead ? 1 : 0);
    query.bindValue(":is_starred", email.isStarred ? 1 : 0);
    query.bindValue(":priority", static_cast<int>(email.priority));
    query.bindValue(":sent_at", email.sentAt);
    query.bindValue(":received_at", email.receivedAt);
    query.bindValue(":created_at", email.createdAt.isNull() ? QDateTime::currentDateTime() : email.createdAt);
    query.bindValue(":modified_at", email.modifiedAt.isNull() ? QDateTime::currentDateTime() : email.modifiedAt);
    query.bindValue(":raw_json", email.rawJson);

    if (!query.exec()) {
        qWarning() << "添加邮件失败:" << query.lastError().text();
        return false;
    }

    return true;
}

bool EmailDataManager::updateEmail(const Email& email) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("UPDATE emails SET subject = :subject, to_address = :to_address, "
                  "cc_addresses = :cc_addresses, bcc_addresses = :bcc_addresses, body = :body, "
                  "html_body = :html_body, attachments = :attachments, is_read = :is_read, "
                  "is_starred = :is_starred, is_deleted = :is_deleted, priority = :priority, modified_at = :modified_at "
                  "WHERE id = :id");

    query.bindValue(":id", email.id);
    query.bindValue(":subject", email.subject);
    query.bindValue(":to_address", email.toAddress);
    query.bindValue(":cc_addresses", email.ccAddresses.join(";"));
    query.bindValue(":bcc_addresses", email.bccAddresses.join(";"));
    query.bindValue(":body", email.body);
    query.bindValue(":html_body", email.htmlBody);
    
    QStringList attachmentFilenames;
    for (const EmailAttachment& att : email.attachments) {
        attachmentFilenames.append(att.filename);
    }
    query.bindValue(":attachments", attachmentFilenames.join("|"));
    query.bindValue(":is_read", email.isRead ? 1 : 0);
    query.bindValue(":is_starred", email.isStarred ? 1 : 0);
    query.bindValue(":is_deleted", email.isDeleted ? 1 : 0);
    query.bindValue(":priority", static_cast<int>(email.priority));
    query.bindValue(":modified_at", QDateTime::currentDateTime());

    if (!query.exec()) {
        qWarning() << "更新邮件失败:" << query.lastError().text();
        return false;
    }

    return true;
}

bool EmailDataManager::deleteEmail(int emailId, bool permanent) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    if (permanent) {
        query.prepare("DELETE FROM emails WHERE id = :id");
        query.bindValue(":id", emailId);
    } else {
        query.prepare("UPDATE emails SET folder = :folder, modified_at = :modified_at WHERE id = :id");
        query.bindValue(":id", emailId);
        query.bindValue(":folder", static_cast<int>(EmailFolder::Trash));
        query.bindValue(":modified_at", QDateTime::currentDateTime());
    }

    if (!query.exec()) {
        qWarning() << "删除邮件失败:" << query.lastError().text();
        return false;
    }

    return true;
}

bool EmailDataManager::moveEmail(int emailId, EmailFolder targetFolder) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("UPDATE emails SET folder = :folder, modified_at = :modified_at WHERE id = :id");
    query.bindValue(":id", emailId);
    query.bindValue(":folder", static_cast<int>(targetFolder));
    query.bindValue(":modified_at", QDateTime::currentDateTime());

    if (!query.exec()) {
        qWarning() << "移动邮件失败:" << query.lastError().text();
        return false;
    }

    return true;
}

Email EmailDataManager::getEmailById(int emailId) {
    qDebug() << "[EmailDataManager] getEmailById: START - emailId=" << emailId;
    
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    qDebug() << "[EmailDataManager] getEmailById: Database connection name:" << db.connectionName();
    qDebug() << "[EmailDataManager] getEmailById: Database isOpen:" << db.isOpen();
    
    QSqlQuery query(db);

    query.prepare("SELECT * FROM emails WHERE id = :id");
    query.bindValue(":id", emailId);

    qDebug() << "[EmailDataManager] getEmailById: Executing query...";
    
    if (query.exec()) {
        qDebug() << "[EmailDataManager] getEmailById: Query executed successfully";
        qDebug() << "[EmailDataManager] getEmailById: query.numRowsAffected:" << query.numRowsAffected();
        qDebug() << "[EmailDataManager] getEmailById: query.next():" << query.next();
        
        if (query.next()) {
            Email email = parseEmailFromQuery(query);
            qDebug() << "[EmailDataManager] getEmailById: Email found - id:" << email.id << "subject:" << email.subject;
            qDebug() << "[EmailDataManager] getEmailById: Email body.length:" << email.body.length() << "htmlBody.length:" << email.htmlBody.length();
            return email;
        } else {
            qDebug() << "[EmailDataManager] getEmailById: No email found for id:" << emailId;
        }
    } else {
        qDebug() << "[ERROR] EmailDataManager::getEmailById: Query execution failed!";
        qDebug() << "[ERROR] EmailDataManager::getEmailById: lastError:" << query.lastError().text();
        qDebug() << "[ERROR] EmailDataManager::getEmailById: lastQuery:" << query.lastQuery();
    }

    qDebug() << "[EmailDataManager] getEmailById: END - returning empty Email";
    return Email();
}

QVector<Email> EmailDataManager::getEmailsByFolder(int accountId, EmailFolder folder) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    qDebug() << "[DEBUG] getEmailsByFolder: accountId=" << accountId << "folder=" << static_cast<int>(folder);

    query.prepare("SELECT * FROM emails WHERE account_id = :account_id AND folder = :folder ORDER BY sent_at DESC");
    query.bindValue(":account_id", accountId);
    query.bindValue(":folder", static_cast<int>(folder));

    if (!query.exec()) {
        qDebug() << "[ERROR] getEmailsByFolder query failed:" << query.lastError().text();
        return QVector<Email>();
    }

    qDebug() << "[DEBUG] getEmailsByFolder: query executed, numRowsAffected=" << query.numRowsAffected();

    QVector<Email> emails;
    while (query.next()) {
        emails.append(parseEmailFromQuery(query));
    }

    qDebug() << "[DEBUG] getEmailsByFolder: returning" << emails.size() << "emails";

    return emails;
}

QVector<Email> EmailDataManager::getEmailsByFilter(const EmailFilter& filter) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    QString sql = "SELECT * FROM emails WHERE account_id = :account_id ";
    QMap<QString, QVariant> params;

    params[":account_id"] = filter.accountId;

    if (filter.folder != EmailFolder::Inbox) {
        sql += " AND folder = :folder ";
        params[":folder"] = static_cast<int>(filter.folder);
    }

    if (!filter.searchText.isEmpty()) {
        sql += " AND (subject LIKE :search OR body LIKE :search) ";
        params[":search"] = "%" + filter.searchText + "%";
    }

    if (!filter.fromFilter.isEmpty()) {
        sql += " AND from_address LIKE :from_filter ";
        params[":from_filter"] = "%" + filter.fromFilter + "%";
    }

    if (!filter.toFilter.isEmpty()) {
        sql += " AND to_address LIKE :to_filter ";
        params[":to_filter"] = "%" + filter.toFilter + "%";
    }

    if (filter.unreadOnly) {
        sql += " AND is_read = 0 ";
    }

    if (filter.starredOnly) {
        sql += " AND is_starred = 1 ";
    }

    sql += " ORDER BY sent_at DESC";

    query.prepare(sql);
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(it.key(), it.value());
    }

    QVector<Email> emails;
    if (query.exec()) {
        while (query.next()) {
            emails.append(parseEmailFromQuery(query));
        }
    }

    return emails;
}

bool EmailDataManager::markEmailAsRead(int emailId) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("UPDATE emails SET is_read = 1, modified_at = :modified_at WHERE id = :id");
    query.bindValue(":id", emailId);
    query.bindValue(":modified_at", QDateTime::currentDateTime());

    return query.exec();
}

bool EmailDataManager::markEmailAsUnread(int emailId) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("UPDATE emails SET is_read = 0, modified_at = :modified_at WHERE id = :id");
    query.bindValue(":id", emailId);
    query.bindValue(":modified_at", QDateTime::currentDateTime());

    return query.exec();
}

bool EmailDataManager::toggleEmailStar(int emailId) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("UPDATE emails SET is_starred = NOT is_starred, modified_at = :modified_at WHERE id = :id");
    query.bindValue(":id", emailId);
    query.bindValue(":modified_at", QDateTime::currentDateTime());

    return query.exec();
}

int EmailDataManager::getUnreadCount(int accountId, EmailFolder folder) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("SELECT COUNT(*) FROM emails WHERE account_id = :account_id AND folder = :folder AND is_read = 0");
    query.bindValue(":account_id", accountId);
    query.bindValue(":folder", static_cast<int>(folder));

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

QMap<EmailFolder, int> EmailDataManager::getFolderCounts(int accountId) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    QMap<EmailFolder, int> counts;

    for (int i = 0; i <= 3; ++i) {
        EmailFolder folder = static_cast<EmailFolder>(i);
        query.prepare("SELECT COUNT(*) FROM emails WHERE account_id = :account_id AND folder = :folder");
        query.bindValue(":account_id", accountId);
        query.bindValue(":folder", i);

        if (query.exec() && query.next()) {
            counts[folder] = query.value(0).toInt();
        } else {
            counts[folder] = 0;
        }
    }

    return counts;
}

bool EmailDataManager::saveDraft(const Email& email) {
    Email draft = email;
    draft.folder = EmailFolder::Draft;
    draft.modifiedAt = QDateTime::currentDateTime();

    if (email.id > 0) {
        return updateEmail(draft);
    } else {
        return addEmail(draft);
    }
}

Email EmailDataManager::getDraft(int accountId) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("SELECT * FROM emails WHERE account_id = :account_id AND folder = :folder ORDER BY modified_at DESC LIMIT 1");
    query.bindValue(":account_id", accountId);
    query.bindValue(":folder", static_cast<int>(EmailFolder::Draft));

    if (query.exec() && query.next()) {
        return parseEmailFromQuery(query);
    }

    return Email();
}

EmailAccount EmailDataManager::parseAccountFromQuery(const QSqlQuery& query) {
    EmailAccount account;
    account.id = query.value("id").toInt();
    account.emailAddress = query.value("email_address").toString();
    account.displayName = query.value("display_name").toString();
    account.smtpServer = query.value("smtp_server").toString();
    account.smtpPort = query.value("smtp_port").toInt();
    account.imapServer = query.value("imap_server").toString();
    account.imapPort = query.value("imap_port").toInt();
    account.username = query.value("username").toString();
    account.password = query.value("password").toString();
    account.isDefault = query.value("is_default").toBool();
    account.isActive = query.value("is_active").toBool();
    account.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    account.lastUsedAt = QDateTime::fromString(query.value("last_used_at").toString(), Qt::ISODate);
    return account;
}

Email EmailDataManager::parseEmailFromQuery(const QSqlQuery& query) {
    Email email;
    email.id = query.value("id").toInt();
    email.accountId = query.value("account_id").toInt();
    email.messageId = query.value("message_id").toString();
    email.subject = query.value("subject").toString();
    email.fromAddress = query.value("from_address").toString();
    email.fromName = query.value("from_name").toString();
    email.toAddress = query.value("to_address").toString();
    email.ccAddresses = query.value("cc_addresses").toString().split(";", Qt::SkipEmptyParts);
    email.bccAddresses = query.value("bcc_addresses").toString().split(";", Qt::SkipEmptyParts);
    email.body = query.value("body").toString();
    email.htmlBody = query.value("html_body").toString();
    
    QStringList attachmentFilenames = query.value("attachments").toString().split("|", Qt::SkipEmptyParts);
    for (const QString& filename : attachmentFilenames) {
        EmailAttachment att;
        att.filename = filename;
        att.size = 0;
        email.attachments.append(att);
    }
    
    email.folder = static_cast<EmailFolder>(query.value("folder").toInt());
    email.isRead = query.value("is_read").toBool();
    email.isStarred = query.value("is_starred").toBool();
    email.isDeleted = query.value("is_deleted").toBool();
    email.priority = static_cast<EmailPriority>(query.value("priority").toInt());
    email.sentAt = QDateTime::fromString(query.value("sent_at").toString(), Qt::ISODate);
    email.receivedAt = QDateTime::fromString(query.value("received_at").toString(), Qt::ISODate);
    email.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    email.modifiedAt = QDateTime::fromString(query.value("modified_at").toString(), Qt::ISODate);
    email.rawJson = query.value("raw_json").toString();
    return email;
}

QSqlDatabase EmailDataManager::getEmailBoxDatabase(const QString& connectionName, const QString& dbPath, bool& success) {
    success = false;
    
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase existingDb = QSqlDatabase::database(connectionName);
        if (existingDb.isOpen()) {
            existingDb.close();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }
    
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        qWarning() << "[EmailDataManager] getEmailBoxDatabase: Failed to open database:" << dbPath;
        QSqlDatabase::removeDatabase(connectionName);
        return QSqlDatabase();
    }
    
    success = true;
    return db;
}

void EmailDataManager::closeEmailBoxDatabase(const QString& connectionName) {
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if (db.isOpen()) {
            db.close();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }
}

bool EmailDataManager::clearAllEmailsForAccount(int accountId) {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("DELETE FROM emails WHERE account_id = :account_id");
    query.bindValue(":account_id", accountId);

    if (!query.exec()) {
        qWarning() << "清空账户邮件失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "[EmailDataManager] 已清空账户" << accountId << "的所有邮件";
    return true;
}

bool EmailDataManager::clearAllEmails() {
    QSqlDatabase db = QSqlDatabase::database("email_connection");
    QSqlQuery query(db);

    query.prepare("DELETE FROM emails");

    if (!query.exec()) {
        qWarning() << "清空所有邮件失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "[EmailDataManager] 已清空所有邮件";
    return true;
}

bool EmailDataManager::IniAllEmailDatabaseFiles() {
    qDebug() << "[EmailDataManager] IniAllEmailDatabaseFiles: 开始初始化所有邮件数据库文件";

    QVector<EmailAccount> accounts = getAllEmailAccounts();

    if (accounts.isEmpty()) {
        qDebug() << "[EmailDataManager] IniAllEmailDatabaseFiles: 没有找到任何活动账户";
        return true;
    }

    int successCount = 0;
    int failCount = 0;

    for (const EmailAccount& account : accounts) {
        if (createEmailBoxForAccount(account.emailAddress)) {
            successCount++;
            qDebug() << "[EmailDataManager] IniAllEmailDatabaseFiles: 已初始化账户" << account.emailAddress;
        } else {
            failCount++;
            qWarning() << "[EmailDataManager] IniAllEmailDatabaseFiles: 初始化账户失败" << account.emailAddress;
        }
    }

    qDebug() << "[EmailDataManager] IniAllEmailDatabaseFiles: 完成，成功" << successCount << "个，失败" << failCount << "个";

    return failCount == 0;
}

bool EmailDataManager::clearEmailBoxForAccount(const QString& emailAddress) {
    if (emailAddress.isEmpty()) {
        qWarning() << "[EmailDataManager] clearEmailBoxForAccount: emailAddress为空";
        return false;
    }

    QString sanitizedEmail = emailAddress;
    sanitizedEmail.replace("@", "_at_");
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString emailBoxPath = appDataPath + "/" + sanitizedEmail + "EmailBox.db";

    QString connectionName = "email_box_clear_" + sanitizedEmail;
    bool dbSuccess = false;
    QSqlDatabase emailBoxDb = getEmailBoxDatabase(connectionName, emailBoxPath, dbSuccess);

    if (!dbSuccess) {
        qWarning() << "[EmailDataManager] clearEmailBoxForAccount: 无法打开邮箱数据库";
        return false;
    }

    QSqlQuery query(emailBoxDb);
    query.prepare("DELETE FROM emails");

    if (!query.exec()) {
        qWarning() << "[EmailDataManager] clearEmailBoxForAccount: 清空邮件失败:" << query.lastError().text();
        query.finish();
        query.clear();
        closeEmailBoxDatabase(connectionName);
        return false;
    }

    qDebug() << "[EmailDataManager] 已清空邮箱数据:" << emailAddress;
    query.finish();
    query.clear();
    closeEmailBoxDatabase(connectionName);

    return true;
}

bool EmailDataManager::IniEmailBoxForAccount(const QString& emailAddress) {
    if (emailAddress.isEmpty()) {
        qWarning() << "[EmailDataManager] IniEmailBoxForAccount: emailAddress为空";
        return false;
    }

    QString sanitizedEmail = emailAddress;
    sanitizedEmail.replace("@", "_at_");
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString emailBoxPath = appDataPath + "/" + sanitizedEmail + "EmailBox.db";

    QFile emailBoxFile(emailBoxPath);
    if (emailBoxFile.exists()) {
        if (emailBoxFile.remove()) {
            qDebug() << "[EmailDataManager] 已删除邮箱文件:" << emailBoxPath;
        } else {
            qWarning() << "[EmailDataManager] 无法删除邮箱文件:" << emailBoxPath;
            return false;
        }
    }

    createEmailBoxForAccount(emailAddress);
    
    return true;
}

bool EmailDataManager::createEmailBoxForAccount(const QString& emailAddress) {
    if (emailAddress.isEmpty()) {
        qWarning() << "[EmailDataManager] createEmailBoxForAccount: emailAddress为空";
        return false;
    }

    QString sanitizedEmail = emailAddress;
    sanitizedEmail.replace("@", "_at_");
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString emailBoxPath = appDataPath + "/" + sanitizedEmail + "EmailBox.db";

    QString connectionName = "email_box_" + sanitizedEmail;
    bool dbSuccess = false;
    QSqlDatabase emailBoxDb = getEmailBoxDatabase(connectionName, emailBoxPath, dbSuccess);

    if (!dbSuccess) {
        qWarning() << "[EmailDataManager] 无法创建邮箱数据库";
        return false;
    }

    QSqlQuery query(emailBoxDb);

    query.exec("CREATE TABLE IF NOT EXISTS emails ("
              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
              "account_id INTEGER NOT NULL, "
              "message_id TEXT, "
              "subject TEXT, "
              "from_address TEXT, "
              "from_name TEXT, "
              "to_address TEXT NOT NULL, "
              "cc_addresses TEXT, "
              "bcc_addresses TEXT, "
              "body TEXT, "
              "html_body TEXT, "
              "attachments TEXT, "
              "folder INTEGER DEFAULT 0, "
              "is_read INTEGER DEFAULT 0, "
              "is_starred INTEGER DEFAULT 0, "
              "is_deleted INTEGER DEFAULT 0, "
              "priority INTEGER DEFAULT 1, "
              "sent_at TEXT, "
              "received_at TEXT, "
              "created_at TEXT, "
              "modified_at TEXT, "
              "raw_json TEXT"
              ")");

    query.exec("CREATE INDEX IF NOT EXISTS idx_emails_account_folder ON emails(account_id, folder)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_emails_message_id ON emails(message_id)");

    qDebug() << "[EmailDataManager] 已创建邮箱数据库:" << emailBoxPath;
    query.finish();
    query.clear();
    closeEmailBoxDatabase(connectionName);

    return true;
}

bool EmailDataManager::saveEmailToEmailBox(const Email& email, const QString& emailAddress) {
    if (emailAddress.isEmpty()) {
        qWarning() << "[EmailDataManager] saveEmailToEmailBox: emailAddress为空";
        return false;
    }

    QString sanitizedEmail = emailAddress;
    sanitizedEmail.replace("@", "_at_");
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString emailBoxPath = appDataPath + "/" + sanitizedEmail + "EmailBox.db";

    QString connectionName = "email_box_" + sanitizedEmail;
    bool dbSuccess = false;
    QSqlDatabase emailBoxDb = getEmailBoxDatabase(connectionName, emailBoxPath, dbSuccess);

    if (!dbSuccess) {
        qWarning() << "[EmailDataManager] 无法打开邮箱数据库";
        return false;
    }

    QSqlQuery checkQuery(emailBoxDb);
    checkQuery.prepare("SELECT id FROM emails WHERE message_id = :message_id AND account_id = :account_id");
    checkQuery.bindValue(":message_id", email.messageId);
    checkQuery.bindValue(":account_id", email.accountId);
    
    if (checkQuery.exec() && checkQuery.next()) {
        qDebug() << "[EmailDataManager] saveEmailToEmailBox: 邮件已存在，跳过插入 - message_id:" << email.messageId;
        checkQuery.finish();
        checkQuery.clear();
        closeEmailBoxDatabase(connectionName);
        return true;
    }

    QSqlQuery query(emailBoxDb);

    query.prepare("INSERT INTO emails (account_id, message_id, subject, from_address, from_name, "
                  "to_address, cc_addresses, bcc_addresses, body, html_body, attachments, "
                  "folder, is_read, is_starred, is_deleted, priority, sent_at, received_at, created_at, modified_at, raw_json) "
                  "VALUES (:account_id, :message_id, :subject, :from_address, :from_name, "
                  ":to_address, :cc_addresses, :bcc_addresses, :body, :html_body, :attachments, "
                  ":folder, :is_read, :is_starred, :is_deleted, :priority, :sent_at, :received_at, :created_at, :modified_at, :raw_json)");

    query.bindValue(":account_id", email.accountId);
    query.bindValue(":message_id", email.messageId);
    query.bindValue(":subject", email.subject);
    query.bindValue(":from_address", email.fromAddress);
    query.bindValue(":from_name", email.fromName);
    query.bindValue(":to_address", email.toAddress);
    query.bindValue(":cc_addresses", email.ccAddresses.join(";"));
    query.bindValue(":bcc_addresses", email.bccAddresses.join(";"));
    query.bindValue(":body", email.body);
    query.bindValue(":html_body", email.htmlBody);

    QStringList attachmentFilenames;
    for (const EmailAttachment& att : email.attachments) {
        attachmentFilenames.append(att.filename);
    }
    query.bindValue(":attachments", attachmentFilenames.join("|"));
    query.bindValue(":folder", static_cast<int>(email.folder));
    query.bindValue(":is_read", email.isRead ? 1 : 0);
    query.bindValue(":is_starred", email.isStarred ? 1 : 0);
    query.bindValue(":is_deleted", email.isDeleted ? 1 : 0);
    query.bindValue(":priority", static_cast<int>(email.priority));
    query.bindValue(":sent_at", email.sentAt);
    query.bindValue(":received_at", email.receivedAt);
    query.bindValue(":created_at", email.createdAt.isNull() ? QDateTime::currentDateTime() : email.createdAt);
    query.bindValue(":modified_at", email.modifiedAt.isNull() ? QDateTime::currentDateTime() : email.modifiedAt);
    query.bindValue(":raw_json", email.rawJson);

    if (!query.exec()) {
        qWarning() << "[EmailDataManager] 保存邮件失败:" << query.lastError().text();
        checkQuery.finish();
        checkQuery.clear();
        query.finish();
        query.clear();
        closeEmailBoxDatabase(connectionName);
        return false;
    }

    checkQuery.finish();
    checkQuery.clear();
    query.finish();
    query.clear();
    closeEmailBoxDatabase(connectionName);

    return true;
}

QVector<Email> EmailDataManager::getEmailsFromEmailBox(const QString& emailAddress) {
    QVector<Email> emails;

    if (emailAddress.isEmpty()) {
        qWarning() << "[EmailDataManager] getEmailsFromEmailBox: emailAddress为空";
        return emails;
    }

    QString sanitizedEmail = emailAddress;
    sanitizedEmail.replace("@", "_at_");
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString emailBoxPath = appDataPath + "/" + sanitizedEmail + "EmailBox.db";

    QString connectionName = "email_box_" + sanitizedEmail;
    bool dbSuccess = false;
    QSqlDatabase emailBoxDb = getEmailBoxDatabase(connectionName, emailBoxPath, dbSuccess);

    if (!dbSuccess) {
        qWarning() << "[EmailDataManager] 无法打开邮箱数据库";
        return emails;
    }

    QSqlQuery query(emailBoxDb);
    query.prepare("SELECT * FROM emails WHERE folder = :folder ORDER BY sent_at DESC");
    query.bindValue(":folder", static_cast<int>(EmailFolder::Inbox));

    if (query.exec()) {
        while (query.next()) {
            emails.append(parseEmailFromQuery(query));
        }
    } else {
        qWarning() << "[EmailDataManager] 查询邮箱邮件失败:" << query.lastError().text();
    }

    query.finish();
    query.clear();
    closeEmailBoxDatabase(connectionName);

    return emails;
}

bool EmailDataManager::mergeEmailBoxesToAll(const QVector<EmailAccount>& accounts) {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString allEmailBoxPath = appDataPath + "/AllEmailBox.db";

    QFile allEmailBoxFile(allEmailBoxPath);
    if (allEmailBoxFile.exists()) {
        allEmailBoxFile.remove();
    }

    QSqlDatabase allEmailBoxDb = QSqlDatabase::addDatabase("QSQLITE", "all_email_box");
    allEmailBoxDb.setDatabaseName(allEmailBoxPath);

    if (!allEmailBoxDb.open()) {
        qWarning() << "[EmailDataManager] 无法创建AllEmailBox数据库:" << allEmailBoxDb.lastError().text();
        return false;
    }

    QSqlQuery query(allEmailBoxDb);

    query.exec("CREATE TABLE IF NOT EXISTS emails ("
              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
              "account_id INTEGER NOT NULL, "
              "message_id TEXT, "
              "subject TEXT, "
              "from_address TEXT, "
              "from_name TEXT, "
              "to_address TEXT NOT NULL, "
              "cc_addresses TEXT, "
              "bcc_addresses TEXT, "
              "body TEXT, "
              "html_body TEXT, "
              "attachments TEXT, "
              "folder INTEGER DEFAULT 0, "
              "is_read INTEGER DEFAULT 0, "
              "is_starred INTEGER DEFAULT 0, "
              "is_deleted INTEGER DEFAULT 0, "
              "priority INTEGER DEFAULT 1, "
              "sent_at TEXT, "
              "received_at TEXT, "
              "created_at TEXT, "
              "modified_at TEXT, "
              "raw_json TEXT"
              ")");

    query.exec("CREATE INDEX IF NOT EXISTS idx_emails_account_folder ON emails(account_id, folder)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_emails_message_id ON emails(message_id)");

    for (const EmailAccount& account : accounts) {
        QString sanitizedEmail = account.emailAddress;
        sanitizedEmail.replace("@", "_at_");
        QString emailBoxPath = appDataPath + "/" + sanitizedEmail + "EmailBox.db";

        QString connectionName = "email_box_" + sanitizedEmail;

        QSqlDatabase emailBoxDb;
        if (QSqlDatabase::contains(connectionName)) {
            emailBoxDb = QSqlDatabase::database(connectionName);
            if (!emailBoxDb.isOpen()) {
                emailBoxDb.open();
            }
        } else {
            emailBoxDb = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            emailBoxDb.setDatabaseName(emailBoxPath);
            if (!emailBoxDb.open()) {
                qWarning() << "[EmailDataManager] 无法打开邮箱数据库:" << emailBoxDb.lastError().text();
                continue;
            }
        }

        QSqlQuery selectQuery(emailBoxDb);
        selectQuery.prepare("SELECT * FROM emails WHERE folder = :folder ORDER BY sent_at DESC");
        selectQuery.bindValue(":folder", static_cast<int>(EmailFolder::Inbox));

        if (selectQuery.exec()) {
            while (selectQuery.next()) {
                Email email = parseEmailFromQuery(selectQuery);

                QSqlQuery insertQuery(allEmailBoxDb);
                insertQuery.prepare("INSERT INTO emails (account_id, message_id, subject, from_address, from_name, "
                                   "to_address, cc_addresses, bcc_addresses, body, html_body, attachments, "
                                   "folder, is_read, is_starred, is_deleted, priority, sent_at, received_at, created_at, modified_at, raw_json) "
                                   "VALUES (:account_id, :message_id, :subject, :from_address, :from_name, "
                                   ":to_address, :cc_addresses, :bcc_addresses, :body, :html_body, :attachments, "
                                   ":folder, :is_read, :is_starred, :is_deleted, :priority, :sent_at, :received_at, :created_at, :modified_at, :raw_json)");

                insertQuery.bindValue(":account_id", email.accountId);
                insertQuery.bindValue(":message_id", email.messageId);
                insertQuery.bindValue(":subject", email.subject);
                insertQuery.bindValue(":from_address", email.fromAddress);
                insertQuery.bindValue(":from_name", email.fromName);
                insertQuery.bindValue(":to_address", email.toAddress);
                insertQuery.bindValue(":cc_addresses", email.ccAddresses.join(";"));
                insertQuery.bindValue(":bcc_addresses", email.bccAddresses.join(";"));
                insertQuery.bindValue(":body", email.body);
                insertQuery.bindValue(":html_body", email.htmlBody);

                QStringList attachmentFilenames;
                for (const EmailAttachment& att : email.attachments) {
                    attachmentFilenames.append(att.filename);
                }
                insertQuery.bindValue(":attachments", attachmentFilenames.join("|"));
                insertQuery.bindValue(":folder", static_cast<int>(email.folder));
                insertQuery.bindValue(":is_read", email.isRead ? 1 : 0);
                insertQuery.bindValue(":is_starred", email.isStarred ? 1 : 0);
                insertQuery.bindValue(":is_deleted", email.isDeleted ? 1 : 0);
                insertQuery.bindValue(":priority", static_cast<int>(email.priority));
                insertQuery.bindValue(":sent_at", email.sentAt);
                insertQuery.bindValue(":received_at", email.receivedAt);
                insertQuery.bindValue(":created_at", email.createdAt.isNull() ? QDateTime::currentDateTime() : email.createdAt);
                insertQuery.bindValue(":modified_at", email.modifiedAt.isNull() ? QDateTime::currentDateTime() : email.modifiedAt);
                insertQuery.bindValue(":raw_json", email.rawJson);

                insertQuery.exec();
            }
        }
    }

    qDebug() << "[EmailDataManager] 已合并所有邮箱邮件到AllEmailBox";
    allEmailBoxDb.close();

    return true;
}

bool EmailDataManager::prepareEmailBoxForRefresh(const QString& emailAddress) {
    qDebug() << "[EmailDataManager] prepareEmailBoxForRefresh:" << emailAddress;
    
    // 步骤1：清空现有的EmailBox
    if (!clearEmailBoxForAccount(emailAddress)) {
        qDebug() << "[EmailDataManager] Failed to clear EmailBox for:" << emailAddress;
        return false;
    }
    
    // 步骤2：创建新的空EmailBox（确保数据库结构存在）
    if (!createEmailBoxForAccount(emailAddress)) {
        qDebug() << "[EmailDataManager] Failed to create EmailBox for:" << emailAddress;
        return false;
    }
    
    qDebug() << "[EmailDataManager] EmailBox prepared successfully for:" << emailAddress;
    return true;
}

QVector<Email> EmailDataManager::loadEmailsFromEmailBox(const QString& emailAddress) {
    qDebug() << "[EmailDataManager] loadEmailsFromEmailBox:" << emailAddress;
    
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString sanitizedEmail = emailAddress;
    sanitizedEmail.replace("@", "_at_");
    QString dbFileName = QString("/%1EmailBox.db").arg(sanitizedEmail);
    QString dbPath = appDataPath + dbFileName;
    
    if (!QFile::exists(dbPath)) {
        qDebug() << "[EmailDataManager] EmailBox.db does not exist:" << dbPath;
        return QVector<Email>();
    }
    
    QString connectionName = QString("email_box_load_%1").arg(sanitizedEmail);
    bool dbSuccess = false;
    QSqlDatabase db = getEmailBoxDatabase(connectionName, dbPath, dbSuccess);
    
    if (!dbSuccess) {
        return QVector<Email>();
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT * FROM emails WHERE folder = :folder ORDER BY sent_at DESC");
    query.bindValue(":folder", static_cast<int>(EmailFolder::Inbox));
    
    QVector<Email> emails;
    if (query.exec()) {
        while (query.next()) {
            emails.append(parseEmailFromQuery(query));
        }
    } else {
        qDebug() << "[EmailDataManager] Query failed:" << query.lastError().text();
    }
    
    query.finish();
    query.clear();
    closeEmailBoxDatabase(connectionName);
    
    qDebug() << "[EmailDataManager] Loaded" << emails.size() << "emails from:" << emailAddress;
    return emails;
}

bool EmailDataManager::prepareAllEmailBoxForRefresh() {
    qDebug() << "[EmailDataManager] prepareAllEmailBoxForRefresh";
    
    // 步骤1：构建AllEmailBox数据库路径
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString allEmailBoxPath = appDataPath + "/AllEmailBox.db";
    
    // 步骤2：关闭并移除现有的数据库连接
    QString connectionName = "all_email_box_refresh";
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if (db.isOpen()) {
            db.close();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }
    
    // 步骤3：如果数据库文件存在，则删除（清空）
    QFile dbFile(allEmailBoxPath);
    if (dbFile.exists()) {
        if (!dbFile.remove()) {
            qDebug() << "[EmailDataManager] Failed to remove existing AllEmailBox.db";
            return false;
        }
        qDebug() << "[EmailDataManager] Removed existing AllEmailBox.db";
    }
    
    // 步骤4：创建新的空数据库结构
    return createAllEmailBoxDatabase();
}

bool EmailDataManager::createAllEmailBoxDatabase() {
    qDebug() << "[EmailDataManager] createAllEmailBoxDatabase";
    
    // 步骤1：构建数据库路径
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir appDir(appDataPath);
    if (!appDir.exists()) {
        if (!appDir.mkpath(".")) {
            qDebug() << "[EmailDataManager] Failed to create directory:" << appDataPath;
            return false;
        }
    }
    
    QString allEmailBoxPath = appDataPath + "/AllEmailBox.db";
    
    // 步骤2：创建数据库连接
    QString connectionName = "all_email_box_refresh";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(allEmailBoxPath);
    
    if (!db.open()) {
        qDebug() << "[EmailDataManager] Failed to open database:" << allEmailBoxPath;
        QSqlDatabase::removeDatabase(connectionName);
        return false;
    }
    
    // 步骤3：创建数据表（与EmailBox相同的结构）
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS emails (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            account_id INTEGER,
            message_id TEXT,
            subject TEXT,
            from_address TEXT,
            from_name TEXT,
            to_address TEXT,
            cc_addresses TEXT,
            sent_at DATETIME,
            received_at DATETIME,
            body TEXT,
            html_body TEXT,
            is_read INTEGER DEFAULT 0,
            is_starred INTEGER DEFAULT 0,
            folder INTEGER DEFAULT 0,
            raw_json TEXT,
            created_at DATETIME,
            modified_at DATETIME
        )
    )";
    
    QSqlQuery query(db);
    if (!query.exec(createTableSql)) {
        qDebug() << "[EmailDataManager] Failed to create table:" << query.lastError().text();
        db.close();
        QSqlDatabase::removeDatabase(connectionName);
        return false;
    }
    
    // 步骤4：创建索引以加速查询
    query.exec("CREATE INDEX IF NOT EXISTS idx_emails_folder ON emails(folder)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_emails_received_at ON emails(received_at)");
    
    db.close();
    QSqlDatabase::removeDatabase(connectionName);
    
    qDebug() << "[EmailDataManager] AllEmailBox.db created successfully";
    return true;
}

bool EmailDataManager::mergeAllEmailBoxesToAllEmailBox(const QVector<EmailAccount>& accounts) {
    qDebug() << "[EmailDataManager] mergeAllEmailBoxesToAllEmailBox, accounts:" << accounts.size();
    
    // 步骤1：准备AllEmailBox（清空并创建）
    if (!prepareAllEmailBoxForRefresh()) {
        qDebug() << "[EmailDataManager] Failed to prepare AllEmailBox";
        return false;
    }
    
    // 步骤2：打开AllEmailBox数据库
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString allEmailBoxPath = appDataPath + "/AllEmailBox.db";
    
    QString connectionName = "all_email_box_merge";
    QSqlDatabase allEmailDb = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    allEmailDb.setDatabaseName(allEmailBoxPath);
    
    if (!allEmailDb.open()) {
        qDebug() << "[EmailDataManager] Failed to open AllEmailBox";
        return false;
    }
    
    // 步骤3：开启事务以提高性能
    allEmailDb.transaction();
    
    // 步骤4：遍历所有账户，复制邮件到AllEmailBox
    int totalMerged = 0;
    for (const EmailAccount& account : accounts) {
        qDebug() << "[EmailDataManager] Merging emails from:" << account.emailAddress;
        
        // 从该账户的EmailBox加载邮件
        QVector<Email> accountEmails = loadEmailsFromEmailBox(account.emailAddress);
        
        // 复制每封邮件到AllEmailBox
        for (const Email& email : accountEmails) {
            // 创建新的Email对象（id设为0，由数据库自动生成）
            Email mergedEmail = email;
            mergedEmail.id = 0;
            mergedEmail.accountId = account.id;
            
            // 插入到AllEmailBox
            QSqlQuery insertQuery(allEmailDb);
            insertQuery.prepare(R"(
                INSERT INTO emails (
                    account_id, message_id, subject, from_address, from_name,
                    to_address, cc_addresses, sent_at, received_at, body,
                    html_body, is_read, is_starred, folder, raw_json,
                    created_at, modified_at
                ) VALUES (
                    :account_id, :message_id, :subject, :from_address, :from_name,
                    :to_address, :cc_addresses, :sent_at, :received_at, :body,
                    :html_body, :is_read, :is_starred, :folder, :raw_json,
                    :created_at, :modified_at
                )
            )");
            
            insertQuery.bindValue(":account_id", mergedEmail.accountId);
            insertQuery.bindValue(":message_id", mergedEmail.messageId);
            insertQuery.bindValue(":subject", mergedEmail.subject);
            insertQuery.bindValue(":from_address", mergedEmail.fromAddress);
            insertQuery.bindValue(":from_name", mergedEmail.fromName);
            insertQuery.bindValue(":to_address", mergedEmail.toAddress);
            insertQuery.bindValue(":cc_addresses", mergedEmail.ccAddresses.join(","));
            insertQuery.bindValue(":sent_at", mergedEmail.sentAt);
            insertQuery.bindValue(":received_at", mergedEmail.receivedAt);
            insertQuery.bindValue(":body", mergedEmail.body);
            insertQuery.bindValue(":html_body", mergedEmail.htmlBody);
            insertQuery.bindValue(":is_read", mergedEmail.isRead ? 1 : 0);
            insertQuery.bindValue(":is_starred", mergedEmail.isStarred ? 1 : 0);
            insertQuery.bindValue(":folder", static_cast<int>(mergedEmail.folder));
            insertQuery.bindValue(":raw_json", mergedEmail.rawJson);
            insertQuery.bindValue(":created_at", mergedEmail.createdAt);
            insertQuery.bindValue(":modified_at", mergedEmail.modifiedAt);
            
            if (!insertQuery.exec()) {
                qDebug() << "[EmailDataManager] Failed to insert email:" << insertQuery.lastError().text();
            } else {
                totalMerged++;
            }
        }
    }
    
    // 步骤5：提交事务
    allEmailDb.commit();
    allEmailDb.close();
    QSqlDatabase::removeDatabase(connectionName);
    
    qDebug() << "[EmailDataManager] Merged" << totalMerged << "emails to AllEmailBox";
    return true;
}

QVector<Email> EmailDataManager::loadEmailsFromAllEmailBox() {
    qDebug() << "[EmailDataManager] loadEmailsFromAllEmailBox";
    
    // 步骤1：构建数据库路径
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString allEmailBoxPath = appDataPath + "/AllEmailBox.db";
    
    // 步骤2：检查数据库文件是否存在
    if (!QFile::exists(allEmailBoxPath)) {
        qDebug() << "[EmailDataManager] AllEmailBox.db does not exist";
        return QVector<Email>();
    }
    
    // 步骤3：打开数据库连接
    QString connectionName = "all_email_box_load";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(allEmailBoxPath);
    
    if (!db.open()) {
        qDebug() << "[EmailDataManager] Failed to open AllEmailBox.db";
        QSqlDatabase::removeDatabase(connectionName);
        return QVector<Email>();
    }
    
    // 步骤4：执行查询（按发送时间降序，即新的在前）
    QSqlQuery query(db);
    query.prepare("SELECT * FROM emails WHERE folder = :folder ORDER BY sent_at DESC");
    query.bindValue(":folder", static_cast<int>(EmailFolder::Inbox));
    
    QVector<Email> emails;
    if (query.exec()) {
        while (query.next()) {
            emails.append(parseEmailFromQuery(query));
        }
    } else {
        qDebug() << "[EmailDataManager] Query failed:" << query.lastError().text();
    }
    
    // 步骤5：关闭数据库连接
    db.close();
    QSqlDatabase::removeDatabase(connectionName);
    
    qDebug() << "[EmailDataManager] Loaded" << emails.size() << "emails from AllEmailBox";
    return emails;
}
