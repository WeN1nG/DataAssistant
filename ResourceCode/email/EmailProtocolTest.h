#ifndef EMAILPROTOCOLTEST_H
#define EMAILPROTOCOLTEST_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include "../../PythonEmail/PythonEmailClient.h"

class EmailProtocolTest : public QObject
{
    Q_OBJECT

public:
    explicit EmailProtocolTest(QObject* parent = nullptr);
    ~EmailProtocolTest();

    void testImapConnection(const QString& host, quint16 port, const QString& username, const QString& password);
    void testSmtpConnection(const QString& host, quint16 port, const QString& username, const QString& password);

    void testImapOperations();
    void testSmtpOperations();

    static void testEmailValidation();
    static void testMimeEncoding();
    static void testBase64Encoding();

signals:
    void testStarted(const QString& testName);
    void testCompleted(const QString& testName, bool success);
    void testFailed(const QString& testName, const QString& error);

private slots:
    void onImapConnected();
    void onImapError(PythonEmailClient::EmailError error, const QString& message);
    void onSmtpConnected();
    void onSmtpError(const QString& message);

private:
    PythonEmailClient* m_emailClient;
    QString m_testUsername;
    QString m_testPassword;
    QString m_testImapHost;
    QString m_testSmtpHost;
    quint16 m_testImapPort;
    quint16 m_testSmtpPort;
    bool m_imapTestPassed;
    bool m_smtpTestPassed;
};

#endif