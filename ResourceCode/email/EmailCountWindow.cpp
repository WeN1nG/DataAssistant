#include "EmailCountWindow.h"
#include "AddAccountDialog.h"
#include <QApplication>
#include <QGroupBox>
#include <QLineEdit>
#include <QMessageBox>
#include "../Database/DatabaseManager.h"

using namespace fa;

EmailAccountItem::EmailAccountItem(QWidget* parent)
    : QFrame(parent)
{
    m_awesome = new fa::QtAwesome(qApp);
    m_awesome->initFontAwesome();

    setObjectName("accountItem");
    setFrameShape(QFrame::StyledPanel);
    setLineWidth(1);
    setStyleSheet("QFrame#accountItem { background-color: white; border: 1px solid #e0e0e0; border-radius: 6px; margin: 5px 0; } QFrame#accountItem:hover { border-color: #1976d2; }");

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 12, 15, 12);
    mainLayout->setSpacing(12);

    m_accountNameLabel = new QLabel(this);
    m_accountNameLabel->setObjectName("accountNameLabel");
    m_accountNameLabel->setStyleSheet("color: #333; font-size: 14px; font-weight: bold;");
    mainLayout->addWidget(m_accountNameLabel, 1);

    m_editBtn = new QPushButton(this);
    m_editBtn->setObjectName("editBtn");
    m_editBtn->setIcon(m_awesome->icon(fa_solid, fa_pen_to_square));
    m_editBtn->setText("编辑");
    m_editBtn->setIconSize(QSize(14, 14));
    m_editBtn->setMinimumHeight(28);
    m_editBtn->setStyleSheet("QPushButton { background-color: #2196f3; color: white; border: none; border-radius: 4px; padding: 4px 12px; font-weight: bold; font-size: 12px; } QPushButton:hover { background-color: #1e88e5; }");
    mainLayout->addWidget(m_editBtn);

    m_deleteBtn = new QPushButton(this);
    m_deleteBtn->setObjectName("deleteBtn");
    m_deleteBtn->setIcon(m_awesome->icon(fa_solid, fa_trash));
    m_deleteBtn->setText("删除");
    m_deleteBtn->setIconSize(QSize(14, 14));
    m_deleteBtn->setMinimumHeight(28);
    m_deleteBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 4px; padding: 4px 12px; font-weight: bold; font-size: 12px; } QPushButton:hover { background-color: #d32f2f; }");
    mainLayout->addWidget(m_deleteBtn);

    setMinimumHeight(50);

    connect(m_editBtn, &QPushButton::clicked, [this]() {
        emit editClicked(m_account);
    });

    connect(m_deleteBtn, &QPushButton::clicked, [this]() {
        emit deleteClicked(m_account);
    });
}

EmailAccountItem::~EmailAccountItem() {
    delete m_awesome;
}

void EmailAccountItem::setAccount(const EmailAccount& account) {
    m_account = account;
    updateDisplay();
}

void EmailAccountItem::updateDisplay() {
    QString displayName = m_account.displayName.isEmpty() ? m_account.emailAddress : m_account.displayName;
    m_accountNameLabel->setText(displayName);
}

void EmailAccountItem::mousePressEvent(QMouseEvent* event) {
    QFrame::mousePressEvent(event);
}

EmailCountWindow::EmailCountWindow(QWidget* parent)
    : QWidget(parent)
    , m_awesome(new QtAwesome(qApp))
{
    m_awesome->initFontAwesome();

    setupUI();
    loadAccounts();
}

EmailCountWindow::~EmailCountWindow() {
}

void EmailCountWindow::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);

    setupImapSection();
    setupSmtpSection();

    m_mainLayout->addStretch();
}

void EmailCountWindow::setupImapSection() {
    m_imapSection = new QWidget(this);
    QVBoxLayout* sectionLayout = new QVBoxLayout(m_imapSection);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    sectionLayout->setSpacing(15);

    QWidget* headerWidget = new QWidget(m_imapSection);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(10);

    QLabel* imapTitle = new QLabel("IMAP账户", m_imapSection);
    imapTitle->setObjectName("imapTitle");
    imapTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    headerLayout->addWidget(imapTitle);

    headerLayout->addStretch();

    QPushButton* addImapBtn = new QPushButton(m_imapSection);
    addImapBtn->setObjectName("addImapBtn");
    addImapBtn->setIcon(m_awesome->icon(fa_solid, fa_plus));
    addImapBtn->setText("添加IMAP账户");
    addImapBtn->setIconSize(QSize(16, 16));
    addImapBtn->setStyleSheet("QPushButton { background-color: #4caf50; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; } QPushButton:hover { background-color: #45a049; }");
    connect(addImapBtn, &QPushButton::clicked, this, &EmailCountWindow::onAddImapAccount);
    headerLayout->addWidget(addImapBtn);

    sectionLayout->addWidget(headerWidget);

    m_imapAccountListContainer = new QWidget(m_imapSection);
    m_imapAccountListLayout = new QVBoxLayout(m_imapAccountListContainer);
    m_imapAccountListLayout->setContentsMargins(0, 0, 0, 0);
    m_imapAccountListLayout->setSpacing(8);
    sectionLayout->addWidget(m_imapAccountListContainer);

    m_mainLayout->addWidget(m_imapSection);
}

void EmailCountWindow::setupSmtpSection() {
    m_smtpSection = new QWidget(this);
    QVBoxLayout* sectionLayout = new QVBoxLayout(m_smtpSection);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    sectionLayout->setSpacing(15);

    QWidget* headerWidget = new QWidget(m_smtpSection);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(10);

    QLabel* smtpTitle = new QLabel("SMTP账户", m_smtpSection);
    smtpTitle->setObjectName("smtpTitle");
    smtpTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    headerLayout->addWidget(smtpTitle);

    headerLayout->addStretch();

    QPushButton* addSmtpBtn = new QPushButton(m_smtpSection);
    addSmtpBtn->setObjectName("addSmtpBtn");
    addSmtpBtn->setIcon(m_awesome->icon(fa_solid, fa_plus));
    addSmtpBtn->setText("添加SMTP账户");
    addSmtpBtn->setIconSize(QSize(16, 16));
    addSmtpBtn->setStyleSheet("QPushButton { background-color: #2196f3; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; } QPushButton:hover { background-color: #1e88e5; }");
    connect(addSmtpBtn, &QPushButton::clicked, this, &EmailCountWindow::onAddSmtpAccount);
    headerLayout->addWidget(addSmtpBtn);

    sectionLayout->addWidget(headerWidget);

    m_smtpAccountListContainer = new QWidget(m_smtpSection);
    m_smtpAccountListLayout = new QVBoxLayout(m_smtpAccountListContainer);
    m_smtpAccountListLayout->setContentsMargins(0, 0, 0, 0);
    m_smtpAccountListLayout->setSpacing(8);
    sectionLayout->addWidget(m_smtpAccountListContainer);

    m_mainLayout->addWidget(m_smtpSection);
}

void EmailCountWindow::loadAccounts() {
    updateImapAccountList();
    updateSmtpAccountList();
}

void EmailCountWindow::updateImapAccountList() {
    QList<EmailAccountItem*> items = m_imapAccountListContainer->findChildren<EmailAccountItem*>();
    for (EmailAccountItem* item : items) {
        m_imapAccountListLayout->removeWidget(item);
        delete item;
    }
    m_imapItemMap.clear();

    DatabaseManager dbManager;
    QVector<EmailAccount> accounts = dbManager.getImapAccounts();
    for (const EmailAccount& account : accounts) {
        EmailAccountItem* item = new EmailAccountItem(m_imapAccountListContainer);
        item->setAccount(account);
        m_imapAccountListLayout->addWidget(item);
        m_imapItemMap[account.id] = item;
        connect(item, &EmailAccountItem::editClicked, this, &EmailCountWindow::onEditImapAccount);
        connect(item, &EmailAccountItem::deleteClicked, this, &EmailCountWindow::onDeleteImapAccount);
    }
}

void EmailCountWindow::updateSmtpAccountList() {
    QList<EmailAccountItem*> items = m_smtpAccountListContainer->findChildren<EmailAccountItem*>();
    for (EmailAccountItem* item : items) {
        m_smtpAccountListLayout->removeWidget(item);
        delete item;
    }
    m_smtpItemMap.clear();

    DatabaseManager dbManager;
    QVector<EmailAccount> accounts = dbManager.getSmtpAccounts();
    for (const EmailAccount& account : accounts) {
        EmailAccountItem* item = new EmailAccountItem(m_smtpAccountListContainer);
        item->setAccount(account);
        m_smtpAccountListLayout->addWidget(item);
        m_smtpItemMap[account.id] = item;
        connect(item, &EmailAccountItem::editClicked, this, &EmailCountWindow::onEditSmtpAccount);
        connect(item, &EmailAccountItem::deleteClicked, this, &EmailCountWindow::onDeleteSmtpAccount);
    }
}

void EmailCountWindow::onAddImapAccount() {
    AddImapAccountDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        EmailAccount account = dialog.getAccount();
        DatabaseManager dbManager;
        if (dbManager.saveImapAccount(account)) {
            updateImapAccountList();
            QMessageBox::information(this, "成功", "IMAP账户添加成功！");
        } else {
            QMessageBox::warning(this, "错误", "保存账户失败，请重试。");
        }
    }
}

void EmailCountWindow::onAddSmtpAccount() {
    AddSmtpAccountDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        EmailAccount account = dialog.getAccount();
        DatabaseManager dbManager;
        if (dbManager.saveSmtpAccount(account)) {
            updateSmtpAccountList();
            QMessageBox::information(this, "成功", "SMTP账户添加成功！");
        } else {
            QMessageBox::warning(this, "错误", "保存账户失败，请重试。");
        }
    }
}

void EmailCountWindow::onEditImapAccount(const EmailAccount& account) {
    EditImapAccountDialog dialog(account, this);
    if (dialog.exec() == QDialog::Accepted) {
        EmailAccount updatedAccount = dialog.getAccount();
        DatabaseManager dbManager;
        if (dbManager.saveImapAccount(updatedAccount)) {
            updateImapAccountList();
            QMessageBox::information(this, "成功", "IMAP账户信息已更新！");
        } else {
            QMessageBox::warning(this, "错误", "保存账户失败，请重试。");
        }
    }
}

void EmailCountWindow::onEditSmtpAccount(const EmailAccount& account) {
    EditSmtpAccountDialog dialog(account, this);
    if (dialog.exec() == QDialog::Accepted) {
        EmailAccount updatedAccount = dialog.getAccount();
        DatabaseManager dbManager;
        if (dbManager.saveSmtpAccount(updatedAccount)) {
            updateSmtpAccountList();
            QMessageBox::information(this, "成功", "SMTP账户信息已更新！");
        } else {
            QMessageBox::warning(this, "错误", "保存账户失败，请重试。");
        }
    }
}

void EmailCountWindow::onDeleteImapAccount(const EmailAccount& account) {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除此IMAP账户吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        DatabaseManager dbManager;
        if (dbManager.deleteImapAccount(account.id)) {
            updateImapAccountList();
            QMessageBox::information(this, "删除成功", "IMAP账户已删除");
        } else {
            QMessageBox::warning(this, "删除失败", "删除账户失败，请重试。");
        }
    }
}

void EmailCountWindow::onDeleteSmtpAccount(const EmailAccount& account) {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除此SMTP账户吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        DatabaseManager dbManager;
        if (dbManager.deleteSmtpAccount(account.id)) {
            updateSmtpAccountList();
            QMessageBox::information(this, "删除成功", "SMTP账户已删除");
        } else {
            QMessageBox::warning(this, "删除失败", "删除账户失败，请重试。");
        }
    }
}

void EmailCountWindow::onSaveAccount() {
}
