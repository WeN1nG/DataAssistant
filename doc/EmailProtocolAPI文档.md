# Email Protocol API Documentation

## 概述

本模块实现了完整的IMAP和SMTP协议支持，用于电子邮件的接收和发送功能。

## 文件结构

```
ResourceCode/email/
├── ImapClient.h/cpp       - IMAP协议客户端实现
├── SmtpClient.h/cpp      - SMTP协议客户端实现
├── EmailProtocolTest.h/cpp - 协议测试代码
├── EmailModels.h         - 数据模型定义
└── EmailDataManager.h/cpp - 邮件数据管理
```

## IMAP客户端 (ImapClient)

### 基本用法

```cpp
ImapClient* imap = new ImapClient(this);

// 连接服务器
imap->connectToServer("imap.example.com", 993, true);

// 登录
imap->login("username@example.com", "password");

// 选择邮箱
imap->selectMailbox("INBOX");

// 获取邮件列表
QVector<ImapClient::EmailInfo> emails = imap->fetchEmailList(1, 100);

// 获取邮件内容
QString body = imap->fetchEmailBody(1, true);

// 搜索邮件
QVector<int> results = imap->searchEmails("subject:urgent");

// 删除邮件
imap->deleteEmail(1);

// 断开连接
imap->disconnect();
```

### 主要方法

#### 连接管理

| 方法 | 说明 |
|------|------|
| `connectToServer(host, port, useSsl)` | 连接到IMAP服务器 |
| `login(username, password)` | 用户认证 |
| `logout()` | 登出 |
| `disconnect()` | 断开连接 |

#### 邮箱操作

| 方法 | 说明 |
|------|------|
| `selectMailbox(name)` | 选择邮箱（可读写） |
| `examineMailbox(name)` | 打开邮箱（只读） |
| `listMailboxes()` | 列出所有邮箱 |
| `createMailbox(name)` | 创建新邮箱 |
| `deleteMailbox(name)` | 删除邮箱 |
| `renameMailbox(oldName, newName)` | 重命名邮箱 |
| `getMailboxStatus(name)` | 获取邮箱状态 |

#### 邮件操作

| 方法 | 说明 |
|------|------|
| `fetchEmailList(start, count)` | 获取邮件列表 |
| `fetchEmailHeader(msgNum)` | 获取邮件头信息 |
| `fetchEmailBody(msgNum, markAsRead)` | 获取邮件正文 |
| `deleteEmail(msgNum)` | 删除邮件（标记为已删除） |
| `moveEmail(msgNum, destination)` | 移动邮件 |
| `copyEmail(msgNum, destination)` | 复制邮件 |
| `searchEmails(criteria)` | 搜索邮件 |
| `setFlags(msgNum, flags)` | 设置邮件标志 |
| `addFlags(msgNum, flags)` | 添加邮件标志 |
| `removeFlags(msgNum, flags)` | 移除邮件标志 |

### 搜索条件

```cpp
QMap<ImapClient::SearchCriteria, QString> criteria;
criteria[ImapClient::SearchCriteria::From] = "john@example.com";
criteria[ImapClient::SearchCriteria::Subject] = "urgent";
criteria[ImapClient::SearchCriteria::Since] = "01-Jan-2024";

QVector<int> results = imap->searchEmails(criteria);
```

### 错误处理

```cpp
ImapClient::ImapError error = imap->error();
QString errorMsg = imap->errorString();

if (error != ImapClient::ImapError::NoError) {
    qDebug() << "Error:" << errorMsg;
}
```

### 信号

| 信号 | 说明 |
|------|------|
| `connected()` | 连接成功 |
| `disconnected()` | 连接断开 |
| `authenticated()` | 认证成功 |
| `errorOccurred(error, message)` | 发生错误 |
| `mailboxSelected(name)` | 邮箱已选中 |
| `emailReceived(msgNum)` | 收到新邮件 |
| `emailDeleted(msgNum)` | 邮件已删除 |

## SMTP客户端 (SmtpClient)

### 基本用法

```cpp
SmtpClient* smtp = new SmtpClient(this);

// 连接服务器
smtp->connectToServer("smtp.example.com", 587, true);

// 登录
smtp->login("username@example.com", "password", SmtpClient::AuthMethod::Login);

// 发送文本邮件
SmtpClient::Email email;
email.from = "sender@example.com";
email.fromName = "Sender Name";
email.to.append("recipient@example.com");
email.cc.append("cc@example.com");
email.bcc.append("bcc@example.com");
email.subject = "Test Subject";
email.body = "Email body text";
email.format = SmtpClient::EmailFormat::Text;

smtp->sendEmail(email);

// 发送HTML邮件
email.htmlBody = "<html><body><h1>HTML Content</h1></body></html>";
email.format = SmtpClient::EmailFormat::Html;

// 发送带附件的邮件
SmtpClient::Attachment attachment("C:/path/to/file.pdf");
email.attachments.append(attachment);

smtp->sendEmail(email);

// 断开连接
smtp->disconnect();
```

### 主要方法

#### 连接管理

| 方法 | 说明 |
|------|------|
| `connectToServer(host, port, useSsl)` | 连接到SMTP服务器 |
| `login(username, password, method)` | 用户认证 |
| `logout()` | 登出 |
| `disconnect()` | 断开连接 |

#### 邮件发送

| 方法 | 说明 |
|------|------|
| `sendEmail(email)` | 发送完整邮件 |
| `sendTextEmail(from, fromName, to, subject, body)` | 发送纯文本邮件 |
| `sendHtmlEmail(from, fromName, to, subject, htmlBody, cc, bcc)` | 发送HTML邮件 |

#### 工具方法

| 方法 | 说明 |
|------|------|
| `generateMessageId(domain)` | 生成唯一消息ID |
| `encodeBase64(text)` | Base64编码 |
| `encodeQuotedPrintable(text)` | Quoted-Printable编码 |
| `encodeMimeString(text, charset)` | MIME字符串编码 |

### 认证方式

```cpp
enum class AuthMethod {
    None,     // 无认证
    Plain,    // PLAIN认证
    Login,    // LOGIN认证
    CramMd5   // CRAM-MD5认证
};
```

### 附件处理

```cpp
// 从文件创建附件
SmtpClient::Attachment attachment("C:/documents/report.pdf");

// 或手动创建
SmtpClient::Attachment manualAttachment;
manualAttachment.fileName = "document.pdf";
manualAttachment.mimeType = "application/pdf";
manualAttachment.data = fileData;
manualAttachment.filePath = "/path/to/file";
```

### MIME类型支持

内置支持以下MIME类型：
- 文本：txt, html, css, js, json, xml
- 图片：jpg, png, gif, bmp, svg, ico
- 文档：pdf, doc, docx, xls, xlsx, ppt, pptx
- 压缩：zip, rar, 7z, tar, gz
- 音频：mp3, wav
- 视频：mp4, avi, mov

### 错误处理

```cpp
SmtpClient::SmtpError error = smtp->error();
QString errorMsg = smtp->errorString();

if (error != SmtpClient::SmtpError::NoError) {
    qDebug() << "SMTP Error:" << errorMsg;
}
```

### 信号

| 信号 | 说明 |
|------|------|
| `connected()` | 连接成功 |
| `disconnected()` | 连接断开 |
| `authenticated()` | 认证成功 |
| `emailSent(messageId)` | 邮件发送成功 |
| `errorOccurred(error, message)` | 发生错误 |
| `progressUpdated(operation, current, total)` | 操作进度更新 |

## 使用示例

### 完整的邮件收发示例

```cpp
// 初始化
ImapClient* imap = new ImapClient();
SmtpClient* smtp = new SmtpClient();

// 连接IMAP
if (imap->connectToServer("imap.gmail.com", 993)) {
    if (imap->login("user@gmail.com", "password")) {
        imap->selectMailbox("INBOX");
        QVector<ImapClient::EmailInfo> emails = imap->fetchEmailList();
    }
}

// 连接SMTP
if (smtp->connectToServer("smtp.gmail.com", 587)) {
    if (smtp->login("user@gmail.com", "password")) {
        SmtpClient::Email email;
        email.from = "user@gmail.com";
        email.to.append("recipient@example.com");
        email.subject = "Test";
        email.body = "Hello World";
        smtp->sendEmail(email);
    }
}

// 清理
imap->disconnect();
smtp->disconnect();
```

### 带错误处理的完整示例

```cpp
class EmailManager : public QObject {
    Q_OBJECT
public:
    EmailManager() {
        m_imap = new ImapClient(this);
        m_smtp = new SmtpClient(this);

        connect(m_imap, &ImapClient::errorOccurred, this, &EmailManager::handleImapError);
        connect(m_smtp, &SmtpClient::errorOccurred, this, &EmailManager::handleSmtpError);
        connect(m_smtp, &SmtpClient::emailSent, this, &EmailManager::handleEmailSent);
    }

public slots:
    void sendEmail(const QString& to, const QString& subject, const QString& body) {
        SmtpClient::Email email;
        email.from = m_account.emailAddress;
        email.fromName = m_account.displayName;
        email.to.append(to);
        email.subject = subject;
        email.body = body;
        email.format = SmtpClient::EmailFormat::Text;

        if (m_smtp->sendEmail(email)) {
            qDebug() << "Email sent successfully";
        }
    }

private slots:
    void handleImapError(ImapClient::ImapError error, const QString& message) {
        qDebug() << "IMAP Error:" << error << message;
    }

    void handleSmtpError(SmtpClient::SmtpError error, const QString& message) {
        qDebug() << "SMTP Error:" << error << message;
    }

    void handleEmailSent(const QString& messageId) {
        qDebug() << "Email sent with ID:" << messageId;
    }

private:
    ImapClient* m_imap;
    SmtpClient* m_smtp;
    EmailAccount m_account;
};
```

## 注意事项

1. **SSL/TLS**: 建议使用SSL连接（IMAP默认993端口，SMTP建议587端口）
2. **错误处理**: 始终检查返回值和错误信号
3. **资源管理**: 使用完记得调用disconnect()释放资源
4. **线程安全**: 网络操作在主线程进行，避免阻塞UI
5. **超时设置**: 默认超时30秒，可根据网络状况调整

## 错误码说明

### IMAP错误码

| 错误码 | 说明 |
|--------|------|
| NoError | 无错误 |
| ConnectionFailed | 连接失败 |
| AuthenticationFailed | 认证失败 |
| SocketError | 套接字错误 |
| Timeout | 操作超时 |
| InvalidResponse | 无效响应 |
| CommandFailed | 命令执行失败 |
| NotAuthenticated | 未认证 |
| MailboxNotSelected | 未选择邮箱 |
| NetworkError | 网络错误 |

### SMTP错误码

| 错误码 | 说明 |
|--------|------|
| NoError | 无错误 |
| ConnectionFailed | 连接失败 |
| AuthenticationFailed | 认证失败 |
| SocketError | 套接字错误 |
| Timeout | 操作超时 |
| InvalidResponse | 无效响应 |
| CommandFailed | 命令执行失败 |
| NotConnected | 未连接 |
| NotAuthenticated | 未认证 |
| InvalidRecipient | 无效收件人 |
| InvalidSender | 无效发件人 |
| AttachmentFailed | 附件处理失败 |
| NetworkError | 网络错误 |
| SSLFailed | SSL错误 |
