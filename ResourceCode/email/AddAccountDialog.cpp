#include "AddAccountDialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QRegularExpressionValidator>

using namespace fa;

AddImapAccountDialog::AddImapAccountDialog(QWidget* parent)
    : QDialog(parent)
    , m_awesome(new QtAwesome(qApp))
{
    m_awesome->initFontAwesome();

    setWindowTitle("添加IMAP账户");
    setMinimumWidth(380);
    setStyleSheet("background-color: #f5f5f5;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    QLabel* titleLabel = new QLabel("添加IMAP账户", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; padding-bottom: 8px;");
    mainLayout->addWidget(titleLabel);

    QWidget* formWidget = new QWidget(this);
    formWidget->setStyleSheet("background-color: white; border: 1px solid #e0e0e0; border-radius: 6px; padding: 15px;");
    QFormLayout* formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText("your@email.com");
    m_emailEdit->setMinimumHeight(28);
    m_emailEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QRegularExpressionValidator* emailValidator = new QRegularExpressionValidator(
        QRegularExpression("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"), this);
    m_emailEdit->setValidator(emailValidator);
    QLabel* emailLabel = new QLabel("邮箱账号:", this);
    emailLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    emailLabel->setFixedHeight(17);
    formLayout->addRow(emailLabel, m_emailEdit);

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("用户名（通常与邮箱相同）");
    m_usernameEdit->setMinimumHeight(28);
    m_usernameEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* usernameLabel = new QLabel("用户名:", this);
    usernameLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    usernameLabel->setFixedHeight(17);
    formLayout->addRow(usernameLabel, m_usernameEdit);

    m_serverEdit = new QLineEdit(this);
    m_serverEdit->setPlaceholderText("imap.example.com");
    m_serverEdit->setMinimumHeight(28);
    m_serverEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* serverLabel = new QLabel("服务器地址:", this);
    serverLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    serverLabel->setFixedHeight(17);
    formLayout->addRow(serverLabel, m_serverEdit);

    QHBoxLayout* portLayout = new QHBoxLayout();
    portLayout->setSpacing(8);

    m_portSpin = new QSpinBox(this);
    m_portSpin->setMinimum(1);
    m_portSpin->setMaximum(65535);
    m_portSpin->setValue(993);
    qDebug() << "IMAP port set to:" << m_portSpin->value();
    m_portSpin->setMinimumHeight(28);
    m_portSpin->setFixedWidth(70);
    m_portSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_portSpin->setStyleSheet("padding: 2px 4px; font-size: 12px;");
    portLayout->addWidget(m_portSpin);
    portLayout->addStretch();

    m_encryptionCombo = new QComboBox(this);
    m_encryptionCombo->addItem("SSL/TLS", 1);
    m_encryptionCombo->addItem("STARTTLS", 2);
    m_encryptionCombo->addItem("无加密", 0);
    m_encryptionCombo->setMinimumHeight(28);
    m_encryptionCombo->setFixedWidth(100);
    m_encryptionCombo->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    portLayout->addWidget(m_encryptionCombo);

    QLabel* portLabel = new QLabel("端口号:", this);
    portLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    portLabel->setFixedHeight(17);
    formLayout->addRow(portLabel, portLayout);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("密码或应用专用密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(28);
    m_passwordEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* passwordLabel = new QLabel("密码:", this);
    passwordLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    passwordLabel->setFixedHeight(17);
    formLayout->addRow(passwordLabel, m_passwordEdit);

    mainLayout->addWidget(formWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setIcon(m_awesome->icon(fa_solid, fa_xmark));
    m_cancelBtn->setIconSize(QSize(14, 14));
    m_cancelBtn->setMinimumHeight(32);
    m_cancelBtn->setMinimumWidth(90);
    m_cancelBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 5px; padding: 6px 16px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #d32f2f; }");
    connect(m_cancelBtn, &QPushButton::clicked, this, &AddImapAccountDialog::onCancel);
    btnLayout->addWidget(m_cancelBtn);

    m_saveBtn = new QPushButton("保存", this);
    m_saveBtn->setIcon(m_awesome->icon(fa_solid, fa_check));
    m_saveBtn->setIconSize(QSize(14, 14));
    m_saveBtn->setMinimumHeight(32);
    m_saveBtn->setMinimumWidth(90);
    m_saveBtn->setStyleSheet("QPushButton { background-color: #4caf50; color: white; border: none; border-radius: 5px; padding: 6px 16px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #45a049; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &AddImapAccountDialog::onSave);
    btnLayout->addWidget(m_saveBtn);

    mainLayout->addLayout(btnLayout);
}

AddImapAccountDialog::~AddImapAccountDialog() {
}

EmailAccount AddImapAccountDialog::getAccount() const {
    EmailAccount account;
    account.emailAddress = m_emailEdit->text().trimmed();
    account.displayName = m_emailEdit->text().trimmed();
    account.imapServer = m_serverEdit->text().trimmed();
    account.imapPort = m_portSpin->value();
    qDebug() << "IMAP getAccount - port value:" << account.imapPort;
    account.username = m_usernameEdit->text().trimmed();
    account.password = m_passwordEdit->text();
    account.isActive = true;
    return account;
}

void AddImapAccountDialog::onSave() {
    if (m_emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入邮箱账号");
        m_emailEdit->setFocus();
        return;
    }

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(m_emailEdit->text().trimmed()).hasMatch()) {
        QMessageBox::warning(this, "验证错误", "请输入正确的邮箱格式\n例如: your@email.com");
        m_emailEdit->setFocus();
        return;
    }

    if (m_serverEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入服务器地址");
        m_serverEdit->setFocus();
        return;
    }

    if (m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入密码");
        m_passwordEdit->setFocus();
        return;
    }

    accept();
}

void AddImapAccountDialog::onCancel() {
    reject();
}

AddSmtpAccountDialog::AddSmtpAccountDialog(QWidget* parent)
    : QDialog(parent)
    , m_awesome(new QtAwesome(qApp))
{
    m_awesome->initFontAwesome();

    setWindowTitle("添加SMTP账户");
    setMinimumWidth(380);
    setStyleSheet("background-color: #f5f5f5;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    QLabel* titleLabel = new QLabel("添加SMTP账户", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; padding-bottom: 8px;");
    mainLayout->addWidget(titleLabel);

    QWidget* formWidget = new QWidget(this);
    formWidget->setStyleSheet("background-color: white; border: 1px solid #e0e0e0; border-radius: 6px; padding: 15px;");
    QFormLayout* formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText("your@email.com");
    m_emailEdit->setMinimumHeight(28);
    m_emailEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QRegularExpressionValidator* emailValidator = new QRegularExpressionValidator(
        QRegularExpression("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"), this);
    m_emailEdit->setValidator(emailValidator);
    QLabel* smtpEmailLabel = new QLabel("邮箱账号:", this);
    smtpEmailLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    smtpEmailLabel->setFixedHeight(17);
    formLayout->addRow(smtpEmailLabel, m_emailEdit);

    m_serverEdit = new QLineEdit(this);
    m_serverEdit->setPlaceholderText("smtp.example.com");
    m_serverEdit->setMinimumHeight(28);
    m_serverEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* smtpServerLabel = new QLabel("服务器地址:", this);
    smtpServerLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    smtpServerLabel->setFixedHeight(17);
    formLayout->addRow(smtpServerLabel, m_serverEdit);

    QHBoxLayout* portLayout = new QHBoxLayout();
    portLayout->setSpacing(8);

    m_portSpin = new QSpinBox(this);
    m_portSpin->setMinimum(1);
    m_portSpin->setMaximum(65535);
    m_portSpin->setValue(587);
    qDebug() << "SMTP port set to:" << m_portSpin->value();
    m_portSpin->setMinimumHeight(28);
    m_portSpin->setFixedWidth(70);
    m_portSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_portSpin->setStyleSheet("padding: 2px 4px; font-size: 12px;");
    portLayout->addWidget(m_portSpin);
    portLayout->addStretch();

    m_encryptionCombo = new QComboBox(this);
    m_encryptionCombo->addItem("STARTTLS", 2);
    m_encryptionCombo->addItem("SSL/TLS", 1);
    m_encryptionCombo->addItem("无加密", 0);
    m_encryptionCombo->setMinimumHeight(28);
    m_encryptionCombo->setFixedWidth(100);
    m_encryptionCombo->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    portLayout->addWidget(m_encryptionCombo);

    QLabel* smtpPortLabel = new QLabel("端口号:", this);
    smtpPortLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    smtpPortLabel->setFixedHeight(17);
    formLayout->addRow(smtpPortLabel, portLayout);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("密码或应用专用密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(28);
    m_passwordEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* smtpPasswordLabel = new QLabel("密码:", this);
    smtpPasswordLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    smtpPasswordLabel->setFixedHeight(17);
    formLayout->addRow(smtpPasswordLabel, m_passwordEdit);

    mainLayout->addWidget(formWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setIcon(m_awesome->icon(fa_solid, fa_xmark));
    m_cancelBtn->setIconSize(QSize(14, 14));
    m_cancelBtn->setMinimumHeight(32);
    m_cancelBtn->setMinimumWidth(90);
    m_cancelBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 5px; padding: 6px 16px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #d32f2f; }");
    connect(m_cancelBtn, &QPushButton::clicked, this, &AddSmtpAccountDialog::onCancel);
    btnLayout->addWidget(m_cancelBtn);

    m_saveBtn = new QPushButton("保存", this);
    m_saveBtn->setIcon(m_awesome->icon(fa_solid, fa_check));
    m_saveBtn->setIconSize(QSize(14, 14));
    m_saveBtn->setMinimumHeight(32);
    m_saveBtn->setMinimumWidth(90);
    m_saveBtn->setStyleSheet("QPushButton { background-color: #2196f3; color: white; border: none; border-radius: 5px; padding: 6px 16px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #1e88e5; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &AddSmtpAccountDialog::onSave);
    btnLayout->addWidget(m_saveBtn);

    mainLayout->addLayout(btnLayout);
}

AddSmtpAccountDialog::~AddSmtpAccountDialog() {
}

EmailAccount AddSmtpAccountDialog::getAccount() const {
    EmailAccount account;
    account.emailAddress = m_emailEdit->text().trimmed();
    account.displayName = m_emailEdit->text().trimmed();
    account.smtpServer = m_serverEdit->text().trimmed();
    account.smtpPort = m_portSpin->value();
    qDebug() << "SMTP getAccount - port value:" << account.smtpPort;
    account.username = m_emailEdit->text().trimmed();
    account.password = m_passwordEdit->text();
    account.isActive = true;
    return account;
}

void AddSmtpAccountDialog::onSave() {
    if (m_emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入邮箱账号");
        m_emailEdit->setFocus();
        return;
    }

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(m_emailEdit->text().trimmed()).hasMatch()) {
        QMessageBox::warning(this, "验证错误", "请输入正确的邮箱格式\n例如: your@email.com");
        m_emailEdit->setFocus();
        return;
    }

    if (m_serverEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入服务器地址");
        m_serverEdit->setFocus();
        return;
    }

    if (m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入密码");
        m_passwordEdit->setFocus();
        return;
    }

    accept();
}

void AddSmtpAccountDialog::onCancel() {
    reject();
}

EditImapAccountDialog::EditImapAccountDialog(const EmailAccount& account, QWidget* parent)
    : QDialog(parent)
    , m_account(account)
    , m_awesome(new QtAwesome(qApp))
{
    m_awesome->initFontAwesome();

    setWindowTitle("编辑IMAP账户");
    setMinimumWidth(380);
    setStyleSheet("background-color: #f5f5f5;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    QLabel* titleLabel = new QLabel("编辑IMAP账户", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; padding-bottom: 8px;");
    mainLayout->addWidget(titleLabel);

    QWidget* formWidget = new QWidget(this);
    formWidget->setStyleSheet("background-color: white; border: 1px solid #e0e0e0; border-radius: 6px; padding: 15px;");
    QFormLayout* formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setText(account.emailAddress);
    m_emailEdit->setMinimumHeight(28);
    m_emailEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* emailLabel = new QLabel("邮箱账号:", this);
    emailLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    emailLabel->setFixedHeight(17);
    formLayout->addRow(emailLabel, m_emailEdit);

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setText(account.username);
    m_usernameEdit->setPlaceholderText("用户名");
    m_usernameEdit->setMinimumHeight(28);
    m_usernameEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* usernameLabel = new QLabel("用户名:", this);
    usernameLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    usernameLabel->setFixedHeight(17);
    formLayout->addRow(usernameLabel, m_usernameEdit);

    m_serverEdit = new QLineEdit(this);
    m_serverEdit->setText(account.imapServer);
    m_serverEdit->setMinimumHeight(28);
    m_serverEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* serverLabel = new QLabel("服务器地址:", this);
    serverLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    serverLabel->setFixedHeight(17);
    formLayout->addRow(serverLabel, m_serverEdit);

    QHBoxLayout* portLayout = new QHBoxLayout();
    portLayout->setSpacing(8);

    m_portSpin = new QSpinBox(this);
    m_portSpin->setMinimum(1);
    m_portSpin->setMaximum(65535);
    m_portSpin->setValue(account.imapPort);
    m_portSpin->setMinimumHeight(28);
    m_portSpin->setFixedWidth(70);
    m_portSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_portSpin->setStyleSheet("padding: 2px 4px; font-size: 12px;");
    portLayout->addWidget(m_portSpin);
    portLayout->addStretch();

    m_encryptionCombo = new QComboBox(this);
    m_encryptionCombo->addItem("SSL/TLS", 1);
    m_encryptionCombo->addItem("STARTTLS", 2);
    m_encryptionCombo->addItem("无加密", 0);
    m_encryptionCombo->setMinimumHeight(28);
    m_encryptionCombo->setFixedWidth(100);
    m_encryptionCombo->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    portLayout->addWidget(m_encryptionCombo);

    QLabel* portLabel = new QLabel("端口号:", this);
    portLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    portLabel->setFixedHeight(17);
    formLayout->addRow(portLabel, portLayout);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setText(account.password);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(28);
    m_passwordEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* passwordLabel = new QLabel("密码:", this);
    passwordLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    passwordLabel->setFixedHeight(17);
    formLayout->addRow(passwordLabel, m_passwordEdit);

    mainLayout->addWidget(formWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setIcon(m_awesome->icon(fa_solid, fa_xmark));
    m_cancelBtn->setIconSize(QSize(14, 14));
    m_cancelBtn->setMinimumHeight(32);
    m_cancelBtn->setMinimumWidth(90);
    m_cancelBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 5px; padding: 6px 16px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #d32f2f; }");
    connect(m_cancelBtn, &QPushButton::clicked, this, &EditImapAccountDialog::onCancel);
    btnLayout->addWidget(m_cancelBtn);

    m_saveBtn = new QPushButton("保存", this);
    m_saveBtn->setIcon(m_awesome->icon(fa_solid, fa_check));
    m_saveBtn->setIconSize(QSize(14, 14));
    m_saveBtn->setMinimumHeight(32);
    m_saveBtn->setMinimumWidth(90);
    m_saveBtn->setStyleSheet("QPushButton { background-color: #4caf50; color: white; border: none; border-radius: 5px; padding: 6px 16px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #45a049; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &EditImapAccountDialog::onSave);
    btnLayout->addWidget(m_saveBtn);

    mainLayout->addLayout(btnLayout);
}

EditImapAccountDialog::~EditImapAccountDialog() {
}

EmailAccount EditImapAccountDialog::getAccount() const {
    EmailAccount account = m_account;
    account.emailAddress = m_emailEdit->text().trimmed();
    account.displayName = m_usernameEdit->text().trimmed();
    account.imapServer = m_serverEdit->text().trimmed();
    account.imapPort = m_portSpin->value();
    account.username = m_usernameEdit->text().trimmed();
    account.password = m_passwordEdit->text();
    account.isActive = true;
    return account;
}

void EditImapAccountDialog::onSave() {
    if (m_emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入邮箱账号");
        m_emailEdit->setFocus();
        return;
    }

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(m_emailEdit->text().trimmed()).hasMatch()) {
        QMessageBox::warning(this, "验证错误", "请输入正确的邮箱格式\n例如: your@email.com");
        m_emailEdit->setFocus();
        return;
    }

    if (m_usernameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入用户名");
        m_usernameEdit->setFocus();
        return;
    }

    if (m_serverEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入服务器地址");
        m_serverEdit->setFocus();
        return;
    }

    if (m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入密码");
        m_passwordEdit->setFocus();
        return;
    }

    accept();
}

void EditImapAccountDialog::onCancel() {
    reject();
}

EditSmtpAccountDialog::EditSmtpAccountDialog(const EmailAccount& account, QWidget* parent)
    : QDialog(parent)
    , m_account(account)
    , m_awesome(new QtAwesome(qApp))
{
    m_awesome->initFontAwesome();

    setWindowTitle("编辑SMTP账户");
    setMinimumWidth(380);
    setStyleSheet("background-color: #f5f5f5;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    QLabel* titleLabel = new QLabel("编辑SMTP账户", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; padding-bottom: 8px;");
    mainLayout->addWidget(titleLabel);

    QWidget* formWidget = new QWidget(this);
    formWidget->setStyleSheet("background-color: white; border: 1px solid #e0e0e0; border-radius: 6px; padding: 15px;");
    QFormLayout* formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setText(account.emailAddress);
    m_emailEdit->setMinimumHeight(28);
    m_emailEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* smtpEmailLabel = new QLabel("邮箱账号:", this);
    smtpEmailLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    smtpEmailLabel->setFixedHeight(17);
    formLayout->addRow(smtpEmailLabel, m_emailEdit);

    m_serverEdit = new QLineEdit(this);
    m_serverEdit->setText(account.smtpServer);
    m_serverEdit->setMinimumHeight(28);
    m_serverEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* smtpServerLabel = new QLabel("服务器地址:", this);
    smtpServerLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    smtpServerLabel->setFixedHeight(17);
    formLayout->addRow(smtpServerLabel, m_serverEdit);

    QHBoxLayout* portLayout = new QHBoxLayout();
    portLayout->setSpacing(8);

    m_portSpin = new QSpinBox(this);
    m_portSpin->setMinimum(1);
    m_portSpin->setMaximum(65535);
    m_portSpin->setValue(account.smtpPort);
    m_portSpin->setMinimumHeight(28);
    m_portSpin->setFixedWidth(70);
    m_portSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_portSpin->setStyleSheet("padding: 2px 4px; font-size: 12px;");
    portLayout->addWidget(m_portSpin);
    portLayout->addStretch();

    m_encryptionCombo = new QComboBox(this);
    m_encryptionCombo->addItem("STARTTLS", 2);
    m_encryptionCombo->addItem("SSL/TLS", 1);
    m_encryptionCombo->addItem("无加密", 0);
    m_encryptionCombo->setMinimumHeight(28);
    m_encryptionCombo->setFixedWidth(100);
    m_encryptionCombo->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    portLayout->addWidget(m_encryptionCombo);

    QLabel* smtpPortLabel = new QLabel("端口号:", this);
    smtpPortLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    smtpPortLabel->setFixedHeight(17);
    formLayout->addRow(smtpPortLabel, portLayout);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setText(account.password);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(28);
    m_passwordEdit->setStyleSheet("padding: 4px 8px; font-size: 13px;");
    QLabel* smtpPasswordLabel = new QLabel("密码:", this);
    smtpPasswordLabel->setStyleSheet("font-size: 15px; margin: 0; padding: 0;");
    smtpPasswordLabel->setFixedHeight(17);
    formLayout->addRow(smtpPasswordLabel, m_passwordEdit);

    mainLayout->addWidget(formWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setIcon(m_awesome->icon(fa_solid, fa_xmark));
    m_cancelBtn->setIconSize(QSize(14, 14));
    m_cancelBtn->setMinimumHeight(32);
    m_cancelBtn->setMinimumWidth(90);
    m_cancelBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 5px; padding: 6px 16px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #d32f2f; }");
    connect(m_cancelBtn, &QPushButton::clicked, this, &EditSmtpAccountDialog::onCancel);
    btnLayout->addWidget(m_cancelBtn);

    m_saveBtn = new QPushButton("保存", this);
    m_saveBtn->setIcon(m_awesome->icon(fa_solid, fa_check));
    m_saveBtn->setIconSize(QSize(14, 14));
    m_saveBtn->setMinimumHeight(32);
    m_saveBtn->setMinimumWidth(90);
    m_saveBtn->setStyleSheet("QPushButton { background-color: #2196f3; color: white; border: none; border-radius: 5px; padding: 6px 16px; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #1e88e5; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &EditSmtpAccountDialog::onSave);
    btnLayout->addWidget(m_saveBtn);

    mainLayout->addLayout(btnLayout);
}

EditSmtpAccountDialog::~EditSmtpAccountDialog() {
}

EmailAccount EditSmtpAccountDialog::getAccount() const {
    EmailAccount account = m_account;
    account.emailAddress = m_emailEdit->text().trimmed();
    account.displayName = m_emailEdit->text().trimmed();
    account.smtpServer = m_serverEdit->text().trimmed();
    account.smtpPort = m_portSpin->value();
    account.username = m_emailEdit->text().trimmed();
    account.password = m_passwordEdit->text();
    account.isActive = true;
    return account;
}

void EditSmtpAccountDialog::onSave() {
    if (m_emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入邮箱账号");
        m_emailEdit->setFocus();
        return;
    }

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(m_emailEdit->text().trimmed()).hasMatch()) {
        QMessageBox::warning(this, "验证错误", "请输入正确的邮箱格式\n例如: your@email.com");
        m_emailEdit->setFocus();
        return;
    }

    if (m_serverEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入服务器地址");
        m_serverEdit->setFocus();
        return;
    }

    if (m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请输入密码");
        m_passwordEdit->setFocus();
        return;
    }

    accept();
}

void EditSmtpAccountDialog::onCancel() {
    reject();
}
