#include "EmailProtocolTest.h"
#include <QDebug>
#include <QRegularExpression>

EmailProtocolTest::EmailProtocolTest(QObject* parent)
    : QObject(parent)
    , m_emailClient(new PythonEmailClient(this))
    , m_testImapPort(993)
    , m_testSmtpPort(587)
    , m_imapTestPassed(false)
    , m_smtpTestPassed(false)
{
    connect(m_emailClient, &PythonEmailClient::connected, this, &EmailProtocolTest::onImapConnected);
    connect(m_emailClient, &PythonEmailClient::errorOccurred, this, &EmailProtocolTest::onImapError);
}

EmailProtocolTest::~EmailProtocolTest() {
}

void EmailProtocolTest::testImapConnection(const QString& host, quint16 port,
                                          const QString& username, const QString& password) {
    emit testStarted("IMAP Connection Test");

    m_testUsername = username;
    m_testPassword = password;
    m_testImapHost = host;
    m_testImapPort = port;

    if (m_emailClient->connectToServer(host, port, true)) {
        if (m_emailClient->login(username, password)) {
            m_imapTestPassed = true;
            emit testCompleted("IMAP Connection Test", true);
        } else {
            emit testFailed("IMAP Connection Test", "Authentication failed");
        }
    } else {
        emit testFailed("IMAP Connection Test", "Connection failed");
    }
}

void EmailProtocolTest::testSmtpConnection(const QString& host, quint16 port,
                                          const QString& username, const QString& password) {
    emit testStarted("SMTP Connection Test");

    m_testUsername = username;
    m_testPassword = password;
    m_testSmtpHost = host;
    m_testSmtpPort = port;

    QString fromName = "Test User";
    QStringList toList;
    toList.append("recipient@example.com");
    QString subject = "Test Email";
    QString body = "This is a test email.";

    if (m_emailClient->sendEmail(username, fromName, toList, subject, body)) {
        m_smtpTestPassed = true;
        emit testCompleted("SMTP Connection Test", true);
    } else {
        emit testFailed("SMTP Connection Test", "Failed to send test email");
    }
}

void EmailProtocolTest::testImapOperations() {
    if (!m_imapTestPassed) {
        qDebug() << "IMAP not connected, skipping operations test";
        return;
    }

    emit testStarted("IMAP Operations Test");

    QStringList mailboxes = m_emailClient->listMailboxes();
    qDebug() << "Found" << mailboxes.size() << "mailboxes";

    if (m_emailClient->selectMailbox("INBOX")) {
        QVector<PythonEmailClient::EmailInfo> emails = m_emailClient->fetchEmailList(1, 10);
        qDebug() << "Fetched" << emails.size() << "emails from INBOX";
    }

    emit testCompleted("IMAP Operations Test", true);
}

void EmailProtocolTest::testSmtpOperations() {
    if (!m_testUsername.isEmpty()) {
        qDebug() << "SMTP not connected, skipping operations test";
        return;
    }

    emit testStarted("SMTP Operations Test");

    QStringList toList;
    toList.append("recipient@example.com");

    if (m_emailClient->sendEmail(m_testUsername, "Test User", toList, "Test Email", "This is a test email.")) {
        qDebug() << "Test email sent successfully";
        emit testCompleted("SMTP Operations Test", true);
    } else {
        emit testFailed("SMTP Operations Test", "Failed to send test email");
    }
}

void EmailProtocolTest::testEmailValidation() {
    qDebug() << "Running Email Validation Test";

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");

    QStringList validEmails = {
        "test@example.com",
        "user.name@domain.org",
        "user+tag@example.co.uk"
    };

    QStringList invalidEmails = {
        "invalid",
        "@nodomain.com",
        "spaces in@email.com"
    };

    for (const QString& email : validEmails) {
        QRegularExpressionMatch match = emailRegex.match(email);
        qDebug() << "Valid email" << email << ":" << match.hasMatch();
    }

    for (const QString& email : invalidEmails) {
        QRegularExpressionMatch match = emailRegex.match(email);
        qDebug() << "Invalid email" << email << ":" << !match.hasMatch();
    }
}

void EmailProtocolTest::testMimeEncoding() {
    qDebug() << "Running MIME Encoding Test";

    QString testString = "测试中文 Subject";
    QString encoded = QString::fromUtf8(testString.toLocal8Bit().toBase64());
    qDebug() << "Original:" << testString;
    qDebug() << "Encoded:" << encoded;

    QString base64Test = "Hello World";
    QString base64Encoded = QString::fromUtf8(base64Test.toUtf8().toBase64());
    qDebug() << "Base64 of" << base64Test << ":" << base64Encoded;
}

void EmailProtocolTest::testBase64Encoding() {
    qDebug() << "Running Base64 Encoding Test";

    QByteArray testData = "Hello, World!";
    QString encoded = QString::fromUtf8(testData.toBase64());
    qDebug() << "Original:" << testData;
    qDebug() << "Base64:" << encoded;

    QByteArray decoded = QByteArray::fromBase64(encoded.toUtf8());
    qDebug() << "Decoded:" << decoded;
}

void EmailProtocolTest::onImapConnected() {
    qDebug() << "IMAP Connected";
}

void EmailProtocolTest::onImapError(PythonEmailClient::EmailError error, const QString& message) {
    qDebug() << "IMAP Error:" << message;
    emit testFailed("IMAP Connection", message);
}

void EmailProtocolTest::onSmtpConnected() {
    qDebug() << "SMTP Connected";
}

void EmailProtocolTest::onSmtpError(const QString& message) {
    qDebug() << "SMTP Error:" << message;
    emit testFailed("SMTP Connection", message);
}