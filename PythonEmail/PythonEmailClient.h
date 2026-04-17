#ifndef PYTHONEMAILCLIENT_H
#define PYTHONEMAILCLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include <QVariant>
#include "EmailModels.h"

class PythonEmailClient : public QObject
{
    Q_OBJECT

public:
    enum class ConnectionState {
        Disconnected,
        Connecting,
        Authenticating,
        Authenticated,
        Selected,
        LoggedOut
    };

    enum class EmailError {
        NoError,
        ConnectionFailed,
        AuthenticationFailed,
        Timeout,
        InvalidResponse,
        CommandFailed,
        NotAuthenticated,
        MailboxNotSelected,
        NetworkError,
        PythonError
    };

    struct EmailInfo {
        int uid;
        int messageNumber;
        QString subject;
        QString from;
        QString to;
        QDateTime date;
        int size;
        bool isRead;
        bool isAnswered;
        bool isFlagged;
        bool isDeleted;
        bool isDraft;
        
        QString cc;
        QString body;
        QString htmlBody;
        QVector<QString> attachments;
    };

    struct EmailContent {
        QString subject;
        QString from;
        QString to;
        QString cc;
        QString date;
        QString body;
        QString htmlBody;
        QVector<EmailAttachment> attachments;
    };

    explicit PythonEmailClient(QObject* parent = nullptr);
    ~PythonEmailClient();

    bool connectToServer(const QString& host, quint16 port, bool useSsl = true);
    bool connectToServer(const QString& host, quint16 port, const QString& username, const QString& password, bool useSsl = true);
    void disconnect();
    bool login(const QString& username, const QString& password);
    void logout();

    bool selectMailbox(const QString& mailboxName);
    QStringList listMailboxes();
    
    QVector<EmailInfo> fetchEmailList(int startSeq = 1, int count = 100);
    QString fetchEmailListAsJson(int startSeq = 1, int count = 100);
    QString fetchEmailListAsJsonFast(int startSeq = 1, int count = 50);
    EmailContent fetchEmailBody(int messageNumber, bool markAsRead = false);
    bool deleteEmail(int messageNumber);
    bool moveEmail(int messageNumber, const QString& destinationMailbox);
    EmailAttachment fetchAttachment(int messageNumber, const QString& filename);
    
    bool sendEmail(const QString& from, const QString& fromName,
                   const QStringList& to, const QString& subject,
                   const QString& body, const QString& htmlBody = QString(),
                   const QStringList& cc = QStringList(),
                   const QStringList& bcc = QStringList());

    EmailError error() const { return m_error; }
    QString errorString() const { return m_errorString; }
    ConnectionState state() const { return m_state; }
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void authenticated();
    void errorOccurred(EmailError error, const QString& message);
    void mailboxSelected(const QString& mailboxName);
    void emailReceived(int messageNumber);
    void emailDeleted(int messageNumber);
    void progressUpdated(const QString& operation, int current, int total);

private:
    QString getLastError();
    EmailInfo parseEmailInfo(const QVariantMap& map);

private:
    ConnectionState m_state;
    EmailError m_error;
    QString m_errorString;
    
    QString m_host;
    quint16 m_port;
    QString m_username;
    QString m_password;
    bool m_useSsl;
    QString m_currentMailbox;
    int m_imapHandlerId;
    int m_smtpHandlerId;
};

#endif