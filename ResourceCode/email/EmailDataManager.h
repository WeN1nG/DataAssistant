#ifndef EMAILDATAMANAGER_H
#define EMAILDATAMANAGER_H

#include "EmailModels.h"
#include "../Database/DatabaseManager.h"
#include <QVector>
#include <QMap>
#include <QSqlQuery>

class EmailDataManager {
public:
    EmailDataManager();
    ~EmailDataManager();

    bool initializeDatabase();

    bool addEmailAccount(const EmailAccount& account);
    bool updateEmailAccount(const EmailAccount& account);
    bool deleteEmailAccount(int accountId);
    EmailAccount getEmailAccount(int accountId);
    QVector<EmailAccount> getAllEmailAccounts();
    EmailAccount getDefaultEmailAccount();

    bool addEmail(const Email& email);
    bool updateEmail(const Email& email);
    bool deleteEmail(int emailId, bool permanent = false);
    bool moveEmail(int emailId, EmailFolder targetFolder);
    Email getEmailById(int emailId);
    QVector<Email> getEmailsByFolder(int accountId, EmailFolder folder);
    QVector<Email> getEmailsByFilter(const EmailFilter& filter);

    bool markEmailAsRead(int emailId);
    bool markEmailAsUnread(int emailId);
    bool toggleEmailStar(int emailId);

    int getUnreadCount(int accountId, EmailFolder folder);
    QMap<EmailFolder, int> getFolderCounts(int accountId);

    bool saveDraft(const Email& email);
    Email getDraft(int accountId);

    bool clearAllEmailsForAccount(int accountId);
    bool clearAllEmails();

    bool IniAllEmailDatabaseFiles();

    bool clearEmailBoxForAccount(const QString& emailAddress);
    bool IniEmailBoxForAccount(const QString& emailAddress);
    bool createEmailBoxForAccount(const QString& emailAddress);
    bool saveEmailToEmailBox(const Email& email, const QString& emailAddress);
    QVector<Email> getEmailsFromEmailBox(const QString& emailAddress);
    bool mergeEmailBoxesToAll(const QVector<EmailAccount>& accounts);

    Email parseEmailFromQuery(const QSqlQuery& query);

    // ==================== EmailBox 刷新管理方法 ====================
    
    // 功能：准备指定账户的EmailBox进行刷新（清空并重建）
    // 参数：emailAddress - 邮箱地址（将用于构建数据库文件名）
    // 返回：true-成功，false-失败
    // 调用时机：单账户刷新时调用
    bool prepareEmailBoxForRefresh(const QString& emailAddress);
    
    // 功能：从指定账户的EmailBox加载邮件列表
    // 参数：emailAddress - 邮箱地址
    // 返回：邮件列表（按received_at降序）
    // 调用时机：加载显示邮件时调用
    QVector<Email> loadEmailsFromEmailBox(const QString& emailAddress);
    
    // 功能：准备AllEmailBox进行刷新（清空并重建）
    // 返回：true-成功，false-失败
    // 调用时机：全部账户刷新时调用
    bool prepareAllEmailBoxForRefresh();
    
    // 功能：合并所有账户的EmailBox到AllEmailBox
    // 参数：accounts - 所有邮箱账户列表
    // 返回：true-成功，false-失败
    // 说明：会自动按received_at降序排序
    // 调用时机：全部账户刷新完成后调用
    bool mergeAllEmailBoxesToAllEmailBox(const QVector<EmailAccount>& accounts);
    
    // 功能：从AllEmailBox加载邮件列表
    // 返回：邮件列表（按received_at降序）
    // 调用时机：加载显示"全部账户"邮件时调用
    QVector<Email> loadEmailsFromAllEmailBox();

private:
    QString dbPath;
    bool initializeTables();
    bool createAllEmailBoxDatabase();  // 创建AllEmailBox数据库结构（内部使用）

    bool executeQuery(const QString& query, const QMap<QString, QVariant>& params);
    QVector<Email> parseEmailQuery(const QString& query, const QMap<QString, QVariant>& params);
    EmailAccount parseAccountFromQuery(const QSqlQuery& query);
    
    QSqlDatabase getEmailBoxDatabase(const QString& connectionName, const QString& dbPath, bool& success);
    void closeEmailBoxDatabase(const QString& connectionName);
};

#endif
