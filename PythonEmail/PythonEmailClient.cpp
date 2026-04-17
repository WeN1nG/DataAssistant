#include "PythonEmailClient.h"
#include "PythonEmbedder.h"
#include <QDebug>
#include <QCoreApplication>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

PythonEmailClient::PythonEmailClient(QObject* parent)
    : QObject(parent)
    , m_state(ConnectionState::Disconnected)
    , m_error(EmailError::NoError)
    , m_port(993)
    , m_useSsl(true)
    , m_imapHandlerId(-1)
    , m_smtpHandlerId(-1)
{
}

PythonEmailClient::~PythonEmailClient() {
    disconnect();
}

bool PythonEmailClient::connectToServer(const QString& host, quint16 port, bool useSsl) {
    qDebug() << "PythonEmailClient: Connecting to" << host << ":" << port;
    
    if (m_state == ConnectionState::Authenticated) {
        qDebug() << "[PythonEmailClient] Already authenticated, emitting connected signal";
        emit connected();
        return true;
    }
    
    m_host = host;
    m_port = port;
    m_useSsl = useSsl;
    
    if (!PythonEmbedder::instance().initialize()) {
        m_error = EmailError::PythonError;
        m_errorString = "Failed to initialize Python interpreter";
        emit errorOccurred(m_error, m_errorString);
        return false;
    }
    
    m_state = ConnectionState::Connecting;
    
    qDebug() << "[PythonEmailClient] connectToServer parameters:";
    qDebug() << "  Host:" << host << "Port:" << port;
    qDebug() << "  Username:" << m_username;
    qDebug() << "  Password length:" << m_password.length() << "chars";
    
    if (m_imapHandlerId < 0) {
        QVariant result = PythonEmbedder::instance().callPythonFunction("imap_handler", "create_handler", QVariantList());
        if (result.isNull() || result.toInt() <= 0) {
            m_error = EmailError::PythonError;
            m_errorString = "Failed to create IMAP handler: " + PythonEmbedder::instance().getLastError();
            emit errorOccurred(m_error, m_errorString);
            return false;
        }
        m_imapHandlerId = result.toInt();
        qDebug() << "Created IMAP handler with ID:" << m_imapHandlerId;
    }
    
    qDebug() << "[PythonEmailClient] Calling handler_connect with args:";
    qDebug() << "  Handler ID:" << m_imapHandlerId;
    qDebug() << "  Server:" << host;
    qDebug() << "  Port:" << port;
    qDebug() << "  Username:" << m_username;
    qDebug() << "  Password:" << (m_password.isEmpty() ? "(empty)" : QString("(length: %1)").arg(m_password.length()));
    
    QVariantList args;
    args.append(m_imapHandlerId);
    args.append(host);
    args.append(static_cast<int>(port));
    args.append(m_username);
    args.append(m_password);
    
    QVariant result = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_connect", args);
    
    qDebug() << "[PythonEmailClient] handler_connect result type:" << result.typeId() << "isNull:" << result.isNull();
    
    if (result.canConvert<QVariantList>()) {
        QVariantList resultList = result.toList();
        if (resultList.size() >= 2) {
            bool success = resultList[0].toBool();
            if (!success && resultList.size() >= 2) {
                m_errorString = resultList[1].toString();
            }
            if (success) {
                m_state = ConnectionState::Authenticated;
                emit connected();
                qDebug() << "PythonEmailClient: Connected successfully";
                return true;
            }
        }
    } else if (result.canConvert<bool>() && result.toBool()) {
        m_state = ConnectionState::Authenticated;
        emit connected();
        qDebug() << "PythonEmailClient: Connected successfully";
        return true;
    }
    
    m_error = EmailError::ConnectionFailed;
    emit errorOccurred(m_error, m_errorString);
    qCritical() << "PythonEmailClient: Connection failed:" << m_errorString;
    return false;
}

bool PythonEmailClient::connectToServer(const QString& host, quint16 port, const QString& username, const QString& password, bool useSsl) {
    qDebug() << "[PythonEmailClient] connectToServer (with credentials) called:";
    qDebug() << "  Host:" << host << ":" << port;
    qDebug() << "  Username:" << username;
    qDebug() << "  Password length:" << (password.isEmpty() ? 0 : password.length());
    qDebug() << "  useSsl:" << useSsl;
    
    m_username = username;
    m_password = password;
    return connectToServer(host, port, useSsl);
}

void PythonEmailClient::disconnect() {
    if (m_state != ConnectionState::Disconnected) {
        if (m_imapHandlerId >= 0) {
            QVariantList args;
            args.append(m_imapHandlerId);
            PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_disconnect", args);
            m_imapHandlerId = -1;
        }
        
        if (m_smtpHandlerId >= 0) {
            QVariantList args;
            args.append(m_smtpHandlerId);
            PythonEmbedder::instance().callPythonFunction("smtp_handler", "handler_disconnect", args);
            m_smtpHandlerId = -1;
        }
        
        m_state = ConnectionState::Disconnected;
        emit disconnected();
        qDebug() << "PythonEmailClient: Disconnected";
    }
}

bool PythonEmailClient::login(const QString& username, const QString& password) {
    qDebug() << "[PythonEmailClient] login called:";
    qDebug() << "  Username:" << username;
    qDebug() << "  Password:" << (password.isEmpty() ? "(empty)" : QString("(length: %1)").arg(password.length()));
    qDebug() << "  Current state:" << static_cast<int>(m_state);
    qDebug() << "  Current m_host:" << m_host << "m_port:" << m_port << "m_useSsl:" << m_useSsl;
    
    m_username = username;
    m_password = password;
    
    if (m_state == ConnectionState::Authenticated) {
        qDebug() << "[PythonEmailClient] Already authenticated";
        return true;
    }
    
    qDebug() << "[PythonEmailClient] login: State is not Authenticated, calling connectToServer...";
    bool result = connectToServer(m_host, m_port, m_useSsl);
    qDebug() << "[PythonEmailClient] login: connectToServer returned:" << result << ", current state:" << static_cast<int>(m_state);
    
    return result;
}

void PythonEmailClient::logout() {
    qDebug() << "PythonEmailClient: Logging out";
    disconnect();
}

bool PythonEmailClient::selectMailbox(const QString& mailboxName) {
    qDebug() << "[PythonEmailClient] selectMailbox called:";
    qDebug() << "  mailboxName:" << mailboxName;
    qDebug() << "  m_state:" << static_cast<int>(m_state);
    qDebug() << "  m_imapHandlerId:" << m_imapHandlerId;
    
    if (m_state == ConnectionState::Disconnected) {
        qDebug() << "[ERROR] PythonEmailClient::selectMailbox: State is Disconnected, cannot select mailbox";
        m_error = EmailError::NotAuthenticated;
        m_errorString = "Not connected to server";
        return false;
    }
    
    if (m_imapHandlerId < 0) {
        qDebug() << "[ERROR] PythonEmailClient::selectMailbox: m_imapHandlerId is invalid:" << m_imapHandlerId;
        m_error = EmailError::NotAuthenticated;
        m_errorString = "No IMAP handler";
        return false;
    }
    
    qDebug() << "PythonEmailClient: Selecting mailbox:" << mailboxName;
    
    QVariantList args;
    args.append(m_imapHandlerId);
    args.append(mailboxName);
    
    qDebug() << "[PythonEmailClient] selectMailbox: Calling Python handler_select_mailbox...";
    QVariant result = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_select_mailbox", args);
    qDebug() << "[PythonEmailClient] selectMailbox: Python call returned, result.type:" << result.typeId() << "isNull:" << result.isNull();
    
    if (result.canConvert<QVariantList>()) {
        QVariantList resultList = result.toList();
        qDebug() << "[PythonEmailClient] selectMailbox: Result is QVariantList, size:" << resultList.size();
        if (resultList.size() >= 2) {
            bool success = resultList[0].toBool();
            qDebug() << "[PythonEmailClient] selectMailbox: success:" << success;
            if (!success && resultList.size() >= 2) {
                qDebug() << "[PythonEmailClient] selectMailbox: error message:" << resultList[1].toString();
            }
            if (success) {
                m_currentMailbox = mailboxName;
                m_state = ConnectionState::Selected;
                qDebug() << "[PythonEmailClient] selectMailbox: Emitting mailboxSelected signal";
                emit mailboxSelected(mailboxName);
                return true;
            }
        }
    } else if (result.canConvert<bool>()) {
        bool success = result.toBool();
        qDebug() << "[PythonEmailClient] selectMailbox: Result is bool, value:" << success;
        if (success) {
            m_currentMailbox = mailboxName;
            m_state = ConnectionState::Selected;
            qDebug() << "[PythonEmailClient] selectMailbox: Emitting mailboxSelected signal";
            emit mailboxSelected(mailboxName);
            return true;
        }
    } else {
        qDebug() << "[ERROR] PythonEmailClient::selectMailbox: Unexpected result type:" << result.typeName();
    }
    
    qDebug() << "[ERROR] PythonEmailClient::selectMailbox: Failed, getting last error...";
    m_error = EmailError::MailboxNotSelected;
    m_errorString = getLastError();
    qDebug() << "[ERROR] PythonEmailClient::selectMailbox: lastError:" << m_errorString;
    emit errorOccurred(m_error, m_errorString);
    return false;
}

QStringList PythonEmailClient::listMailboxes() {
    qDebug() << "PythonEmailClient: Listing mailboxes";
    
    if (m_imapHandlerId < 0) {
        return QStringList();
    }
    
    QVariantList args;
    args.append(m_imapHandlerId);
    
    QVariant result = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_list_mailboxes", args);
    
    if (result.canConvert<QVariantList>()) {
        QStringList mailboxes;
        for (const QVariant& item : result.toList()) {
            mailboxes.append(item.toString());
        }
        return mailboxes;
    }
    
    return QStringList();
}

QVector<PythonEmailClient::EmailInfo> PythonEmailClient::fetchEmailList(int startSeq, int count) {
    QVector<EmailInfo> result;
    
    qDebug() << "PythonEmailClient: Fetching email list";
    
    if (m_imapHandlerId < 0) {
        qDebug() << "[ERROR] fetchEmailList: m_imapHandlerId is invalid:" << m_imapHandlerId;
        return result;
    }
    
    QVariantList args;
    args.append(m_imapHandlerId);
    
    qDebug() << "[DEBUG] Calling Python handler_get_recent with handler_id:" << m_imapHandlerId;
    QVariant pythonResult = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_get_recent", args);
    
    qDebug() << "[DEBUG] Python result type:" << pythonResult.typeId() << "isNull:" << pythonResult.isNull();
    
    if (pythonResult.isNull()) {
        qDebug() << "[ERROR] Python result is null!";
        return result;
    }
    
    if (pythonResult.canConvert<QVariantList>()) {
        QVariantList resultList = pythonResult.toList();
        qDebug() << "[DEBUG] Python returned" << resultList.size() << "items";
        
        if (resultList.isEmpty()) {
            qDebug() << "[WARNING] Result list is empty!";
            return result;
        }
        
        for (int i = 0; i < resultList.size() && i < 3; ++i) {
            const QVariant& item = resultList[i];
            qDebug() << "[DEBUG] Item" << i << "typeId:" << item.typeId() << "typeName:" << item.typeName();
            
            if (item.canConvert<QVariantMap>()) {
                qDebug() << "[DEBUG] Item" << i << "is a QVariantMap";
                QVariantMap map = item.toMap();
                result.append(parseEmailInfo(map));
            } else if (item.canConvert<QString>()) {
                QString str = item.toString();
                qDebug() << "[WARNING] Item" << i << "is a QString, length:" << str.length();
                qDebug() << "[DEBUG] Item" << i << "content (first 50 chars):" << str.left(50);
            } else {
                qDebug() << "[WARNING] Item" << i << "has unknown type:" << item.typeName();
            }
        }
        
        qDebug() << "[DEBUG] Total emails parsed:" << result.size();
    } else {
        qDebug() << "[ERROR] Python result cannot convert to QVariantList, type:" << pythonResult.typeName();
        qDebug() << "[ERROR] Python result:" << pythonResult.toString();
    }
    
    return result;
}

QString PythonEmailClient::fetchEmailListAsJson(int startSeq, int count) {
    qDebug() << "PythonEmailClient: Fetching email list as JSON";
    
    if (m_imapHandlerId < 0) {
        qDebug() << "[ERROR] fetchEmailListAsJson: m_imapHandlerId is invalid:" << m_imapHandlerId;
        return "{}";
    }
    
    QVariantList args;
    args.append(m_imapHandlerId);
    args.append(count);
    
    qDebug() << "[DEBUG] Calling Python handler_get_recent_emails_json with handler_id:" << m_imapHandlerId;
    QVariant pythonResult = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_get_recent_emails_json", args);
    
    qDebug() << "[DEBUG] Python JSON result type:" << pythonResult.typeId() << "isNull:" << pythonResult.isNull();
    
    if (pythonResult.isNull()) {
        qDebug() << "[ERROR] Python JSON result is null!";
        return "{}";
    }
    
    QString jsonStr = pythonResult.toString();
    qDebug() << "[DEBUG] Python JSON string length:" << jsonStr.length();
    
    return jsonStr;
}

QString PythonEmailClient::fetchEmailListAsJsonFast(int startSeq, int count) {
    qDebug() << "PythonEmailClient: Fetching email list as JSON (FAST - headers only)";
    
    if (m_imapHandlerId < 0) {
        qDebug() << "[ERROR] fetchEmailListAsJsonFast: m_imapHandlerId is invalid:" << m_imapHandlerId;
        return "[]";
    }
    
    QVariantList args;
    args.append(m_imapHandlerId);
    args.append(count);
    
    qDebug() << "[DEBUG] Calling Python handler_get_recent_fast with handler_id:" << m_imapHandlerId;
    QVariant pythonResult = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_get_recent_fast", args);
    
    qDebug() << "[DEBUG] Python fast result type:" << pythonResult.typeId() << "isNull:" << pythonResult.isNull();
    qDebug() << "[DEBUG] Python fast result isValid:" << pythonResult.isValid();
    
    if (pythonResult.isNull()) {
        qDebug() << "[ERROR] Python fast result is null!";
        return "[]";
    }
    
    QVariantList resultList = pythonResult.toList();
    qDebug() << "[DEBUG] Python fast result list size:" << resultList.size();
    
    if (resultList.isEmpty()) {
        qDebug() << "[WARNING] Python fast result list is empty!";
        qDebug() << "[DEBUG] Python result type name:" << pythonResult.typeName();
        qDebug() << "[DEBUG] Python result:" << pythonResult;
        return "[]";
    }
    
    QString jsonStr = QString("[");
    for (int i = 0; i < resultList.size(); ++i) {
        if (i > 0) jsonStr += ",";
        QJsonDocument doc = QJsonDocument::fromVariant(resultList[i]);
        jsonStr += doc.toJson(QJsonDocument::Compact);
        
        if (i == 0) {
            qDebug() << "[DEBUG] First email JSON:" << jsonStr;
        }
    }
    jsonStr += QString("]");
    
    qDebug() << "[DEBUG] Python fast JSON string length:" << jsonStr.length();
    qDebug() << "[DEBUG] Python fast JSON first 500 chars:" << jsonStr.left(500);
    
    return jsonStr;
}

PythonEmailClient::EmailContent PythonEmailClient::fetchEmailBody(int messageNumber, bool markAsRead) {
    EmailContent content;
    
    qDebug() << "[PythonEmailClient] fetchEmailBody called:";
    qDebug() << "  messageNumber:" << messageNumber;
    qDebug() << "  markAsRead:" << markAsRead;
    qDebug() << "  m_imapHandlerId:" << m_imapHandlerId;
    qDebug() << "  m_state:" << static_cast<int>(m_state);
    
    if (m_imapHandlerId < 0) {
        qDebug() << "[ERROR] PythonEmailClient::fetchEmailBody: m_imapHandlerId is invalid:" << m_imapHandlerId;
        return content;
    }
    
    QVariantList args;
    args.append(m_imapHandlerId);
    args.append(QString::number(messageNumber));
    
    qDebug() << "[PythonEmailClient] fetchEmailBody: Calling Python handler_fetch_body...";
    QVariant pythonResult = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_fetch_body", args);
    qDebug() << "[PythonEmailClient] fetchEmailBody: Python call returned, result.type:" << pythonResult.typeId() << "isNull:" << pythonResult.isNull();
    
    if (pythonResult.isNull()) {
        qDebug() << "[ERROR] PythonEmailClient::fetchEmailBody: Python result is null!";
        return content;
    }
    
    if (pythonResult.canConvert<QVariantMap>()) {
        QVariantMap map = pythonResult.toMap();
        qDebug() << "[PythonEmailClient] fetchEmailBody: Parsing QVariantMap, keys:" << map.keys();
        
        content.subject = map.value("subject").toString();
        content.from = map.value("from").toString();
        content.to = map.value("to").toString();
        content.cc = map.value("cc").toString();
        content.date = map.value("date").toString();
        content.body = map.value("body").toString();
        content.htmlBody = map.value("html_body").toString();
        
        qDebug() << "[PythonEmailClient] fetchEmailBody: Parsed email content:";
        qDebug() << "  subject:" << content.subject;
        qDebug() << "  from:" << content.from;
        qDebug() << "  body.length:" << content.body.length();
        qDebug() << "  htmlBody.length:" << content.htmlBody.length();
        
        if (map.contains("attachments")) {
            QVariantList attachments = map.value("attachments").toList();
            for (const QVariant& att : attachments) {
                QVariantMap attMap = att.toMap();
                EmailAttachment emailAtt;
                emailAtt.filename = attMap.value("filename").toString();
                emailAtt.contentType = attMap.value("content_type").toString();
                emailAtt.size = attMap.value("size").toInt();
                QString contentBase64 = attMap.value("content").toString();
                emailAtt.content = QByteArray::fromBase64(contentBase64.toUtf8());
                content.attachments.append(emailAtt);
            }
        }
    }
    
    return content;
}

EmailAttachment PythonEmailClient::fetchAttachment(int messageNumber, const QString& filename) {
    qDebug() << "PythonEmailClient: Fetching attachment" << filename << "from email" << messageNumber;
    
    EmailAttachment attachment;
    attachment.filename = filename;
    
    if (m_imapHandlerId < 0) {
        qWarning() << "No IMAP handler available";
        return attachment;
    }
    
    QVariantList args;
    args.append(m_imapHandlerId);
    args.append(QString::number(messageNumber));
    args.append(filename);
    
    QVariant result = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_fetch_attachment", args);
    
    if (result.isNull()) {
        qWarning() << "Failed to fetch attachment";
        return attachment;
    }
    
    QVariantMap attMap = result.toMap();
    if (!attMap.isEmpty()) {
        attachment.filename = attMap.value("filename", filename).toString();
        attachment.contentType = attMap.value("content_type").toString();
        QString contentBase64 = attMap.value("content").toString();
        attachment.content = QByteArray::fromBase64(contentBase64.toUtf8());
        attachment.size = attMap.value("size").toInt();
        
        qDebug() << "Attachment fetched successfully:" << attachment.filename 
                 << "Size:" << attachment.size << "bytes";
    }
    
    return attachment;
}

bool PythonEmailClient::deleteEmail(int messageNumber) {
    qDebug() << "PythonEmailClient: Deleting email" << messageNumber;
    
    if (m_imapHandlerId < 0) {
        return false;
    }
    
    QVariantList args;
    args.append(m_imapHandlerId);
    args.append(QString::number(messageNumber));
    
    QVariant result = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_delete", args);
    
    if (result.canConvert<bool>() && result.toBool()) {
        emit emailDeleted(messageNumber);
        return true;
    }
    
    return false;
}

bool PythonEmailClient::moveEmail(int messageNumber, const QString& destinationMailbox) {
    qDebug() << "PythonEmailClient: Moving email" << messageNumber << "to" << destinationMailbox;
    
    if (m_imapHandlerId < 0) {
        return false;
    }
    
    QVariantList args;
    args.append(m_imapHandlerId);
    args.append(QString::number(messageNumber));
    args.append(destinationMailbox);
    
    QVariant result = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_move", args);
    
    if (result.canConvert<bool>()) {
        return result.toBool();
    }
    return false;
}

bool PythonEmailClient::sendEmail(const QString& from, const QString& fromName,
                                  const QStringList& to, const QString& subject,
                                  const QString& body, const QString& htmlBody,
                                  const QStringList& cc, const QStringList& bcc) {
    qDebug() << "PythonEmailClient: Sending email to" << to;
    
    if (m_smtpHandlerId < 0) {
        QVariant result = PythonEmbedder::instance().callPythonFunction("smtp_handler", "create_handler", QVariantList());
        if (result.isNull() || result.toInt() <= 0) {
            qCritical() << "Failed to create SMTP handler";
            return false;
        }
        m_smtpHandlerId = result.toInt();
    }
    
    QVariantList args;
    args.append(m_smtpHandlerId);
    args.append(from);
    args.append(fromName);
    args.append(QVariant(to));
    args.append(subject);
    args.append(body);
    args.append(htmlBody);
    args.append(QVariant(cc));
    args.append(QVariant(bcc));
    
    QVariant result = PythonEmbedder::instance().callPythonFunction("smtp_handler", "handler_send_email", args);
    
    if (result.canConvert<bool>() && result.toBool()) {
        qDebug() << "Email sent successfully";
        return true;
    }
    
    m_errorString = getLastError();
    m_error = EmailError::CommandFailed;
    emit errorOccurred(m_error, m_errorString);
    return false;
}

QString PythonEmailClient::getLastError() {
    if (m_imapHandlerId < 0) {
        return "No handler";
    }
    
    QVariantList args;
    args.append(m_imapHandlerId);
    
    QVariant result = PythonEmbedder::instance().callPythonFunction("imap_handler", "handler_get_error", args);
    
    if (result.canConvert<QString>()) {
        return result.toString();
    }
    
    return "Unknown error";
}

bool PythonEmailClient::isConnected() const {
    return m_state != ConnectionState::Disconnected && m_state != ConnectionState::LoggedOut;
}

PythonEmailClient::EmailInfo PythonEmailClient::parseEmailInfo(const QVariantMap& map) {
    EmailInfo info;
    info.uid = map.value("id").toInt();
    info.messageNumber = map.value("id").toInt();
    info.subject = map.value("subject").toString();
    info.from = map.value("from").toString();
    info.to = map.value("to").toString();
    
    QString dateStr = map.value("date").toString();
    info.date = QDateTime::fromString(dateStr, Qt::ISODate);
    if (!info.date.isValid()) {
        info.date = QDateTime::currentDateTime();
    }
    
    info.size = 0;
    info.isRead = false;
    info.isAnswered = false;
    info.isFlagged = false;
    info.isDeleted = false;
    info.isDraft = false;
    
    info.cc = map.value("cc").toString();
    info.body = map.value("body").toString();
    info.htmlBody = map.value("html_body").toString();
    
    if (map.contains("attachments")) {
        QVariantList attachments = map.value("attachments").toList();
        for (const QVariant& att : attachments) {
            if (att.canConvert<QVariantMap>()) {
                QVariantMap attMap = att.toMap();
                info.attachments.append(attMap.value("filename").toString());
            }
        }
    }
    
    return info;
}