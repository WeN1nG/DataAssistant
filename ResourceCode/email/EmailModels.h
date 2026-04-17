#ifndef EMAILMODELS_H
#define EMAILMODELS_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QMap>

enum class EmailFolder {
    Inbox = 0,
    Sent = 1,
    Draft = 2,
    Trash = 3
};

enum class EmailPriority {
    Low = 0,
    Normal = 1,
    High = 2
};

struct EmailAttachment {
    QString filename;
    QString contentType;
    QByteArray content;
    int size;

    EmailAttachment()
        : size(0)
    {}
};

struct EmailAccount {
    int id;
    QString emailAddress;
    QString displayName;
    QString smtpServer;
    int smtpPort;
    QString imapServer;
    int imapPort;
    QString username;
    QString password;
    bool isDefault;
    bool isActive;
    QDateTime createdAt;
    QDateTime lastUsedAt;

    EmailAccount()
        : id(0)
        , smtpPort(587)
        , imapPort(993)
        , isDefault(false)
        , isActive(true)
    {}
};

struct Email {
    int id;
    int accountId;
    QString messageId;
    QString subject;
    QString fromAddress;
    QString fromName;
    QString toAddress;
    QStringList ccAddresses;
    QStringList bccAddresses;
    QString body;
    QString htmlBody;
    QVector<EmailAttachment> attachments;
    EmailFolder folder;
    bool isRead;
    bool isStarred;
    bool isDeleted;
    EmailPriority priority;
    QDateTime sentAt;
    QDateTime receivedAt;
    QDateTime createdAt;
    QDateTime modifiedAt;
    QString rawJson;

    Email()
        : id(0)
        , accountId(0)
        , folder(EmailFolder::Inbox)
        , isRead(false)
        , isStarred(false)
        , isDeleted(false)
        , priority(EmailPriority::Normal)
    {}
};

struct EmailFilter {
    int accountId;
    EmailFolder folder;
    QString searchText;
    QString fromFilter;
    QString toFilter;
    QDateTime dateFrom;
    QDateTime dateTo;
    bool unreadOnly;
    bool starredOnly;
    EmailPriority priority;

    EmailFilter()
        : accountId(0)
        , folder(EmailFolder::Inbox)
        , unreadOnly(false)
        , starredOnly(false)
        , priority(EmailPriority::Normal)
    {}
};

#endif
