# IMAP/SMTP 邮件功能集成开发指导文�?

## 文档概述

本文档详细介绍如何在 PersonalDateAssistant 项目中集�?IMAP 邮件接收�?SMTP 邮件发送功能。本项目使用 C++ 语言结合 libcurl 库实现，支持多邮箱账户管理，为日程管理应用提供邮件通知和邮件同步能力�?

***

## 目录

1. [环境配置要求](#1-环境配置要求)
2. [libcurl库安装与配置](#2-libcurl库安装与配置)
3. [IMAP邮件接收功能实现](#3-imap邮件接收功能实现)
4. \[SMTP邮件发送功能实现]\(#4-smtp邮件发送功能实�?
5. [错误处理机制设计](#5-错误处理机制设计)
6. [代码结构建议](#6-代码结构建议)
7. [关键API调用示例](#7-关键api调用示例)
8. [安全注意事项](#8-安全注意事项)
9. \[测试方法与验证步骤]\(#9-测试方法与验证步�?
10. \[常见问题及解决方案]\(#10-常见问题及解决方�?

***

## 1. 环境配置要求

### 1.1 开发环境要�?

| 组件       | 最低版�? | 推荐版本   |
| -------- | ----- | ------ |
| C++ 编译�? | C++17 | C++20  |
| CMake    | 3.16  | 3.24+  |
| Qt       | 5.15  | Qt 6.x |
| libcurl  | 7.65  | 8.0+   |
| OpenSSL  | 1.1.1 | 3.0+   |

### 1.2 操作系统要求

- **Windows 10/11**: 需要安�?Visual Studio 2019 或更高版本，或使�?MinGW-w64 编译�?
- **Linux**: Ubuntu 20.04+, Debian 11+, 或其他主流发行版
- **macOS**: 11.0 (Big Sur) 或更高版�?

### 1.3 项目依赖

本功能集成需要以�?Qt 模块�?

- **QtCore**: 基础核心功能
- **QtNetwork**: 网络通信支持
- **QtWidgets**: 用户界面组件（用于配置对话框�?
- **QtSql**: 数据库存储（用于存储邮箱账户信息�?

确保 CMakeLists.txt 中已包含以下模块�?

```cmake
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Network)
```

***

## 2. libcurl库安装与配置

### 2.1 Windows 平台安装

#### 2.1.1 使用 vcpkg 安装（推荐）

vcpkg 是微软开发的 C++ 包管理工具，推荐使用此方式安�?libcurl�?

```powershell
# 安装 vcpkg（如果尚未安装）
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 安装 libcurl（支�?SSL�?
vcpkg install libcurl:x64-windows
```

安装完成后，�?CMakeLists.txt 中添加：

```cmake
# vcpkg 集成
cmake_policy(SET CMP0091 NEW)
set(CMAKE_TOOLCHAIN_FILE "C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain")

find_package(CURL REQUIRED)
target_link_libraries(PersonalDateAssisant PRIVATE CURL::libcurl)
```

#### 2.1.2 使用预编译二进制文件

从官方下�?libcurl 预编译版本：

1. 访问 <https://curl.se/download.html>
2. 下载 Windows 64-bit SSL 版本（libcurl-vc-x64-release-ssl-winssl.zip�?
3. 解压到项目目录的 `lib/` 文件夹下

解压后的目录结构�?

```
lib/
└── curl/
    ├── include/
    �?  └── curl/
    �?      ├── curl.h
    �?      ├── curlver.h
    �?      └── ...
    └── lib/
        ├── libcurl.dll
        ├── libcurl.exp
        └── libcurl.lib
```

�?CMakeLists.txt 中配置：

```cmake
# 手动指定 libcurl 路径
set(CURL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/curl/include")
set(CURL_LIBRARY "${CMAKE_SOURCE_DIR}/lib/curl/lib/libcurl.lib")

find_package(CURL REQUIRED)
target_include_directories(PersonalDateAssisant PRIVATE ${CURL_INCLUDE_DIR})
target_link_libraries(PersonalDateAssisant PRIVATE ${CURL_LIBRARY})
```

### 2.2 Linux 平台安装

#### 2.2.1 使用包管理器安装

```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev

# Fedora/RHEL
sudo dnf install libcurl-devel

# Arch Linux
sudo pacman -S curl
```

#### 2.2.2 源码编译安装

```bash
# 下载源码
wget https://curl.se/download/curl-8.0.1.tar.gz
tar -xzf curl-8.0.1.tar.gz
cd curl-8.0.1

# 配置编译选项
./configure --with-openssl --prefix=/usr/local

# 编译安装
make -j$(nproc)
sudo make install
sudo ldconfig
```

### 2.3 macOS 平台安装

```bash
# 使用 Homebrew（推荐）
brew install curl

# 如果需要特定版�?
brew install curl@7.88.1
```

### 2.4 CMakeLists.txt 完整配置示例

在项目的 CMakeLists.txt 中添加以下内容：

```cmake
# =====================
# libcurl 配置
# =====================

# 尝试使用 vcpkg（Windows�?
if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain")
endif()

# 查找 libcurl
find_package(CURL REQUIRED)

# 添加到项目源文件列表（稍后创建）
set(EMAIL_SOURCES
    email/EmailAccount.h
    email/EmailAccount.cpp
    email/ImapClient.h
    email/ImapClient.cpp
    email/SmtpClient.h
    email/SmtpClient.cpp
    email/EmailMessage.h
    email/EmailMessage.cpp
    email/EmailManager.h
    email/EmailManager.cpp
)

# 更新项目源文�?
set(PROJECT_SOURCES
    ${PROJECT_SOURCES}
    ${EMAIL_SOURCES}
)

# 链接 libcurl
target_link_libraries(PersonalDateAssisant PRIVATE
    CURL::libcurl
    ${PROJECT_SOURCES}
)

# Windows 下需要添�?ws2_32 �?winmm
if(WIN32)
    target_link_libraries(PersonalDateAssisant PRIVATE ws2_32 winmm)
endif()
```

***

## 3. IMAP邮件接收功能实现

### 3.1 IMAP 协议概述

IMAP（Internet Message Access Protocol，互联网消息访问协议）是用于从邮件服务器接收邮件的协议。相�?POP3，IMAP 支持在服务器上直接管理邮件，支持多设备同步�?

常用 IMAP 服务器端口：

- **IMAP**: 143（明文）
- **IMAPS**: 993（SSL/TLS 加密，推荐）

### 3.2 核心类设�?

#### 3.2.1 EmailAccount �?

EmailAccount 类用于存储邮箱账户配置信息：

```cpp
// email/EmailAccount.h
#ifndef EMAILACCOUNT_H
#define EMAILACCOUNT_H

#include <QString>
#include <QFlags>

class EmailAccount {
public:
    enum class Protocol {
        Imap,
        Pop3
    };
    
    enum class SecurityType {
        None,      // 无加�?
        Ssl,       // SSL 加密
        Tls        // STARTTLS
    };
    
    EmailAccount();
    EmailAccount(const QString& email, const QString& password,
                 const QString& imapServer, quint16 imapPort,
                 SecurityType security = SecurityType::Tls);
    
    QString email() const;
    void setEmail(const QString& email);
    
    QString password() const;
    void setPassword(const QString& password);
    
    QString imapServer() const;
    void setImapServer(const QString& server);
    
    quint16 imapPort() const;
    void setImapPort(quint16 port);
    
    SecurityType securityType() const;
    void setSecurityType(SecurityType type);
    
    QString smtpServer() const;
    void setSmtpServer(const QString& server);
    
    quint16 smtpPort() const;
    void setSmtpPort(quint16 port);
    
    SecurityType smtpSecurityType() const;
    void setSmtpSecurityType(SecurityType type);
    
    QString displayName() const;
    void setDisplayName(const QString& name);
    
    bool isValid() const;
    
private:
    QString m_email;
    QString m_password;
    QString m_imapServer;
    quint16 m_imapPort = 993;
    SecurityType m_imapSecurityType = SecurityType::Tls;
    QString m_smtpServer;
    quint16 m_smtpPort = 587;
    SecurityType m_smtpSecurityType = SecurityType::Tls;
    QString m_displayName;
};

#endif // EMAILACCOUNT_H
```

实现文件�?

```cpp
// email/EmailAccount.cpp
#include "EmailAccount.h"

EmailAccount::EmailAccount() = default;

EmailAccount::EmailAccount(const QString& email, const QString& password,
                           const QString& imapServer, quint16 imapPort,
                           SecurityType security)
    : m_email(email)
    , m_password(password)
    , m_imapServer(imapServer)
    , m_imapPort(imapPort)
    , m_imapSecurityType(security)
{}

QString EmailAccount::email() const { return m_email; }
void EmailAccount::setEmail(const QString& email) { m_email = email; }

QString EmailAccount::password() const { return m_password; }
void EmailAccount::setPassword(const QString& password) { m_password = password; }

QString EmailAccount::imapServer() const { return m_imapServer; }
void EmailAccount::setImapServer(const QString& server) { m_imapServer = server; }

quint16 EmailAccount::imapPort() const { return m_imapPort; }
void EmailAccount::setImapPort(quint16 port) { m_imapPort = port; }

EmailAccount::SecurityType EmailAccount::securityType() const { return m_imapSecurityType; }
void EmailAccount::setSecurityType(SecurityType type) { m_imapSecurityType = type; }

QString EmailAccount::smtpServer() const { return m_smtpServer; }
void EmailAccount::setSmtpServer(const QString& server) { m_smtpServer = server; }

quint16 EmailAccount::smtpPort() const { return m_smtpPort; }
void EmailAccount::setSmtpPort(quint16 port) { m_smtpPort = port; }

EmailAccount::SecurityType EmailAccount::smtpSecurityType() const { return m_smtpSecurityType; }
void EmailAccount::setSmtpSecurityType(SecurityType type) { m_smtpSecurityType = type; }

QString EmailAccount::displayName() const { return m_displayName; }
void EmailAccount::setDisplayName(const QString& name) { m_displayName = name; }

bool EmailAccount::isValid() const {
    return !m_email.isEmpty() && 
           !m_password.isEmpty() && 
           !m_imapServer.isEmpty() && 
           m_imapPort > 0;
}
```

#### 3.2.2 EmailMessage �?

EmailMessage 类用于表示邮件内容：

```cpp
// email/EmailMessage.h
#ifndef EMAILMESSAGE_H
#define EMAILMESSAGE_H

#include <QString>
#include <QDateTime>
#include <QMap>
#include <QStringList>

class EmailMessage {
public:
    EmailMessage();
    
    QString uid() const;
    void setUid(const QString& uid);
    
    int sequenceNumber() const;
    void setSequenceNumber(int number);
    
    QString from() const;
    void setFrom(const QString& from);
    
    QString fromAddress() const;
    void setFromAddress(const QString& address);
    
    QString to() const;
    void setTo(const QString& to);
    
    QString cc() const;
    void setCc(const QString& cc);
    
    QString subject() const;
    void setSubject(const QString& subject);
    
    QString body() const;
    void setBody(const QString& body);
    
    QString plainBody() const;
    void setPlainBody(const QString& body);
    
    QString htmlBody() const;
    void setHtmlBody(const QString& body);
    
    QDateTime date() const;
    void setDate(const QDateTime& date);
    
    bool isRead() const;
    void setRead(bool read);
    
    bool isSeen() const;
    void setSeen(bool seen);
    
    bool hasAttachments() const;
    void setHasAttachments(bool has);
    
    QStringList attachments() const;
    void addAttachment(const QString& attachment);
    
    QMap<QString, QString> headers() const;
    void setHeader(const QString& name, const QString& value);
    QString header(const QString& name) const;
    
private:
    QString m_uid;
    int m_sequenceNumber = 0;
    QString m_from;
    QString m_fromAddress;
    QString m_to;
    QString m_cc;
    QString m_subject;
    QString m_body;
    QString m_plainBody;
    QString m_htmlBody;
    QDateTime m_date;
    bool m_read = false;
    bool m_seen = false;
    bool m_hasAttachments = false;
    QStringList m_attachments;
    QMap<QString, QString> m_headers;
};

#endif // EMAILMESSAGE_H
```

实现文件�?

```cpp
// email/EmailMessage.cpp
#include "EmailMessage.h"

EmailMessage::EmailMessage() = default;

QString EmailMessage::uid() const { return m_uid; }
void EmailMessage::setUid(const QString& uid) { m_uid = uid; }

int EmailMessage::sequenceNumber() const { return m_sequenceNumber; }
void EmailMessage::setSequenceNumber(int number) { m_sequenceNumber = number; }

QString EmailMessage::from() const { return m_from; }
void EmailMessage::setFrom(const QString& from) { m_from = from; }

QString EmailMessage::fromAddress() const { return m_fromAddress; }
void EmailMessage::setFromAddress(const QString& address) { m_fromAddress = address; }

QString EmailMessage::to() const { return m_to; }
void EmailMessage::setTo(const QString& to) { m_to = to; }

QString EmailMessage::cc() const { return m_cc; }
void EmailMessage::setCc(const QString& cc) { m_cc = cc; }

QString EmailMessage::subject() const { return m_subject; }
void EmailMessage::setSubject(const QString& subject) { m_subject = subject; }

QString EmailMessage::body() const { return m_body; }
void EmailMessage::setBody(const QString& body) { m_body = body; }

QString EmailMessage::plainBody() const { return m_plainBody; }
void EmailMessage::setPlainBody(const QString& body) { m_plainBody = body; }

QString EmailMessage::htmlBody() const { return m_htmlBody; }
void EmailMessage::setHtmlBody(const QString& body) { m_htmlBody = body; }

QDateTime EmailMessage::date() const { return m_date; }
void EmailMessage::setDate(const QDateTime& date) { m_date = date; }

bool EmailMessage::isRead() const { return m_read; }
void EmailMessage::setRead(bool read) { m_read = read; }

bool EmailMessage::isSeen() const { return m_seen; }
void EmailMessage::setSeen(bool seen) { m_seen = seen; }

bool EmailMessage::hasAttachments() const { return m_hasAttachments; }
void EmailMessage::setHasAttachments(bool has) { m_hasAttachments = has; }

QStringList EmailMessage::attachments() const { return m_attachments; }
void EmailMessage::addAttachment(const QString& attachment) { m_attachments.append(attachment); }

QMap<QString, QString> EmailMessage::headers() const { return m_headers; }
void EmailMessage::setHeader(const QString& name, const QString& value) { m_headers.insert(name, value); }
QString EmailMessage::header(const QString& name) const { return m_headers.value(name); }
```

#### 3.2.3 ImapClient �?

ImapClient 类是 IMAP 协议的核心实现类，使�?libcurl 实现�?

```cpp
// email/ImapClient.h
#ifndef IMAPCLIENT_H
#define IMAPCLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QMutex>
#include <curl/curl.h>
#include "EmailAccount.h"
#include "EmailMessage.h"

class ImapClient : public QObject {
    Q_OBJECT

public:
    explicit ImapClient(QObject* parent = nullptr);
    ~ImapClient();
    
    bool connectToServer(const EmailAccount& account);
    void disconnect();
    
    bool login(const QString& username, const QString& password);
    bool logout();
    
    bool selectFolder(const QString& folder = "INBOX");
    QStringList getFolderList();
    
    int getMessageCount();
    int getUnreadCount();
    
    QList<EmailMessage> fetchEmailList(int start, int count);
    EmailMessage fetchEmail(const QString& uid);
    EmailMessage fetchEmailBySequence(int sequenceNumber);
    
    bool markAsRead(const QString& uid);
    bool markAsUnread(const QString& uid);
    bool deleteEmail(const QString& uid);
    bool moveToFolder(const QString& uid, const QString& folder);
    
    bool isConnected() const;
    QString lastError() const;
    
signals:
    void connectionEstablished();
    void connectionLost();
    void loginSuccessful();
    void loginFailed(const QString& error);
    void emailListFetched(const QList<EmailMessage>& emails);
    void emailFetched(const EmailMessage& email);
    void operationFailed(const QString& error);
    void progress(int current, int total);

private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t headerCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    bool executeCommand(const QString& command, QString& response);
    bool executeCommandWithData(const QString& command, QString& response, QString& data);
    QString parseResponse(const QString& response);
    bool checkResponse(const QString& response, const QString& expected);
    
    QString extractUid(const QString& response);
    QStringList parseEmailList(const QString& response);
    EmailMessage parseEmail(const QString& response);
    
    CURL* m_curl = nullptr;
    QString m_url;
    QString m_response;
    QString m_responseData;
    QString m_lastError;
    bool m_connected = false;
    bool m_loggedIn = false;
    QString m_currentFolder;
    QString m_tag;
    int m_messageCount = 0;
    QMutex m_mutex;
    struct curl_slist* m_headers = nullptr;
};

#endif // IMAPCLIENT_H
```

实现文件（email/ImapClient.cpp）：

```cpp
#include "ImapClient.h"
#include <QDebug>
#include <QRegularExpression>
#include <QUrl>

ImapClient::ImapClient(QObject* parent)
    : QObject(parent)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    m_curl = curl_easy_init();
    if (!m_curl) {
        m_lastError = "Failed to initialize CURL";
    }
}

ImapClient::~ImapClient() {
    disconnect();
    if (m_curl) {
        curl_easy_cleanup(m_curl);
        m_curl = nullptr;
    }
    if (m_headers) {
        curl_slist_free_all(m_headers);
        m_headers = nullptr;
    }
    curl_global_cleanup();
}

bool ImapClient::connectToServer(const EmailAccount& account) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_curl) {
        m_lastError = "CURL not initialized";
        return false;
    }
    
    QString scheme = (account.securityType() == EmailAccount::SecurityType::Ssl) ? "imaps" : "imap";
    m_url = QString("%1://%2:%3/")
                .arg(scheme)
                .arg(account.imapServer())
                .arg(account.imapPort());
    
    curl_easy_setopt(m_curl, CURLOPT_URL, m_url.toUtf8().constData());
    curl_easy_setopt(m_curl, CURLOPT_PORT, static_cast<long>(account.imapPort()));
    
    if (account.securityType() == EmailAccount::SecurityType::None) {
        curl_easy_setopt(m_curl, CURLOPT_USE_SSL, CURLUSSL_NONE);
    } else {
        curl_easy_setopt(m_curl, CURLOPT_USE_SSL, CURLUSSL_TRY);
    }
    
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(m_curl);
    
    if (res != CURLE_OK) {
        m_lastError = QString("Connection failed: %1").arg(curl_easy_strerror(res));
        qWarning() << "IMAP connection error:" << m_lastError;
        return false;
    }
    
    m_connected = true;
    emit connectionEstablished();
    
    QString initResponse;
    if (!executeCommand("", initResponse)) {
        qWarning() << "IMAP initial command failed:" << initResponse;
    }
    
    return login(account.email(), account.password());
}

void ImapClient::disconnect() {
    QMutexLocker locker(&m_mutex);
    
    if (m_loggedIn) {
        logout();
    }
    
    if (m_curl) {
        curl_easy_reset(m_curl);
    }
    
    m_connected = false;
    emit connectionLost();
}

bool ImapClient::login(const QString& username, const QString& password) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_connected) {
        m_lastError = "Not connected to server";
        return false;
    }
    
    m_tag = "A001";
    QString command = QString("%1 LOGIN %2 \"%3\"")
                         .arg(m_tag)
                         .arg(username)
                         .arg(password);
    
    QString response;
    if (!executeCommand(command, response)) {
        m_lastError = "LOGIN command failed";
        emit loginFailed(m_lastError);
        return false;
    }
    
    if (!response.contains(QString("%1 OK").arg(m_tag))) {
        m_lastError = "Login failed: " + response;
        emit loginFailed(m_lastError);
        return false;
    }
    
    m_loggedIn = true;
    emit loginSuccessful();
    return true;
}

bool ImapClient::logout() {
    QMutexLocker locker(&m_mutex);
    
    if (!m_loggedIn) {
        return true;
    }
    
    m_tag = "A002";
    QString command = QString("%1 LOGOUT").arg(m_tag);
    
    QString response;
    executeCommand(command, response);
    
    m_loggedIn = false;
    return true;
}

bool ImapClient::selectFolder(const QString& folder) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_loggedIn) {
        m_lastError = "Not logged in";
        return false;
    }
    
    m_tag = "A003";
    QString command = QString("%1 SELECT %2").arg(m_tag).arg(folder);
    
    QString response;
    if (!executeCommand(command, response)) {
        m_lastError = "SELECT command failed";
        return false;
    }
    
    if (!response.contains(QString("%1 OK").arg(m_tag))) {
        m_lastError = "Failed to select folder: " + folder;
        return false;
    }
    
    QRegularExpression exp("(\\d+) EXISTS");
    QRegularExpressionMatch match = exp.match(response);
    if (match.hasMatch()) {
        m_messageCount = match.captured(1).toInt();
    }
    
    m_currentFolder = folder;
    return true;
}

QStringList ImapClient::getFolderList() {
    QMutexLocker locker(&m_mutex);
    
    QStringList folders;
    
    if (!m_loggedIn) {
        m_lastError = "Not logged in";
        return folders;
    }
    
    m_tag = "A004";
    QString command = QString("%1 LIST \"\" \"*\"").arg(m_tag);
    
    QString response;
    if (!executeCommand(command, response)) {
        m_lastError = "LIST command failed";
        return folders;
    }
    
    QRegularExpression exp("\"([^\"]+)\"");
    QRegularExpressionMatchIterator it = exp.globalMatch(response);
    while (it.hasNext()) {
        QString folder = it.next().captured(1);
        if (!folder.isEmpty() && folder != "*") {
            folders.append(folder);
        }
    }
    
    return folders;
}

int ImapClient::getMessageCount() {
    if (m_currentFolder.isEmpty()) {
        selectFolder("INBOX");
    }
    return m_messageCount;
}

int ImapClient::getUnreadCount() {
    QMutexLocker locker(&m_mutex);
    
    if (!m_loggedIn || m_currentFolder.isEmpty()) {
        return 0;
    }
    
    m_tag = "A005";
    QString command = QString("%1 STATUS %2 (UNSEEN)")
                         .arg(m_tag)
                         .arg(m_currentFolder);
    
    QString response;
    if (!executeCommand(command, response)) {
        return 0;
    }
    
    QRegularExpression exp("UNSEEN (\\d+)");
    QRegularExpressionMatch match = exp.match(response);
    if (match.hasMatch()) {
        return match.captured(1).toInt();
    }
    
    return 0;
}

QList<EmailMessage> ImapClient::fetchEmailList(int start, int count) {
    QMutexLocker locker(&m_mutex);
    
    QList<EmailMessage> emails;
    
    if (!m_loggedIn || m_currentFolder.isEmpty()) {
        m_lastError = "No folder selected";
        return emails;
    }
    
    m_tag = "A006";
    QString command = QString("%1 FETCH %2:%3 (UID FLAGS ENVELOPE)")
                         .arg(m_tag)
                         .arg(start)
                         .arg(start + count - 1);
    
    QString response;
    if (!executeCommand(command, response)) {
        m_lastError = "FETCH command failed";
        return emails;
    }
    
    emails = parseEmailList(response);
    emit emailListFetched(emails);
    
    return emails;
}

EmailMessage ImapClient::fetchEmail(const QString& uid) {
    QMutexLocker locker(&m_mutex);
    
    EmailMessage email;
    
    if (!m_loggedIn || m_currentFolder.isEmpty()) {
        m_lastError = "No folder selected";
        return email;
    }
    
    m_tag = "A007";
    QString command = QString("%1 UID FETCH %2 (UID FLAGS ENVELOPE BODY[])")
                         .arg(m_tag)
                         .arg(uid);
    
    QString response;
    QString data;
    if (!executeCommandWithData(command, response, data)) {
        m_lastError = "FETCH command failed";
        return email;
    }
    
    email = parseEmail(response);
    email.setBody(data);
    
    emit emailFetched(email);
    return email;
}

EmailMessage ImapClient::fetchEmailBySequence(int sequenceNumber) {
    QMutexLocker locker(&m_mutex);
    
    EmailMessage email;
    
    if (!m_loggedIn || m_currentFolder.isEmpty()) {
        m_lastError = "No folder selected";
        return email;
    }
    
    m_tag = "A008";
    QString command = QString("%1 FETCH %2 (UID FLAGS ENVELOPE BODY[])")
                         .arg(m_tag)
                         .arg(sequenceNumber);
    
    QString response;
    QString data;
    if (!executeCommandWithData(command, response, data)) {
        m_lastError = "FETCH command failed";
        return email;
    }
    
    email = parseEmail(response);
    email.setBody(data);
    
    emit emailFetched(email);
    return email;
}

bool ImapClient::markAsRead(const QString& uid) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_loggedIn) {
        m_lastError = "Not logged in";
        return false;
    }
    
    m_tag = "A009";
    QString command = QString("%1 UID STORE %2 +FLAGS (\\Seen)")
                         .arg(m_tag)
                         .arg(uid);
    
    QString response;
    if (!executeCommand(command, response)) {
        m_lastError = "STORE command failed";
        return false;
    }
    
    return response.contains(QString("%1 OK").arg(m_tag));
}

bool ImapClient::markAsUnread(const QString& uid) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_loggedIn) {
        m_lastError = "Not logged in";
        return false;
    }
    
    m_tag = "A010";
    QString command = QString("%1 UID STORE %2 -FLAGS (\\Seen)")
                         .arg(m_tag)
                         .arg(uid);
    
    QString response;
    if (!executeCommand(command, response)) {
        m_lastError = "STORE command failed";
        return false;
    }
    
    return response.contains(QString("%1 OK").arg(m_tag));
}

bool ImapClient::deleteEmail(const QString& uid) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_loggedIn) {
        m_lastError = "Not logged in";
        return false;
    }
    
    m_tag = "A011";
    QString command = QString("%1 UID STORE %2 +FLAGS (\\Deleted)")
                         .arg(m_tag)
                         .arg(uid);
    
    QString response;
    if (!executeCommand(command, response)) {
        m_lastError = "STORE command failed";
        return false;
    }
    
    return response.contains(QString("%1 OK").arg(m_tag));
}

bool ImapClient::moveToFolder(const QString& uid, const QString& folder) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_loggedIn) {
        m_lastError = "Not logged in";
        return false;
    }
    
    m_tag = "A012";
    QString command = QString("%1 UID MOVE %2 %3")
                         .arg(m_tag)
                         .arg(uid)
                         .arg(folder);
    
    QString response;
    if (!executeCommand(command, response)) {
        m_lastError = "MOVE command failed";
        return false;
    }
    
    return response.contains(QString("%1 OK").arg(m_tag));
}

bool ImapClient::isConnected() const {
    return m_connected && m_loggedIn;
}

QString ImapClient::lastError() const {
    return m_lastError;
}

size_t ImapClient::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

size_t ImapClient::headerCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool ImapClient::executeCommand(const QString& command, QString& response) {
    std::string readBuffer;
    std::string headerBuffer;
    
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &headerBuffer);
    curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, headerCallback);
    
    if (!command.isEmpty()) {
        std::string cmd = command.toUtf8().toStdString() + "\r\n";
        curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(m_curl, CURLOPT_READDATA, &cmd);
        curl_easy_setopt(m_curl, CURLOPT_INFILESIZE, static_cast<long>(cmd.size()));
        
        CURLcode res = curl_easy_perform(m_curl);
        
        curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 0L);
        
        if (res != CURLE_OK) {
            m_lastError = QString("CURL error: %1").arg(curl_easy_strerror(res));
            return false;
        }
    }
    
    m_response = QString::fromUtf8(readBuffer.c_str());
    response = m_response;
    
    return true;
}

bool ImapClient::executeCommandWithData(const QString& command, QString& response, QString& data) {
    std::string readBuffer;
    std::string headerBuffer;
    
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &headerBuffer);
    curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, headerCallback);
    
    std::string cmd = command.toUtf8().toStdString() + "\r\n";
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(m_curl, CURLOPT_READDATA, &cmd);
    curl_easy_setopt(m_curl, CURLOPT_INFILESIZE, static_cast<long>(cmd.size()));
    
    CURLcode res = curl_easy_perform(m_curl);
    
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 0L);
    
    if (res != CURLE_OK) {
        m_lastError = QString("CURL error: %1").arg(curl_easy_strerror(res));
        return false;
    }
    
    QString fullResponse = QString::fromUtf8(readBuffer.c_str());
    
    size_t braceStart = fullResponse.indexOf('{');
    size_t braceEnd = fullResponse.indexOf('}');
    
    if (braceStart != -1 && braceEnd != -1) {
        response = fullResponse.left(braceStart);
        data = fullResponse.mid(braceEnd + 1);
    } else {
        response = fullResponse;
        data = "";
    }
    
    m_response = response;
    m_responseData = data;
    
    return true;
}

QString ImapClient::parseResponse(const QString& response) {
    return response;
}

bool ImapClient::checkResponse(const QString& response, const QString& expected) {
    return response.contains(expected);
}

QString ImapClient::extractUid(const QString& response) {
    QRegularExpression exp("UID (\\d+)");
    QRegularExpressionMatch match = exp.match(response);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return QString();
}

QStringList ImapClient::parseEmailList(const QString& response) {
    QStringList uids;
    
    QRegularExpression exp("(\\d+) (?:FETCH|OK)");
    QRegularExpressionMatchIterator it = exp.globalMatch(response);
    while (it.hasNext()) {
        QString uid = it.next().captured(1);
        if (!uid.isEmpty()) {
            uids.append(uid);
        }
    }
    
    return uids;
}

EmailMessage ImapClient::parseEmail(const QString& response) {
    EmailMessage email;
    
    QRegularExpression uidExp("UID (\\d+)");
    QRegularExpressionMatch uidMatch = uidExp.match(response);
    if (uidMatch.hasMatch()) {
        email.setUid(uidMatch.captured(1));
    }
    
    QRegularExpression fromExp("From:.*?<(.+?)>");
    QRegularExpressionMatch fromMatch = fromExp.match(response);
    if (fromMatch.hasMatch()) {
        email.setFromAddress(fromMatch.captured(1));
    } else {
        QRegularExpression fromExp2("From:\\s*([^\\r\\n]+)");
        QRegularExpressionMatch fromMatch2 = fromExp2.match(response);
        if (fromMatch2.hasMatch()) {
            email.setFromAddress(fromMatch2.captured(1).trimmed());
        }
    }
    
    QRegularExpression subjectExp("Subject:\\s*(.+?)(?:\\r?\\n|$)");
    QRegularExpressionMatch subjectMatch = subjectExp.match(response);
    if (subjectMatch.hasMatch()) {
        email.setSubject(subjectMatch.captured(1).trimmed());
    }
    
    QRegularExpression dateExp("(\\d{1,2}\\s+\\w+\\s+\\d{4}\\s+\\d{2}:\\d{2}:\\d{2})");
    QRegularExpressionMatch dateMatch = dateExp.match(response);
    if (dateMatch.hasMatch()) {
        QDateTime date = QDateTime::fromString(dateMatch.captured(1), "d MMM yyyy HH:mm:ss");
        email.setDate(date);
    }
    
    bool hasSeen = response.contains("\\Seen") || response.contains("Seen");
    email.setSeen(hasSeen);
    
    return email;
}
```

### 3.3 IMAP 连接示例

```cpp
void connectToEmail() {
    ImapClient client;
    
    QObject::connect(&client, &ImapClient::connectionEstablished, []() {
        qDebug() << "Connected to IMAP server";
    });
    
    QObject::connect(&client, &ImapClient::loginSuccessful, []() {
        qDebug() << "Login successful";
    });
    
    QObject::connect(&client, &ImapClient::emailListFetched, [](const QList<EmailMessage>& emails) {
        qDebug() << "Fetched" << emails.size() << "emails";
    });
    
    EmailAccount account;
    account.setEmail("user@example.com");
    account.setPassword("password");
    account.setImapServer("imap.example.com");
    account.setImapPort(993);
    account.setSecurityType(EmailAccount::SecurityType::Ssl);
    
    if (client.connectToServer(account)) {
        client.selectFolder("INBOX");
        int count = client.getMessageCount();
        qDebug() << "Total messages:" << count;
        
        QList<EmailMessage> emails = client.fetchEmailList(1, 10);
        for (const EmailMessage& email : emails) {
            qDebug() << "Subject:" << email.subject();
            qDebug() << "From:" << email.fromAddress();
        }
    }
}
```

***

## 4. SMTP邮件发送功能实�?

### 4.1 SMTP 协议概述

SMTP（Simple Mail Transfer Protocol，简单邮件传输协议）是用于发送电子邮件的协议�?

常用 SMTP 服务器端口：

- **SMTP**: 25（明文，不推荐）
- **SMTPS**: 465（SSL 加密�?
- **SMTP**: 587（STARTTLS，推荐）

### 4.2 SmtpClient 类设�?

```cpp
// email/SmtpClient.h
#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QMutex>
#include <curl/curl.h>
#include "EmailAccount.h"

class EmailMessage;

class SmtpClient : public QObject {
    Q_OBJECT

public:
    explicit SmtpClient(QObject* parent = nullptr);
    ~SmtpClient();
    
    bool connectToServer(const EmailAccount& account);
    void disconnect();
    
    bool login(const QString& username, const QString& password);
    bool logout();
    
    bool sendEmail(const EmailMessage& email);
    bool sendRawEmail(const QString& from, const QString& to, const QString& rawEmail);
    
    void setTimeout(int seconds);
    void setDebugMode(bool enabled);
    
    bool isConnected() const;
    QString lastError() const;
    
signals:
    void connectionEstablished();
    void connectionLost();
    void loginSuccessful();
    void loginFailed(const QString& error);
    void emailSent();
    void sendFailed(const QString& error);

private:
    static size_t payloadSourceCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    bool sendCommand(const QString& command, int expectedCode);
    QString readResponse();
    
    bool sendMimeEmail(const EmailMessage& email);
    QString buildMimeMessage(const EmailMessage& email);
    QString encodeBase64(const QByteArray& data);
    QString encodeQuotedPrintable(const QString& text);
    QString encodeSubject(const QString& subject);
    
    CURL* m_curl = nullptr;
    QString m_url;
    QString m_response;
    QString m_lastError;
    bool m_connected = false;
    bool m_loggedIn = false;
    QString m_username;
    QString m_password;
    int m_timeout = 30;
    bool m_debugMode = false;
    QMutex m_mutex;
    QString m_payload;
    size_t m_payloadReadPosition = 0;
};

#endif // SMTPCLIENT_H
```

实现文件（email/SmtpClient.cpp）：

```cpp
#include "SmtpClient.h"
#include "EmailMessage.h"
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>
#include <QUuid>

SmtpClient::SmtpClient(QObject* parent)
    : QObject(parent)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    m_curl = curl_easy_init();
    if (!m_curl) {
        m_lastError = "Failed to initialize CURL";
    }
}

SmtpClient::~SmtpClient() {
    disconnect();
    if (m_curl) {
        curl_easy_cleanup(m_curl);
        m_curl = nullptr;
    }
    curl_global_cleanup();
}

bool SmtpClient::connectToServer(const EmailAccount& account) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_curl) {
        m_lastError = "CURL not initialized";
        return false;
    }
    
    m_username = account.email();
    m_password = account.password();
    
    QString scheme = (account.smtpSecurityType() == EmailAccount::SecurityType::Ssl) ? "smtps" : "smtp";
    m_url = QString("%1://%2:%3/")
                .arg(scheme)
                .arg(account.smtpServer())
                .arg(account.smtpPort());
    
    curl_easy_setopt(m_curl, CURLOPT_URL, m_url.toUtf8().constData());
    curl_easy_setopt(m_curl, CURLOPT_PORT, static_cast<long>(account.smtpPort()));
    
    if (account.smtpSecurityType() == EmailAccount::SecurityType::None) {
        curl_easy_setopt(m_curl, CURLOPT_USE_SSL, CURLUSSL_NONE);
    } else {
        curl_easy_setopt(m_curl, CURLOPT_USE_SSL, CURLUSSL_TRY);
    }
    
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, m_timeout);
    curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, payloadSourceCallback);
    curl_easy_setopt(m_curl, CURLOPT_READDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1L);
    
    CURLcode res = curl_easy_perform(m_curl);
    
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 0L);
    
    if (res != CURLE_OK) {
        m_lastError = QString("Connection failed: %1").arg(curl_easy_strerror(res));
        qWarning() << "SMTP connection error:" << m_lastError;
        return false;
    }
    
    m_connected = true;
    emit connectionEstablished();
    
    return login(account.email(), account.password());
}

void SmtpClient::disconnect() {
    QMutexLocker locker(&m_mutex);
    
    if (m_loggedIn) {
        logout();
    }
    
    if (m_curl) {
        curl_easy_reset(m_curl);
    }
    
    m_connected = false;
    emit connectionLost();
}

bool SmtpClient::login(const QString& username, const QString& password) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_connected) {
        m_lastError = "Not connected to server";
        return false;
    }
    
    m_username = username;
    m_password = password;
    
    QString authString = QString("\0%1\0%2").arg(username).arg(password);
    QString encodedAuth = encodeBase64(authString.toUtf8());
    
    QString command = QString("AUTH PLAIN %1").arg(encodedAuth);
    if (!sendCommand(command, 235)) {
        m_lastError = "AUTH PLAIN failed";
        emit loginFailed(m_lastError);
        return false;
    }
    
    m_loggedIn = true;
    emit loginSuccessful();
    return true;
}

bool SmtpClient::logout() {
    QMutexLocker locker(&m_mutex);
    
    if (!m_loggedIn) {
        return true;
    }
    
    sendCommand("QUIT", 221);
    
    m_loggedIn = false;
    return true;
}

bool SmtpClient::sendEmail(const EmailMessage& email) {
    return sendMimeEmail(email);
}

bool SmtpClient::sendRawEmail(const QString& from, const QString& to, const QString& rawEmail) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_loggedIn) {
        m_lastError = "Not logged in";
        return false;
    }
    
    m_payload = rawEmail;
    m_payloadReadPosition = 0;
    
    curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, payloadSourceCallback);
    curl_easy_setopt(m_curl, CURLOPT_READDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1L);
    
    QString mailFrom = QString("MAIL FROM:<%1>").arg(from);
    if (!sendCommand(mailFrom, 250)) {
        m_lastError = "MAIL FROM failed";
        return false;
    }
    
    QString rcptTo = QString("RCPT TO:<%1>").arg(to);
    if (!sendCommand(rcptTo, 250)) {
        m_lastError = "RCPT TO failed";
        return false;
    }
    
    if (!sendCommand("DATA", 354)) {
        m_lastError = "DATA command failed";
        return false;
    }
    
    CURLcode res = curl_easy_perform(m_curl);
    
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 0L);
    
    if (res != CURLE_OK) {
        m_lastError = QString("Failed to send email: %1").arg(curl_easy_strerror(res));
        emit sendFailed(m_lastError);
        return false;
    }
    
    emit emailSent();
    return true;
}

void SmtpClient::setTimeout(int seconds) {
    m_timeout = seconds;
    if (m_curl) {
        curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, seconds);
    }
}

void SmtpClient::setDebugMode(bool enabled) {
    m_debugMode = enabled;
    if (m_curl) {
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, enabled ? 1L : 0L);
    }
}

bool SmtpClient::isConnected() const {
    return m_connected && m_loggedIn;
}

QString SmtpClient::lastError() const {
    return m_lastError;
}

size_t SmtpClient::payloadSourceCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    SmtpClient* client = static_cast<SmtpClient*>(userp);
    
    if (client->m_payloadReadPosition >= client->m_payload.size()) {
        return 0;
    }
    
    size_t remaining = client->m_payload.size() - client->m_payloadReadPosition;
    size_t toCopy = qMin(size * nmemb, remaining);
    
    memcpy(contents, 
           client->m_payload.constData() + client->m_payloadReadPosition, 
           toCopy);
    
    client->m_payloadReadPosition += toCopy;
    
    return toCopy;
}

bool SmtpClient::sendCommand(const QString& command, int expectedCode) {
    std::string cmd = command.toUtf8().toStdString() + "\r\n";
    
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1L);
    
    m_payload = command + "\r\n";
    m_payloadReadPosition = 0;
    
    CURLcode res = curl_easy_perform(m_curl);
    
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 0L);
    
    if (res != CURLE_OK) {
        m_lastError = QString("CURL error: %1").arg(curl_easy_strerror(res));
        return false;
    }
    
    return true;
}

QString SmtpClient::readResponse() {
    return m_response;
}

bool SmtpClient::sendMimeEmail(const EmailMessage& email) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_loggedIn) {
        m_lastError = "Not logged in";
        return false;
    }
    
    QString mimeMessage = buildMimeMessage(email);
    
    m_payload = mimeMessage;
    m_payloadReadPosition = 0;
    
    curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, payloadSourceCallback);
    curl_easy_setopt(m_curl, CURLOPT_READDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1L);
    
    QString mailFrom = QString("MAIL FROM:<%1>").arg(email.fromAddress());
    if (!sendCommand(mailFrom, 250)) {
        m_lastError = "MAIL FROM failed";
        return false;
    }
    
    QStringList recipients = email.to().split(',', Qt::SkipEmptyParts);
    for (const QString& recipient : recipients) {
        QString rcptTo = QString("RCPT TO:<%1>").arg(recipient.trimmed());
        if (!sendCommand(rcptTo, 250)) {
            m_lastError = "RCPT TO failed for: " + recipient;
            return false;
        }
    }
    
    if (!email.cc().isEmpty()) {
        QStringList ccRecipients = email.cc().split(',', Qt::SkipEmptyParts);
        for (const QString& cc : ccRecipients) {
            QString rcptTo = QString("RCPT TO:<%1>").arg(cc.trimmed());
            if (!sendCommand(rcptTo, 250)) {
                m_lastError = "RCPT TO failed for CC: " + cc;
                return false;
            }
        }
    }
    
    if (!sendCommand("DATA", 354)) {
        m_lastError = "DATA command failed";
        return false;
    }
    
    CURLcode res = curl_easy_perform(m_curl);
    
    curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 0L);
    
    if (res != CURLE_OK) {
        m_lastError = QString("Failed to send email: %1").arg(curl_easy_strerror(res));
        emit sendFailed(m_lastError);
        return false;
    }
    
    emit emailSent();
    return true;
}

QString SmtpClient::buildMimeMessage(const EmailMessage& email) {
    QString messageId = QString("<%1@%2>")
                             .arg(QUuid::createUuid().toString(QUuid::WithoutBraces))
                             .arg("localhost");
    
    QString boundary = QString("----=_Part_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    
    QString mime;
    
    mime += QString("From: %1\r\n").arg(email.from());
    mime += QString("To: %1\r\n").arg(email.to());
    if (!email.cc().isEmpty()) {
        mime += QString("Cc: %1\r\n").arg(email.cc());
    }
    mime += QString("Subject: %1\r\n").arg(encodeSubject(email.subject()));
    mime += QString("Date: %1\r\n").arg(email.date().toString("ddd, dd MMM yyyy HH:mm:ss +0000"));
    mime += QString("Message-ID: %1\r\n").arg(messageId);
    mime += QString("MIME-Version: 1.0\r\n");
    mime += QString("Content-Type: multipart/mixed; boundary=\"%1\"\r\n").arg(boundary);
    mime += "\r\n";
    
    mime += QString("--%1\r\n").arg(boundary);
    mime += "Content-Type: text/plain; charset=UTF-8\r\n";
    mime += "Content-Transfer-Encoding: quoted-printable\r\n";
    mime += "\r\n";
    mime += encodeQuotedPrintable(email.plainBody());
    mime += "\r\n";
    
    if (!email.htmlBody().isEmpty()) {
        mime += QString("--%1\r\n").arg(boundary);
        mime += "Content-Type: text/html; charset=UTF-8\r\n";
        mime += "Content-Transfer-Encoding: quoted-printable\r\n";
        mime += "\r\n";
        mime += encodeQuotedPrintable(email.htmlBody());
        mime += "\r\n";
    }
    
    for (const QString& attachment : email.attachments()) {
        QFile file(attachment);
        if (file.exists()) {
            QFileInfo fileInfo(file);
            QString filename = fileInfo.fileName();
            QByteArray fileData;
            if (file.open(QIODevice::ReadOnly)) {
                fileData = file.readAll();
                file.close();
            }
            
            mime += QString("--%1\r\n").arg(boundary);
            mime += QString("Content-Type: application/octet-stream; name=\"%1\"\r\n").arg(filename);
            mime += "Content-Transfer-Encoding: base64\r\n";
            mime += QString("Content-Disposition: attachment; filename=\"%1\"\r\n").arg(filename);
            mime += "\r\n";
            mime += encodeBase64(fileData);
            mime += "\r\n";
        }
    }
    
    mime += QString("--%1--\r\n").arg(boundary);
    mime += "\r\n.\r\n";
    
    return mime;
}

QString SmtpClient::encodeBase64(const QByteArray& data) {
    return QString::fromLatin1(data.toBase64());
}

QString SmtpClient::encodeQuotedPrintable(const QString& text) {
    QString result;
    QByteArray utf8 = text.toUtf8();
    
    for (int i = 0; i < utf8.size(); ++i) {
        unsigned char c = utf8[i];
        
        if (c == '=' || c == '\r' || c == '\n') {
            result += QString("=%1").arg(c, 2, 16, QChar('0')).toUpper();
        } else if (c < 33 || c > 126 || c == ' ' || c == '\t') {
            result += QString("=%1").arg(c, 2, 16, QChar('0')).toUpper();
        } else {
            result += QChar(c);
        }
    }
    
    return result;
}

QString SmtpClient::encodeSubject(const QString& subject) {
    QByteArray utf8 = subject.toUtf8();
    if (utf8 == subject.toLatin1()) {
        return subject;
    }
    return QString("=?UTF-8?B?%1?=").arg(QString::fromLatin1(utf8.toBase64()));
}
```

### 4.3 SMTP 发送示�?

```cpp
void sendEmail() {
    SmtpClient client;
    
    QObject::connect(&client, &SmtpClient::emailSent, []() {
        qDebug() << "Email sent successfully!";
    });
    
    QObject::connect(&client, &SmtpClient::sendFailed, [](const QString& error) {
        qDebug() << "Send failed:" << error;
    });
    
    EmailAccount account;
    account.setEmail("user@example.com");
    account.setPassword("password");
    account.setSmtpServer("smtp.example.com");
    account.setSmtpPort(587);
    account.setSmtpSecurityType(EmailAccount::SecurityType::Tls);
    
    if (client.connectToServer(account)) {
        EmailMessage email;
        email.setFromAddress("user@example.com");
        email.setFrom("User Name <user@example.com>");
        email.setTo("recipient@example.com");
        email.setCc("cc@example.com");
        email.setSubject("Test Email Subject");
        email.setPlainBody("This is the plain text body.");
        email.setHtmlBody("<html><body><p>This is the HTML body.</p></body></html>");
        
        email.addAttachment("C:/path/to/attachment.pdf");
        
        email.setDate(QDateTime::currentDateTime());
        
        if (client.sendEmail(email)) {
            qDebug() << "Email sent!";
        }
    }
}
```

***

## 5. 错误处理机制设计

### 5.1 错误码定�?

```cpp
// email/EmailError.h
#ifndef EMAILERROR_H
#define EMAILERROR_H

#include <QString>

enum class EmailErrorCode {
    Success = 0,
    
    // 连接错误 (1000-1099)
    ConnectionFailed = 1000,
    ConnectionTimeout = 1001,
    ConnectionRefused = 1002,
    HostNotFound = 1003,
    SslError = 1004,
    
    // 认证错误 (2000-2099)
    AuthenticationFailed = 2000,
    InvalidCredentials = 2001,
    AccountLocked = 2002,
    TwoFactorRequired = 2003,
    
    // 操作错误 (3000-3099)
    OperationFailed = 3000,
    FolderNotFound = 3001,
    MessageNotFound = 3002,
    PermissionDenied = 3003,
    QuotaExceeded = 3004,
    
    // 网络错误 (4000-4099)
    NetworkError = 4000,
    Timeout = 4001,
    Aborted = 4002,
    
    // 协议错误 (5000-5099)
    ProtocolError = 5000,
    InvalidResponse = 5001,
    
    // 未知错误
    UnknownError = 9999
};

struct EmailError {
    EmailErrorCode code;
    QString message;
    QString details;
    
    EmailError(EmailErrorCode c = EmailErrorCode::Success, 
               const QString& m = QString(), 
               const QString& d = QString())
        : code(c), message(m), details(d) {}
    
    bool isSuccess() const { return code == EmailErrorCode::Success; }
    QString toString() const {
        return QString("[%1] %2%3")
            .arg(static_cast<int>(code))
            .arg(message)
            .arg(details.isEmpty() ? QString() : QString(" (%1)").arg(details));
    }
};

#endif // EMAILERROR_H
```

### 5.2 错误处理示例

```cpp
// 使用示例
void processEmailOperation() {
    ImapClient client;
    
    QObject::connect(&client, &ImapClient::operationFailed,
                     [](const QString& error) {
                         qWarning() << "Operation failed:" << error;
                     });
    
    if (!client.connectToServer(account)) {
        EmailError error(EmailErrorCode::ConnectionFailed, 
                         client.lastError());
        handleError(error);
        return;
    }
    
    if (!client.login(username, password)) {
        EmailError error(EmailErrorCode::AuthenticationFailed,
                         "Login failed",
                         client.lastError());
        handleError(error);
        return;
    }
}

void handleError(const EmailError& error) {
    switch (error.code) {
        case EmailErrorCode::ConnectionFailed:
        case EmailErrorCode::ConnectionTimeout:
            showReconnectDialog();
            break;
        case EmailErrorCode::AuthenticationFailed:
            showReauthDialog();
            break;
        case EmailErrorCode::QuotaExceeded:
            showQuotaWarning();
            break;
        default:
            showGenericError(error.toString());
            break;
    }
}
```

***

## 6. 代码结构建议

### 6.1 目录结构

建议在项目中创建以下目录结构�?

```
DataAssistant/
├── email/                          # 邮件功能模块
�?  ├── EmailAccount.h             # 邮箱账户�?
�?  ├── EmailAccount.cpp
�?  ├── EmailMessage.h             # 邮件消息�?
�?  ├── EmailMessage.cpp
�?  ├── ImapClient.h               # IMAP 客户�?
�?  ├── ImapClient.cpp
�?  ├── SmtpClient.h               # SMTP 客户�?
�?  ├── SmtpClient.cpp
�?  ├── EmailManager.h             # 邮件管理器（统一接口�?
�?  ├── EmailManager.cpp
�?  ├── EmailError.h               # 错误定义
�?  └── EmailParser.h              # 邮件解析工具
�?  └── EmailParser.cpp
├── Widget.cpp                     # 主窗�?
├── Widget.h
├── SettingsDialog.cpp             # 设置对话框（添加邮箱配置�?
├── DatabaseManager.cpp             # 数据库管�?
└── CMakeLists.txt
```

### 6.2 EmailManager 统一管理�?

```cpp
// email/EmailManager.h
#ifndef EMAILMANAGER_H
#define EMAILMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include "EmailAccount.h"
#include "ImapClient.h"
#include "SmtpClient.h"

class EmailMessage;

class EmailManager : public QObject {
    Q_OBJECT

public:
    static EmailManager& instance();
    
    void addAccount(const EmailAccount& account);
    void removeAccount(const QString& email);
    void updateAccount(const EmailAccount& account);
    QList<EmailAccount> getAccounts() const;
    EmailAccount getAccount(const QString& email) const;
    
    bool connectAccount(const QString& email);
    void disconnectAccount(const QString& email);
    void disconnectAll();
    
    bool syncEmails(const QString& email, const QString& folder = "INBOX");
    bool sendEmail(const QString& fromAccount, const EmailMessage& email);
    
    QList<EmailMessage> getCachedEmails(const QString& account, const QString& folder);
    
signals:
    void accountAdded(const QString& email);
    void accountRemoved(const QString& email);
    void accountConnected(const QString& email);
    void accountDisconnected(const QString& email);
    void emailsReceived(const QString& account, const QList<EmailMessage>& emails);
    void emailSent(const QString& account, const QString& messageId);
    void errorOccurred(const QString& account, const QString& error);

private:
    explicit EmailManager(QObject* parent = nullptr);
    ~EmailManager();
    EmailManager(const EmailManager&) = delete;
    EmailManager& operator=(const EmailManager&) = delete;
    
    QMap<QString, EmailAccount> m_accounts;
    QMap<QString, ImapClient*> m_imapClients;
    QMap<QString, SmtpClient*> m_smtpClients;
    QMap<QString, QList<EmailMessage>> m_cachedEmails;
};

#endif // EMAILMANAGER_H
```

实现文件（email/EmailManager.cpp）：

````cpp
// email/EmailManager.cpp
#include "EmailManager.h"
#include <QDebug>

EmailManager& EmailManager::instance() {
    static EmailManager instance;
    return instance;
}

EmailManager::EmailManager(QObject* parent)
    : QObject(parent)
{
}

EmailManager::~EmailManager() {
    disconnectAll();
}

void EmailManager::addAccount(const EmailAccount& account) {
    if (!account.isValid()) {
        emit errorOccurred(account.email(), "Invalid account");
        return;
    }
    
    m_accounts.insert(account.email(), account);
    emit accountAdded(account.email());
}

void EmailManager::removeAccount(const QString& email) {
    if (m_imapClients.contains(email)) {
        delete m_imapClients.take(email);
    }
    if (m_smtpClients.contains(email)) {
        delete m_smtpClients.take(email);
    }
    m_accounts.remove(email);
    m_cachedEmails.remove(email);
    emit accountRemoved(email);
}

void EmailManager::updateAccount(const EmailAccount& account) {
    if (m_accounts.contains(account.email())) {
        bool wasConnected = m_imapClients.contains(account.email());
        if (wasConnected) {
            disconnectAccount(account.email());
        }
        m_accounts.insert(account.email(), account);
        if (wasConnected) {
            connectAccount(account.email());
        }
    }
}

QList<EmailAccount> EmailManager::getAccounts() const {
    return m_accounts.values();
}

EmailAccount EmailManager::getAccount(const QString& email) const {
    return m_accounts.value(email);
}

bool EmailManager::connectAccount(const QString& email) {
    if (!m_accounts.contains(email)) {
        emit errorOccurred(email, "Account not found");
        return false;
    }
    
    EmailAccount account = m_accounts.value(email);
    
    ImapClient* imapClient = new ImapClient(this);
    if (!imapClient->connectToServer(account)) {
        emit errorOccurred(email, imapClient->lastError());
        delete imapClient;
        return false;
    }
    m_imapClients.insert(email, imapClient);
    
    SmtpClient* smtpClient = new SmtpClient(this);
    if (!smtpClient->connectToServer(account)) {
        qWarning() << "SMTP connection failed for" << email << ":" << smtpClient->lastError();
        delete smtpClient;
    } else {
        m_smtpClients.insert(email, smtpClient);
    }
    
    emit accountConnected(email);
    return true;
}

void EmailManager::disconnectAccount(const QString& email) {
    if (m_imapClients.contains(email)) {
        m_imapClients.value(email)->disconnect();
        delete m_imapClients.take(email);
    }
    if (m_smtpClients.contains(email)) {
        m_smtpClients.value(email)->disconnect();
        delete m_smtpClients.take(email);
    }
    emit accountDisconnected(email);
}

void EmailManager::disconnectAll() {
    for (const QString& email : m_imapClients.keys()) {
        disconnectAccount(email);
    }
}

bool EmailManager::syncEmails(const QString& email, const QString& folder) {
    if (!m_imapClients.contains(email)) {
        if (!connectAccount(email)) {
            return false;
        }
    }
    
    ImapClient* client = m_imapClients.value(email);
    if (!client->selectFolder(folder)) {
        emit errorOccurred(email, client->lastError());
        return false;
    }
    
    int count = client->getMessageCount();
    if (count <= 0) {
        return true;
    }
    
    QList<EmailMessage> emails = client->fetchEmailList(1, qMin(count, 100));
    
    QString cacheKey = QString("%1:%2").arg(email).arg(folder);
    m_cachedEmails.insert(cacheKey, emails);
    
    emit emailsReceived(email, emails);
    return true;
}

bool EmailManager::sendEmail(const QString& fromAccount, const EmailMessage& email) {
    if (!m_smtpClients.contains(fromAccount)) {
        if (!m_accounts.contains(fromAccount)) {
            emit errorOccurred(fromAccount, "Account not found");
            return false;
        }
        
        EmailAccount account = m_accounts.value(fromAccount);
        SmtpClient* smtpClient = new SmtpClient(this);
        if (!smtpClient->connectToServer(account)) {
            emit errorOccurred(fromAccount, smtpClient->lastError());
            delete smtpClient;
            return false;
        }
        m_smtpClients.insert(fromAccount, smtpClient);
    }
    
    SmtpClient* client = m_smtpClients.value(fromAccount);
    if (!client->sendEmail(email)) {
        emit errorOccurred(fromAccount, client->lastError());
        return false;
    }
    
    emit emailSent(fromAccount, email.header("Message-ID"));
    return true;
}

QList<EmailMessage> EmailManager::getCachedEmails(const QString& account, const QString& folder) {
    QString cacheKey = QString("%1:%2").arg(account).arg(folder);
    return m_cachedEmails.value(cacheKey);
}

---

## 7. 关键API调用示例

### 7.1 添加邮箱账户并同步邮�?

```cpp
void setupEmailAccount() {
    EmailAccount account;
    account.setEmail("user@example.com");
    account.setPassword("your_app_password");
    account.setImapServer("imap.example.com");
    account.setImapPort(993);
    account.setSecurityType(EmailAccount::SecurityType::Ssl);
    account.setSmtpServer("smtp.example.com");
    account.setSmtpPort(587);
    account.setSmtpSecurityType(EmailAccount::SecurityType::Tls);
    account.setDisplayName("My Email");
    
    EmailManager::instance().addAccount(account);
    
    if (EmailManager::instance().connectAccount(account.email())) {
        EmailManager::instance().syncEmails(account.email());
    }
}

void onEmailsReceived(const QString& account, const QList<EmailMessage>& emails) {
    qDebug() << "Received" << emails.size() << "emails from" << account;
    
    for (const EmailMessage& email : emails) {
        qDebug() << "From:" << email.fromAddress();
        qDebug() << "Subject:" << email.subject();
        qDebug() << "Date:" << email.date().toString();
        qDebug() << "Preview:" << email.plainBody().left(100);
    }
}
````

### 7.2 发送邮�?

```cpp
void sendEmail() {
    EmailMessage email;
    email.setFromAddress("user@example.com");
    email.setFrom("User Name <user@example.com>");
    email.setTo("recipient@example.com");
    email.setCc("cc@example.com");
    email.setSubject("Test Email Subject");
    email.setPlainBody("This is the plain text body.");
    email.setHtmlBody("<html><body><p>This is the HTML body.</p></body></html>");
    
    email.addAttachment("C:/path/to/attachment.pdf");
    
    email.setDate(QDateTime::currentDateTime());
    
    if (EmailManager::instance().sendEmail("user@example.com", email)) {
        qDebug() << "Email sent successfully";
    } else {
        qDebug() << "Failed to send email";
    }
}
```

### 7.3 常见邮箱服务配置

#### 7.3.1 QQ 邮箱

```cpp
EmailAccount qqAccount;
qqAccount.setEmail("123456789@qq.com");
qqAccount.setPassword("your_auth_code");  // 需要在QQ邮箱设置中获取授权码
qqAccount.setImapServer("imap.qq.com");
qqAccount.setImapPort(993);
qqAccount.setSecurityType(EmailAccount::SecurityType::Ssl);
qqAccount.setSmtpServer("smtp.qq.com");
qqAccount.setSmtpPort(587);
qqAccount.setSmtpSecurityType(EmailAccount::SecurityType::Tls);
```

#### 7.3.2 Gmail

```cpp
EmailAccount gmailAccount;
gmailAccount.setEmail("youremail@gmail.com");
gmailAccount.setPassword("your_app_password");  // 需要使用应用专用密�?
gmailAccount.setImapServer("imap.gmail.com");
gmailAccount.setImapPort(993);
gmailAccount.setSecurityType(EmailAccount::SecurityType::Ssl);
gmailAccount.setSmtpServer("smtp.gmail.com");
gmailAccount.setSmtpPort(587);
gmailAccount.setSmtpSecurityType(EmailAccount::SecurityType::Tls);
```

#### 7.3.3 企业邮箱（以阿里云为例）

```cpp
EmailAccount aliYunAccount;
aliYunAccount.setEmail("user@yourcompany.com");
aliYunAccount.setPassword("your_password");
aliYunAccount.setImapServer("imap.mxhichina.com");
aliYunAccount.setImapPort(993);
aliYunAccount.setSecurityType(EmailAccount::SecurityType::Ssl);
aliYunAccount.setSmtpServer("smtp.mxhichina.com");
aliYunAccount.setSmtpPort(465);
aliYunAccount.setSmtpSecurityType(EmailAccount::SecurityType::Ssl);
```

***

## 8. 安全注意事项

### 8.1 敏感信息处理

\**绝对禁止�?*

- 在代码中硬编码邮箱密�?
- 将密码提交到版本控制系统
- 在日志中输出密码

\**推荐做法�?*

```cpp
class SecureStorage {
public:
    static QString getPassword(const QString& account) {
#ifdef WIN32
        return getFromWindowsCredential(account);
#elif defined(Q_OS_LINUX)
        return getFromKeyring(account);
#else
        return getFromKeychain(account);
#endif
    }
    
    static void savePassword(const QString& account, const QString& password) {
    }
    
private:
#ifdef WIN32
    static QString getFromWindowsCredential(const QString& account) {
        QString target = QString("EmailAccount:%1").arg(account);
        HANDLE handle;
        if (CredReadW(target.toStdWString().c_str(), CRED_TYPE_GENERIC, 0, &handle)) {
            CREDENTIALW* cred;
            if (CredFree(handle)) {
                return QString::fromWCharArray(cred->CredentialBlob);
            }
        }
        return QString();
    }
#endif
};
```

### 8.2 TLS/SSL 配置

确保使用安全�?SSL/TLS 配置�?

```cpp
void configureSecureConnection(CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSSL_TRY);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
}
```

### 8.3 OAuth2 认证（推荐用�?Gmail/Office 365�?

```cpp
bool authenticateWithOAuth2(CURL* curl, const QString& accessToken) {
    struct curl_slist* headers = NULL;
    QString authHeader = QString("Authorization: Bearer %1").arg(accessToken);
    headers = curl_slist_append(headers, authHeader.toUtf8().constData());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    return true;
}
```

### 8.4 安全检查清�?

- [ ] 使用 SSL/TLS 加密连接
- [ ] 启用证书验证
- [ ] 使用应用专用密码�?OAuth2
- [ ] 安全存储用户凭据
- [ ] 实现连接超时
- [ ] 处理证书错误时拒绝连�?
- [ ] 定期更新 libcurl 版本
- [ ] 不在日志中输出敏感信�?

***

## 9. 测试方法与验证步�?

### 9.1 单元测试

```cpp
#include <QtTest>
#include "ImapClient.h"
#include "SmtpClient.h"
#include "EmailAccount.h"

class TestEmailClient : public QObject {
    Q_OBJECT

private slots:
    void testAccountValidation();
    void testImapConnection();
    void testSmtpConnection();
    void testFetchEmailList();
    void testSendEmail();

private:
    EmailAccount m_testAccount;
};

void TestEmailClient::testAccountValidation() {
    EmailAccount invalidAccount;
    QVERIFY(!invalidAccount.isValid());
    
    EmailAccount validAccount("test@example.com", "password", 
                             "imap.example.com", 993);
    QVERIFY(validAccount.isValid());
}

void TestEmailClient::testImapConnection() {
    ImapClient client;
    EmailAccount account = m_testAccount;
    
    bool connected = client.connectToServer(account);
    QVERIFY(connected);
    
    client.disconnect();
}
```

### 9.2 集成测试

```cpp
void integrationTest() {
    EmailManager& manager = EmailManager::instance();
    
    EmailAccount testAccount;
    testAccount.setEmail("test@example.com");
    testAccount.setPassword("test_password");
    testAccount.setImapServer("imap.example.com");
    testAccount.setImapPort(993);
    testAccount.setSecurityType(EmailAccount::SecurityType::Ssl);
    testAccount.setSmtpServer("smtp.example.com");
    testAccount.setSmtpPort(587);
    testAccount.setSmtpSecurityType(EmailAccount::SecurityType::Tls);
    
    manager.addAccount(testAccount);
    QVERIFY(manager.connectAccount(testAccount.email()));
    
    bool syncResult = manager.syncEmails(testAccount.email());
    QVERIFY(syncResult);
    
    QList<EmailMessage> emails = manager.getCachedEmails(
        testAccount.email(), "INBOX");
    QVERIFY(emails.size() >= 0);
    
    manager.disconnectAccount(testAccount.email());
    manager.removeAccount(testAccount.email());
}
```

### 9.3 测试步骤

1. **环境准备**
   - 安装 libcurl �?OpenSSL 开发库
   - 配置测试邮箱账户（建议使用专门的测试邮箱�?
   - 确保网络可以访问邮件服务�?
2. **连接测试**
   - 测试 IMAP 连接
   - 测试 SMTP 连接
   - 测试 SSL/TLS 握手
   - 测试身份验证
3. **功能测试**
   - 测试邮件列表获取
   - 测试邮件内容获取
   - 测试邮件发�?
   - 测试附件处理
   - 测试中文编码
4. **错误处理测试**
   - 测试无效凭据
   - 测试网络断开
   - 测试服务器超�?
   - 测试配额超限
5. **性能测试**
   - 测试大邮件处�?
   - 测试多附件邮�?
   - 测试邮件数量较多时的性能

***

## 10. 常见问题及解决方�?

### 10.1 连接问题

#### 问题 1：无法连接到 IMAP/SMTP 服务�?

\**可能原因�?*

- 网络连接问题
- 防火墙阻�?
- 服务器地址或端口错�?
- SSL/TLS 配置错误

\**解决方案�?*

```cpp
void debugConnection(const EmailAccount& account) {
    qDebug() << "Testing connection to:" << account.imapServer() 
             << ":" << account.imapPort();
    
    if (account.securityType() == EmailAccount::SecurityType::Ssl) {
        qDebug() << "Using SSL encryption";
    } else if (account.securityType() == EmailAccount::SecurityType::Tls) {
        qDebug() << "Using STARTTLS";
    } else {
        qDebug() << "WARNING: No encryption!";
    }
}
```

检查防火墙设置�?

```powershell
Test-NetConnection -ComputerName imap.example.com -Port 993
Test-NetConnection -ComputerName smtp.example.com -Port 587
```

#### 问题 2：SSL 证书验证失败

\**解决方案�?*

```cpp
curl_easy_setopt(m_curl, CURLOPT_CAINFO, "C:/path/to/ca-bundle.crt");
curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
```

### 10.2 认证问题

#### 问题 3：认证失�?

\**解决方案�?*
对于 Gmail、QQ邮箱等，需要使用应用专用密码而非邮箱登录密码�?

#### 问题 4：需�?OAuth2

\**解决方案�?*
对于 Gmail、Office 365 企业账户，需要实�?OAuth2 认证流程�?

### 10.3 邮件处理问题

#### 问题 5：邮件乱�?

\**解决方案�?*
实现 Base64 �?Quoted-Printable 解码�?

#### 问题 6：大邮件夹同步慢

\**解决方案�?*
实现增量同步、分页加载、后台异步同步�?

### 10.4 QQ 邮箱问题

QQ 邮箱需要使用授权码而非 QQ 密码�?

1. 登录 QQ 邮箱网页�?
2. 进入设置 �?账户
3. 开�?POP3/IMAP/SMTP/Exchange 服务
4. 使用生成的授权码作为密码

***

## 附录

### A. 常用邮件服务器配�?

| 邮箱服务    | IMAP 服务�?             | IMAP 端口 | SMTP 服务�?          | SMTP 端口 | 加密类型    |
| ------- | --------------------- | ------- | ------------------ | ------- | ------- |
| Gmail   | imap.gmail.com        | 993     | smtp.gmail.com     | 587     | SSL/TLS |
| QQ邮箱    | imap.qq.com           | 993     | smtp.qq.com        | 587     | SSL/TLS |
| 163邮箱   | imap.163.com          | 993     | smtp.163.com       | 465     | SSL     |
| Outlook | outlook.office365.com | 993     | smtp.office365.com | 587     | TLS     |

### B. libcurl 错误码参�?

```cpp
CURLE_OK (0)              // 操作成功
CURLE_COULDNT_CONNECT (7) // 无法连接到服务器
CURLE_LOGIN_DENIED (67)   // 登录被拒�?
CURLE_SEND_ERROR (55)     // 发送数据失�?
CURLE_RECV_ERROR (56)     // 接收数据失败
CURLE_SSL_CONNECT_ERROR (35)  // SSL/TLS 握手失败
CURLE_SSL_CERTPROBLEM (60)    // 证书问题
```

### C. IMAP 响应码参�?

```
OK  - 操作成功
NO  - 操作失败
BAD - 命令语法错误或协议错�?
```

### D. SMTP 响应码参�?

```
220 - 服务就绪
250 - 请求命令成功
235 - 认证成功
354 - 开始输入邮件内�?
221 - 服务关闭
550 - 邮箱不可�?
553 - 邮箱名称不允�?
```

***

## 总结

本开发指导文档详细介绍了�?Qt/C++ 项目中集�?IMAP/SMTP 邮件功能的方法。通过使用 libcurl 库，我们可以�?

1. **实现 IMAP 功能**：支持连接、登录、选择文件夹、获取邮件列表、读取邮件内容、标记已�?未读、删除邮件等操作�?
2. **实现 SMTP 功能**：支持发送纯文本邮件、HTML 邮件、带有附件的邮件，支持多收件人、抄送等功能�?
3. \*\*多邮箱支�?\*：通过 EmailManager 统一管理多个邮箱账户，实现独立的连接和操作�?
4. **安全实现**：支�?SSL/TLS 加密连接，推荐使用应用专用密码�?
5. **错误处理**：完善的错误处理机制和详细的错误码定义，便于问题诊断�?
6. **代码结构**：清晰的分层设计，便于维护和扩展�?

开发人员可以根据本指南快速集成邮件功能到日程管理应用中，实现邮件通知和邮件同步功能�?

***

**文档版本**�?.0\
\*\*最后更�?*�?026-04-10\
**适用项目**：PersonalDateAssistant\
\*\*依赖�?*：libcurl 7.65+, Qt 5.15+/6.x   - 测试配额超限

1. **性能测试**
   - 测试大邮件处�?
   - 测试多附件邮�?
   - 测试邮件数量较多时的性能

***

## 10. 常见问题及解决方�?

### 10.1 连接问题

#### 问题 1：无法连接到 IMAP/SMTP 服务�?

\**可能原因�?*

- 网络连接问题
- 防火墙阻�?
- 服务器地址或端口错�?
- SSL/TLS 配置错误

\**解决方案�?*

```cpp
void debugConnection(const EmailAccount& account) {
    qDebug() << "Testing connection to:" << account.imapServer() 
             << ":" << account.imapPort();
    
    if (account.securityType() == EmailAccount::SecurityType::Ssl) {
        qDebug() << "Using SSL encryption";
    } else if (account.securityType() == EmailAccount::SecurityType::Tls) {
        qDebug() << "Using STARTTLS";
    } else {
        qDebug() << "WARNING: No encryption!";
    }
}
```

检查防火墙设置�?

```powershell
# 检查端口是否开�?
Test-NetConnection -ComputerName imap.example.com -Port 993
Test-NetConnection -ComputerName smtp.example.com -Port 587
```

#### 问题 2：SSL 证书验证失败

\**可能原因�?*

- 系统�?CA 证书库不完整
- 服务器使用自签名证书
- 服务器证书已过期

\**解决方案�?*

```cpp
// 指定 CA 证书路径
curl_easy_setopt(m_curl, CURLOPT_CAINFO, "C:/path/to/ca-bundle.crt");

// 或者禁用证书验证（仅用于测试，不推荐生产环境）
curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
```

### 10.2 认证问题

#### 问题 3：认证失败（用户名或密码错误�?

\**可能原因�?*

- 用户名或密码不正�?
- 使用了错误的认证方式
- 邮箱服务要求应用专用密码

\**解决方案�?*
对于 Gmail、QQ邮箱等，需要使用应用专用密码而非邮箱登录密码�?

#### 问题 4：需要二次验证或 OAuth2

\**解决方案�?*
对于需�?OAuth2 的服务（Gmail、Office 365 企业账户），需要实�?OAuth2 认证流程�?

1. 使用 Qt WebEngine �?QWebEngineView 打开 OAuth 授权页面
2. 获取授权码并交换访问令牌
3. 使用访问令牌进行 SMTP/IMAP 认证

### 10.3 邮件处理问题

#### 问题 5：邮件主题或正文乱码

\**可能原因�?*

- 编码声明与实际编码不一�?
- 中文编码使用 GBK 而非 UTF-8

\**解决方案�?*

```cpp
QString decodeEmailHeader(const QString& encoded) {
    if (encoded.contains("=?")) {
        QRegularExpression exp("=\\?([^?]+)\\?([BQ])\\?([^?]+)\\?=", 
                              QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = exp.match(encoded);
        if (match.hasMatch()) {
            QString charset = match.captured(1);
            QString encoding = match.captured(2).toUpper();
            QString content = match.captured(3);
            
            if (encoding == "B") {
                QByteArray decoded = QByteArray::fromBase64(content.toLatin1());
                return QString::fromUtf8(decoded);
            } else if (encoding == "Q") {
                return decodeQuotedPrintable(content);
            }
        }
    }
    return encoded;
}
```

### 10.4 性能问题

#### 问题 6：大邮件夹同步速度�?

\**解决方案�?*

- 实现增量同步，只获取新邮�?
- 使用分页加载，避免一次性获取所有邮�?
- 实现后台异步同步

### 10.5 特定邮箱服务问题

#### 问题 7：QQ 邮箱连接被拒�?

\**解决方案�?*
QQ 邮箱需要使用授权码而非 QQ 密码�?

1. 登录 QQ 邮箱网页�?
2. 进入设置 �?账户
3. 开�?POP3/IMAP/SMTP/Exchange 服务
4. 使用生成的授权码作为密码

***

## 附录

### A. 常用邮件服务器配�?

| 邮箱服务    | IMAP 服务�?             | IMAP 端口 | SMTP 服务�?          | SMTP 端口 | 加密类型    |
| ------- | --------------------- | ------- | ------------------ | ------- | ------- |
| Gmail   | imap.gmail.com        | 993     | smtp.gmail.com     | 587     | SSL/TLS |
| QQ邮箱    | imap.qq.com           | 993     | smtp.qq.com        | 587     | SSL/TLS |
| 163邮箱   | imap.163.com          | 993     | smtp.163.com       | 465     | SSL     |
| Outlook | outlook.office365.com | 993     | smtp.office365.com | 587     | TLS     |

### B. libcurl 错误码参�?

```cpp
// libcurl 常见错误�?
CURLE_OK (0)              // 操作成功
CURLE_COULDNT_CONNECT (7) // 无法连接到服务器
CURLE_LOGIN_DENIED (67)   // 登录被拒�?
CURLE_SEND_ERROR (55)     // 发送数据失�?
CURLE_RECV_ERROR (56)     // 接收数据失败
CURLE_SSL_CONNECT_ERROR (35)  // SSL/TLS 握手失败
CURLE_SSL_CERTPROBLEM (60)    // 证书问题
```

### C. IMAP 响应码参�?

```
OK  - 操作成功
NO  - 操作失败
BAD - 命令语法错误或协议错�?
```

### D. SMTP 响应码参�?

```
220 - 服务就绪
250 - 请求命令成功
235 - 认证成功
354 - 开始输入邮件内�?
221 - 服务关闭
550 - 邮箱不可�?
553 - 邮箱名称不允�?
```

***

## 总结

本开发指导文档详细介绍了�?Qt/C++ 项目中集�?IMAP/SMTP 邮件功能的方法。通过使用 libcurl 库，我们可以�?

1. **实现 IMAP 功能**：支持连接、登录、选择文件夹、获取邮件列表、读取邮件内容、标记已�?未读、删除邮件等操作�?
2. **实现 SMTP 功能**：支持发送纯文本邮件、HTML 邮件、带有附件的邮件，支持多收件人、抄送等功能�?
3. \*\*多邮箱支�?\*：通过 EmailManager 统一管理多个邮箱账户，实现独立的连接和操作�?
4. **安全实现**：支�?SSL/TLS 加密连接，推荐使用应用专用密码�?
5. **错误处理**：完善的错误处理机制和详细的错误码定义，便于问题诊断�?
6. **代码结构**：清晰的分层设计，便于维护和扩展�?

开发人员可以根据本指南快速集成邮件功能到日程管理应用中，实现邮件通知和邮件同步功能�?

***

**文档版本**�?.0\
\*\*最后更�?*�?026-04-10\
**适用项目**：PersonalDateAssistant\
\*\*依赖�?*：libcurl 7.65+, Qt 5.15+/6.x

\**绝对禁止�?*

- 在代码中硬编码邮箱密�?
- 将密码提交到版本控制系统
- 在日志中输出密码

\**推荐做法�?*

```cpp
class SecureStorage {
public:
    static QString getPassword(const QString& account) {
#ifdef WIN32
        return getFromWindowsCredential(account);
#elif defined(Q_OS_LINUX)
        return getFromKeyring(account);
#else
        return getFromKeychain(account);
#endif
    }
    
    static void savePassword(const QString& account, const QString& password) {
    }
    
private:
#ifdef WIN32
    static QString getFromWindowsCredential(const QString& account) {
        QString target = QString("EmailAccount:%1").arg(account);
        HANDLE handle;
        if (CredReadW(target.toStdWString().c_str(), CRED_TYPE_GENERIC, 0, &handle)) {
            CREDENTIALW* cred;
            if (CredFree(handle)) {
                return QString::fromWCharArray(cred->CredentialBlob);
            }
        }
        return QString();
    }
#endif
};
```

### 8.2 TLS/SSL 配置

确保使用安全�?SSL/TLS 配置�?

```cpp
void configureSecureConnection(CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSSL_TRY);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
}
```

### 8.3 OAuth2 认证（推荐用�?Gmail/Office 365�?

```cpp
bool authenticateWithOAuth2(CURL* curl, const QString& accessToken) {
    struct curl_slist* headers = NULL;
    QString authHeader = QString("Authorization: Bearer %1").arg(accessToken);
    headers = curl_slist_append(headers, authHeader.toUtf8().constData());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    return true;
}
```

### 8.4 安全检查清�?

- [ ] 使用 SSL/TLS 加密连接
- [ ] 启用证书验证
- [ ] 使用应用专用密码�?OAuth2
- [ ] 安全存储用户凭据
- [ ] 实现连接超时
- [ ] 处理证书错误时拒绝连�?
- [ ] 定期更新 libcurl 版本
- [ ] 不在日志中输出敏感信�?

***

## 9. 测试方法与验证步�?

### 9.1 单元测试

```cpp
#include <QtTest>
#include "ImapClient.h"
#include "SmtpClient.h"
#include "EmailAccount.h"

class TestEmailClient : public QObject {
    Q_OBJECT

private slots:
    void testAccountValidation();
    void testImapConnection();
    void testSmtpConnection();
    void testFetchEmailList();
    void testSendEmail();

private:
    EmailAccount m_testAccount;
};

void TestEmailClient::testAccountValidation() {
    EmailAccount invalidAccount;
    QVERIFY(!invalidAccount.isValid());
    
    EmailAccount validAccount("test@example.com", "password", 
                             "imap.example.com", 993);
    QVERIFY(validAccount.isValid());
}

void TestEmailClient::testImapConnection() {
    ImapClient client;
    EmailAccount account = m_testAccount;
    
    bool connected = client.connectToServer(account);
    QVERIFY(connected);
    
    client.disconnect();
}
```

### 9.2 集成测试

```cpp
void integrationTest() {
    EmailManager& manager = EmailManager::instance();
    
    EmailAccount testAccount;
    testAccount.setEmail("test@example.com");
    testAccount.setPassword("test_password");
    testAccount.setImapServer("imap.example.com");
    testAccount.setImapPort(993);
    testAccount.setSecurityType(EmailAccount::SecurityType::Ssl);
    testAccount.setSmtpServer("smtp.example.com");
    testAccount.setSmtpPort(587);
    testAccount.setSmtpSecurityType(EmailAccount::SecurityType::Tls);
    
    manager.addAccount(testAccount);
    QVERIFY(manager.connectAccount(testAccount.email()));
    
    bool syncResult = manager.syncEmails(testAccount.email());
    QVERIFY(syncResult);
    
    QList<EmailMessage> emails = manager.getCachedEmails(
        testAccount.email(), "INBOX");
    QVERIFY(emails.size() >= 0);
    
    manager.disconnectAccount(testAccount.email());
    manager.removeAccount(testAccount.email());
}
```

### 9.3 测试步骤

1. **环境准备**
   - 安装 libcurl �?OpenSSL 开发库
   - 配置测试邮箱账户（建议使用专门的测试邮箱�?
   - 确保网络可以访问邮件服务�?
2. **连接测试**
   - 测试 IMAP 连接
   - 测试 SMTP 连接
   - 测试 SSL/TLS 握手
   - 测试身份验证
3. **功能测试**
   - 测试邮件列表获取
   - 测试邮件内容获取
   - 测试邮件发�?
   - 测试附件处理
   - 测试中文编码
4. **错误处理测试**
   - 测试无效凭据
   - 测试网络断开
   - 测试服务器超�?
   - 测试配额超限
5. **性能测试**
   - 测试大邮件处�?
   - 测试多附件邮�?
   - 测试邮件数量较多时的性能

***

## 10. 常见问题及解决方�?

### 10.1 连接问题

#### 问题 1：无法连接到 IMAP/SMTP 服务�?

\**可能原因�?*

- 网络连接问题
- 防火墙阻�?
- 服务器地址或端口错�?
- SSL/TLS 配置错误

\**解决方案�?*

```cpp
void debugConnection(const EmailAccount& account) {
    qDebug() << "Testing connection to:" << account.imapServer() 
             << ":" << account.imapPort();
    
    if (account.securityType() == EmailAccount::SecurityType::Ssl) {
        qDebug() << "Using SSL encryption";
    } else if (account.securityType() == EmailAccount::SecurityType::Tls) {
        qDebug() << "Using STARTTLS";
    } else {
        qDebug() << "WARNING: No encryption!";
    }
}
```

检查防火墙设置�?

```powershell
# 检查端口是否开�?
Test-NetConnection -ComputerName imap.example.com -Port 993
Test-NetConnection -ComputerName smtp.example.com -Port 587
```

#### 问题 2：SSL 证书验证失败

\**可能原因�?*

- 系统�?CA 证书库不完整
- 服务器使用自签名证书
- 服务器证书已过期

\**解决方案�?*

```cpp
// 指定 CA 证书路径
curl_easy_setopt(m_curl, CURLOPT_CAINFO, "C:/path/to/ca-bundle.crt");

// 或者禁用证书验证（仅用于测试，不推荐生产环境）
curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
```

### 10.2 认证问题

#### 问题 3：认证失败（用户名或密码错误�?

\**可能原因�?*

- 用户名或密码不正�?
- 使用了错误的认证方式
- 邮箱服务要求应用专用密码

\**解决方案�?*
对于 Gmail、QQ邮箱等，需要使用应用专用密码而非邮箱登录密码�?

```cpp
// Gmail 申请应用专用密码�?
// Google 账户 �?安全�?�?应用专用密码

// QQ邮箱申请授权码：
// QQ邮箱设置 �?账户 �?POP3/IMAP/SMTP/Exchange/CardDAV/CalDAV服务
// �?开�?IMAP/SMTP 服务 �?获取授权�?
```

#### 问题 4：需要二次验证或 OAuth2

\**解决方案�?*
对于需�?OAuth2 的服务（Gmail、Office 365 企业账户），需要实�?OAuth2 认证流程�?

1. 使用 Qt WebEngine �?QWebEngineView 打开 OAuth 授权页面
2. 获取授权码并交换访问令牌
3. 使用访问令牌进行 SMTP/IMAP 认证

### 10.3 邮件处理问题

#### 问题 5：邮件主题或正文乱码

\**可能原因�?*

- 编码声明与实际编码不一�?
- 中文编码使用 GBK 而非 UTF-8

\**解决方案�?*

```cpp
QString decodeEmailHeader(const QString& encoded) {
    if (encoded.contains("=?")) {
        QRegularExpression exp("=\\?([^?]+)\\?([BQ])\\?([^?]+)\\?=", 
                              QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = exp.match(encoded);
        if (match.hasMatch()) {
            QString charset = match.captured(1);
            QString encoding = match.captured(2).toUpper();
            QString content = match.captured(3);
            
            if (encoding == "B") {
                QByteArray decoded = QByteArray::fromBase64(content.toLatin1());
                return QString::fromUtf8(decoded);
            } else if (encoding == "Q") {
                // Quoted-printable 解码
                return decodeQuotedPrintable(content);
            }
        }
    }
    return encoded;
}
```

#### 问题 6：附件下载失败或损坏

\**解决方案�?*

```cpp
bool downloadAttachment(const QString& uid, const QString& attachmentId, 
                       const QString& savePath) {
    ImapClient client;
    if (!client.connectToServer(account)) {
        return false;
    }
    
    client.selectFolder("INBOX");
    
    QString command = QString("UID FETCH %1 (ENVELOPE BODY[1.2])").arg(uid);
    QString response;
    if (!client.executeCommand(command, response)) {
        return false;
    }
    
    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(response.toUtf8());
        file.close();
        return true;
    }
    
    return false;
}
```

### 10.4 性能问题

#### 问题 7：大邮件夹同步速度�?

\**解决方案�?*

- 实现增量同步，只获取新邮�?
- 使用分页加载，避免一次性获取所有邮�?
- 实现后台异步同步

```cpp
class EmailSyncWorker : public QThread {
    Q_OBJECT
public:
    void run() override {
        ImapClient client;
        if (!client.connectToServer(m_account)) {
            return;
        }
        
        client.selectFolder(m_folder);
        int total = client.getMessageCount();
        
        int pageSize = 50;
        for (int i = 1; i <= total; i += pageSize) {
            emit progress(i, total);
            QList<EmailMessage> emails = client.fetchEmailList(i, pageSize);
            emit emailsFetched(emails);
            
            QThread::msleep(100);  // 避免请求过快
        }
    }
    
signals:
    void progress(int current, int total);
    void emailsFetched(const QList<EmailMessage>& emails);
};
```

### 10.5 特定邮箱服务问题

#### 问题 8：QQ 邮箱连接被拒�?

\**解决方案�?*
QQ 邮箱需要使用授权码而非 QQ 密码�?

1. 登录 QQ 邮箱网页�?
2. 进入设置 �?账户
3. 开�?POP3/IMAP/SMTP/Exchange 服务
4. 使用生成的授权码作为密码

```cpp
EmailAccount qqAccount;
qqAccount.setEmail("your_qq_number@qq.com");
qqAccount.setPassword("your_auth_code");  // 不是 QQ 密码�?
qqAccount.setImapServer("imap.qq.com");
qqAccount.setImapPort(993);
qqAccount.setSecurityType(EmailAccount::SecurityType::Ssl);
```

#### 问题 9：企业邮箱无法连�?

\**解决方案�?*
企业邮箱通常需要额外配置：

```cpp
EmailAccount enterpriseAccount;
enterpriseAccount.setEmail("user@company.com");
enterpriseAccount.setPassword("your_password");

// 询问管理员获取正确的服务器地址
enterpriseAccount.setImapServer("imap.company.com");
enterpriseAccount.setImapPort(993);
enterpriseAccount.setSecurityType(EmailAccount::SecurityType::Ssl);

// 部分企业邮箱使用非标准端�?
enterpriseAccount.setSmtpServer("smtp.company.com");
enterpriseAccount.setSmtpPort(465);
enterpriseAccount.setSmtpSecurityType(EmailAccount::SecurityType::Ssl);
```

***

## 附录

### A. 常用邮件服务器配�?

| 邮箱服务    | IMAP 服务�?             | IMAP 端口 | SMTP 服务�?          | SMTP 端口 | 加密类型    |
| ------- | --------------------- | ------- | ------------------ | ------- | ------- |
| Gmail   | imap.gmail.com        | 993     | smtp.gmail.com     | 587     | SSL/TLS |
| QQ邮箱    | imap.qq.com           | 993     | smtp.qq.com        | 587     | SSL/TLS |
| 163邮箱   | imap.163.com          | 993     | smtp.163.com       | 465     | SSL     |
| Outlook | outlook.office365.com | 993     | smtp.office365.com | 587     | TLS     |

### B. libcurl 错误码参�?

```cpp
// libcurl 常见错误�?
CURLE_OK (0)              // 操作成功
CURLE_COULDNT_CONNECT (7) // 无法连接到服务器
CURLE_LOGIN_DENIED (67)   // 登录被拒�?
CURLE_SEND_ERROR (55)     // 发送数据失�?
CURLE_RECV_ERROR (56)     // 接收数据失败
CURLE_SSL_CONNECT_ERROR (35)  // SSL/TLS 握手失败
CURLE_SSL_CERTPROBLEM (60)    // 证书问题
```

### C. IMAP 响应码参�?

```
OK  - 操作成功
NO  - 操作失败
BAD - 命令语法错误或协议错�?
```

### D. SMTP 响应码参�?

```
220 - 服务就绪
250 - 请求命令成功
235 - 认证成功
354 - 开始输入邮件内�?
221 - 服务关闭
550 - 邮箱不可�?
553 - 邮箱名称不允�?
```

***

## 总结

本开发指导文档详细介绍了�?Qt/C++ 项目中集�?IMAP/SMTP 邮件功能的方法。通过使用 libcurl 库，我们可以�?

1. **实现 IMAP 功能**：支持连接、登录、选择文件夹、获取邮件列表、读取邮件内容、标记已�?未读、删除邮件等操作�?
2. **实现 SMTP 功能**：支持发送纯文本邮件、HTML 邮件、带有附件的邮件，支持多收件人、抄送等功能�?
3. \*\*多邮箱支�?\*：通过 EmailManager 统一管理多个邮箱账户，实现独立的连接和操作�?
4. **安全实现**：支�?SSL/TLS 加密连接，推荐使用应用专用密码�?
5. **错误处理**：完善的错误处理机制和详细的错误码定义，便于问题诊断�?
6. **代码结构**：清晰的分层设计，便于维护和扩展�?

开发人员可以根据本指南快速集成邮件功能到日程管理应用中，实现邮件通知和邮件同步功能�?

***

**文档版本**�?.0\
\*\*最后更�?*�?026-04-10\
**适用项目**：PersonalDateAssistant\
\*\*依赖�?*：libcurl 7.65+, Qt 5.15+/6.x
