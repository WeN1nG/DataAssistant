#include "EmailBoxWindow.h"
#include <QMessageBox>
#include <QApplication>
#include <QThread>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include <QProgressBar>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QTimer>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QEventLoop>
#include <algorithm>
#include "../Database/DatabaseManager.h"

using namespace fa;

EmailBoxWindow::EmailBoxWindow(QWidget* parent)
    : QWidget(parent)
    , m_emailDataManager(new EmailDataManager())
    , m_pythonEmailClient(nullptr)
    , m_imapThread(nullptr)
    , m_awesome(new QtAwesome(qApp))
    , m_progressDialog(nullptr)
    , m_fetchStep(FetchStep::Idle)
    , m_isInitialized(false)
{
    m_awesome->initFontAwesome();

    if (!m_emailDataManager->initializeDatabase()) {
        QMessageBox::critical(this, "错误", "无法初始化邮箱数据库");
        return;
    }

    m_pythonEmailClient = new PythonEmailClient(this);

    connect(m_pythonEmailClient, &PythonEmailClient::connected, this, &EmailBoxWindow::onImapConnected);
    connect(m_pythonEmailClient, &PythonEmailClient::authenticated, this, &EmailBoxWindow::onImapAuthenticated);
    connect(m_pythonEmailClient, &PythonEmailClient::errorOccurred, this, &EmailBoxWindow::onImapError);
    connect(m_pythonEmailClient, &PythonEmailClient::mailboxSelected, this, &EmailBoxWindow::onMailboxSelected);
    connect(m_pythonEmailClient, &PythonEmailClient::progressUpdated, this, &EmailBoxWindow::onFetchProgress);
    connect(m_pythonEmailClient, &PythonEmailClient::disconnected, this, [this]() {
        qDebug() << "[EmailBoxWindow] Python email client disconnected";
    });

    setupUI();
    applyModernStyle();

    DatabaseManager dbManager;
    m_accounts = dbManager.getImapAccounts();
    
    qDebug() << "=== [DEBUG] Database Account Loading ===";
    qDebug() << "Total accounts loaded:" << m_accounts.size();
    
    for (int i = 0; i < m_accounts.size(); ++i) {
        const EmailAccount& acc = m_accounts[i];
        qDebug() << "Account" << i << ":";
        qDebug() << "  ID:" << acc.id;
        qDebug() << "  Email:" << acc.emailAddress;
        qDebug() << "  Username:" << acc.username;
        qDebug() << "  Password length:" << acc.password.length();
        qDebug() << "  IMAP Server:" << acc.imapServer << "Port:" << acc.imapPort;
        qDebug() << "  Is Default:" << acc.isDefault;
    }
    qDebug() << "=== [DEBUG] End Account Loading ===";
    
    updateAccountSelector();

    if (!m_accounts.isEmpty()) {
        m_currentAccount = m_accounts.first();
        qDebug() << "[DEBUG] Selected account:" << m_currentAccount.emailAddress;
    } else {
        qDebug() << "[DEBUG] WARNING: No accounts loaded from database!";
    }

    m_isInitialized = true;
}

void EmailBoxWindow::setupImapThread() {
    if (m_imapThread) {
        return;
    }

    m_imapThread = new QThread(this);

    connect(m_imapThread, &QThread::finished, this, [this]() {
        if (m_pythonEmailClient) {
            m_pythonEmailClient->disconnect();
        }
    });

    m_imapThread->start();
}

void EmailBoxWindow::cleanupImapThread() {
    if (m_imapThread) {
        if (m_pythonEmailClient) {
            m_pythonEmailClient->disconnect();
        }
        m_imapThread->quit();
        if (!m_imapThread->wait(3000)) {
            m_imapThread->terminate();
            m_imapThread->wait();
        }
    }
}

void EmailBoxWindow::closeEvent(QCloseEvent* event) {
    cleanupImapThread();
    QWidget::closeEvent(event);
}

void EmailBoxWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    
    static bool isShowing = false;
    if (isShowing) {
        return;
    }
    
    if (m_isInitialized && !m_emailItems.isEmpty()) {
        return;
    }
    
    isShowing = true;
    
    qDebug() << "[EmailBoxWindow] showEvent: Loading local cached emails";
    loadLocalCachedEmails();
    
    isShowing = false;
}

EmailBoxWindow::~EmailBoxWindow() {
    cleanupImapThread();
    qDeleteAll(m_emailItems);
}

void EmailBoxWindow::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    setupToolBar();
    setupEmailListArea();
}

void EmailBoxWindow::setupToolBar() {
    m_toolBar = new QWidget(this);
    m_toolBar->setObjectName("toolBar");
    m_toolBar->setMinimumHeight(60);

    QHBoxLayout* toolbarLayout = new QHBoxLayout(m_toolBar);
    toolbarLayout->setContentsMargins(15, 10, 15, 10);
    toolbarLayout->setSpacing(15);

    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setObjectName("refreshBtn");
    m_refreshBtn->setIcon(m_awesome->icon(fa_solid, fa_rotate));
    m_refreshBtn->setStyleSheet("QPushButton { background-color: #1976d2; color: white; border: none; border-radius: 6px; padding: 10px 20px; font-weight: bold; } QPushButton:hover { background-color: #1565c0; }");
    connect(m_refreshBtn, &QPushButton::clicked, this, &EmailBoxWindow::onRefreshEmails);
    toolbarLayout->addWidget(m_refreshBtn);

    toolbarLayout->addStretch();

    QLabel* accountLabel = new QLabel("当前账户:", this);
    accountLabel->setStyleSheet("font-weight: bold; color: #666; font-size: 13px;");
    toolbarLayout->addWidget(accountLabel);

    m_accountSelector = new QComboBox(this);
    m_accountSelector->setObjectName("accountSelector");
    m_accountSelector->setMinimumWidth(200);
    m_accountSelector->setMinimumHeight(36);
    m_accountSelector->setStyleSheet("QComboBox { border: 1px solid #d0d0d0; border-radius: 6px; padding: 8px 12px; background-color: white; } QComboBox:hover { border-color: #1976d2; }");
    connect(m_accountSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EmailBoxWindow::onAccountSelected);
    toolbarLayout->addWidget(m_accountSelector);

    m_mainLayout->addWidget(m_toolBar);
}

void EmailBoxWindow::setupEmailListArea() {
    m_emailScrollArea = new QScrollArea(this);
    m_emailScrollArea->setObjectName("emailScrollArea");
    m_emailScrollArea->setWidgetResizable(true);
    m_emailScrollArea->setFrameShape(QFrame::NoFrame);
    m_emailScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_emailScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_emailListContainer = new QWidget(this);
    m_emailListContainer->setObjectName("emailListContainer");
    m_emailListContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_emailListLayout = new QVBoxLayout(m_emailListContainer);
    m_emailListLayout->setContentsMargins(20, 15, 20, 15);
    m_emailListLayout->setSpacing(12);
    m_emailListLayout->addStretch();

    m_emailScrollArea->setWidget(m_emailListContainer);
    m_emailScrollArea->setWidgetResizable(true);
    m_mainLayout->addWidget(m_emailScrollArea, 1);
}

void EmailBoxWindow::loadEmails() {
    qDebug() << "[DEBUG] loadEmails: Starting...";

    for (EmailListItemNew* item : m_emailItems) {
        m_emailListLayout->removeWidget(item);
        delete item;
    }
    m_emailItems.clear();
    m_emailItemMap.clear();

    qDebug() << "[DEBUG] loadEmails: accountId=" << m_currentAccount.id;

    QVector<Email> emails;

    if (m_currentAccount.id == 0) {
        qDebug() << "[DEBUG] loadEmails: accountId is 0, loading all accounts";
        for (const EmailAccount& account : m_accounts) {
            QVector<Email> accountEmails = m_emailDataManager->getEmailsByFolder(account.id, EmailFolder::Inbox);
            emails.append(accountEmails);
        }
        qDebug() << "[DEBUG] loadEmails: found" << emails.size() << "total emails from all accounts";
    } else {
        emails = m_emailDataManager->getEmailsByFolder(m_currentAccount.id, EmailFolder::Inbox);
        qDebug() << "[DEBUG] loadEmails: found" << emails.size() << "emails from database";
    }

    const int MAX_DISPLAY_EMAILS = 12;
    int displayCount = qMin(emails.size(), MAX_DISPLAY_EMAILS);
    qDebug() << "[DEBUG] loadEmails: displaying" << displayCount << "emails (max limit:" << MAX_DISPLAY_EMAILS << ")";

    const int BATCH_SIZE = 20;
    for (int i = 0; i < displayCount; ++i) {
        const Email& email = emails[i];

        try {
            EmailListItemNew* item = new EmailListItemNew(this);
            item->setEmail(email);

            connect(item, &EmailListItemNew::viewClicked, this, &EmailBoxWindow::onEmailViewClicked);
            connect(item, &EmailListItemNew::deleteClicked, this, &EmailBoxWindow::onEmailDeleteClicked);

            m_emailItems.append(item);
            m_emailItemMap[email.id] = item;

            m_emailListLayout->insertWidget(m_emailListLayout->count() - 1, item);

            qDebug() << "[DEBUG] loadEmails: Created item" << (i + 1) << "/" << emails.size();

            // 每创建BATCH_SIZE个widget后，处理一下事件循环，让UI有机会更新
            if ((i + 1) % BATCH_SIZE == 0) {
                qApp->processEvents();
            }
        } catch (const std::exception& e) {
            qDebug() << "[ERROR] loadEmails: Exception creating item" << (i + 1) << ":" << e.what();
        } catch (...) {
            qDebug() << "[ERROR] loadEmails: Unknown exception creating item" << (i + 1);
        }
    }

    qDebug() << "[DEBUG] loadEmails: added" << m_emailItems.size() << "items to UI";
}

void EmailBoxWindow::updateEmailList() {
    loadEmails();
}

void EmailBoxWindow::onRefreshEmails() {
    qDebug() << "[EmailBoxWindow] onRefreshEmails: currentAccount.id =" << m_currentAccount.id;
    
    // 步骤1：检查是否有账户
    if (m_accounts.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先添加邮箱账户");
        return;
    }
    
    // 步骤2：检查是否正在刷新（防止重复触发）
    if (m_fetchStep != FetchStep::Idle) {
        QMessageBox::information(this, "提示", "正在刷新中，请稍候...");
        return;
    }
    
    // 步骤3：根据当前选中的账户执行刷新
    if (m_currentAccount.id == 0) {
        // 全部账户刷新
        qDebug() << "[EmailBoxWindow] onRefreshEmails: Refreshing all accounts";
        refreshAllAccounts();
    } else {
        // 单账户刷新
        qDebug() << "[EmailBoxWindow] onRefreshEmails: Refreshing single account:" << m_currentAccount.emailAddress;
        refreshSingleAccount(m_currentAccount);
    }
}

void EmailBoxWindow::loadLocalCachedEmails() {
    qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: START - Thread ID:" << QThread::currentThreadId();
    qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: currentAccount.id =" << m_currentAccount.id;
    
    // 防御性检查：验证对象是否仍然有效
    if (!this->m_emailDataManager) {
        qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: ERROR - m_emailDataManager is nullptr!";
        return;
    }
    
    // 如果当前账户为空（全部账户），尝试加载AllEmailBox
    if (this->m_currentAccount.id == 0) {
        qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Loading from AllEmailBox";
        
        QVector<Email> emails = this->m_emailDataManager->loadEmailsFromAllEmailBox();
        qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Loaded" << emails.size() << "emails from AllEmailBox";
        
        // 如果AllEmailBox为空（未刷新过），从各账户EmailBox合并
        if (emails.isEmpty()) {
            qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: AllEmailBox is empty, merging from account boxes";
            QVector<Email> allEmails;
            
            for (const EmailAccount& account : this->m_accounts) {
                qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Loading from account:" << account.emailAddress;
                QVector<Email> accountEmails = this->m_emailDataManager->loadEmailsFromEmailBox(account.emailAddress);
                qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Loaded" << accountEmails.size() << "emails from" << account.emailAddress;
                allEmails.append(accountEmails);
            }
            
            qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Total emails before sort:" << allEmails.size();
            
            // 按发送时间降序排序（新的在前）
            std::sort(allEmails.begin(), allEmails.end(), [](const Email& a, const Email& b) {
                return a.sentAt > b.sentAt;
            });
            
            qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Displaying" << allEmails.size() << "emails";
            this->displayEmails(allEmails);
            qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Display complete (empty fallback)";
        } else {
            qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Displaying" << emails.size() << "emails";
            this->displayEmails(emails);
            qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Display complete (from AllEmailBox)";
        }
    } else {
        // 加载指定账户的EmailBox
        qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Loading from account:" << this->m_currentAccount.emailAddress;
        QVector<Email> emails = this->m_emailDataManager->loadEmailsFromEmailBox(this->m_currentAccount.emailAddress);
        qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Loaded" << emails.size() << "emails, now displaying...";
        this->displayEmails(emails);
        qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: Display complete (from account)";
    }
    
    qDebug() << "[EmailBoxWindow] loadLocalCachedEmails: END";
}

void EmailBoxWindow::refreshSingleAccount(const EmailAccount& account) {
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: START - account:" << account.emailAddress;
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: IMAP Server:" << account.imapServer << "Port:" << account.imapPort;
    
    // 步骤1：更新当前账户信息
    m_currentAccount = account;
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 1 - Updated m_currentAccount";
    
    // 步骤2：准备数据库（清空并创建EmailBox）
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 2 - Preparing database...";
    if (!m_emailDataManager->prepareEmailBoxForRefresh(account.emailAddress)) {
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 2 - FAILED";
        QMessageBox::warning(this, "错误", "无法准备邮箱数据库");
        return;
    }
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 2 - SUCCESS";
    
    // 步骤3：设置UI状态
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 3 - Updating UI state...";
    m_refreshBtn->setEnabled(false);
    m_refreshBtn->setText("连接中...");
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 3 - UI state updated";
    
    // 步骤4：显示进度对话框
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 4 - Creating progress dialog...";
    m_progressDialog = new QProgressDialog("正在连接邮箱服务器...", "取消", 0, 100, this);
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->setValue(10);
    m_progressDialog->show();
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 4 - Progress dialog created";
    
    // 步骤5：设置线程
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 5 - Setting up IMAP thread...";
    setupImapThread();
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 5 - IMAP thread setup complete";
    
    // 步骤6：延迟执行连接（避免阻塞UI）
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: Step 6 - Starting connection in 100ms...";
    QTimer::singleShot(100, this, [this, account]() {
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda START - Thread ID:" << QThread::currentThreadId();
        
        // 防御性检查：验证对象是否仍然有效
        if (!this->m_pythonEmailClient) {
            qDebug() << "[EmailBoxWindow] refreshSingleAccount: ERROR - m_pythonEmailClient is nullptr!";
            this->cleanupProgressDialog();
            return;
        }
        
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Connecting to server...";
        bool connected = this->m_pythonEmailClient->connectToServer(
            account.imapServer,
            account.imapPort,
            account.emailAddress,
            account.password,
            true
        );
        
        if (!connected) {
            qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Connection FAILED";
            this->cleanupProgressDialog();

            // 重新连接邮件选择信号槽
            connect(m_pythonEmailClient, &PythonEmailClient::mailboxSelected, this, &EmailBoxWindow::onMailboxSelected);

            qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Showing error message...";
            QMessageBox::warning(this, "连接失败",
                QString("无法连接到邮箱服务器：%1\n\n请检查：\n"
                        "1. 网络连接是否正常\n"
                        "2. IMAP服务器地址是否正确\n"
                        "3. IMAP端口是否正确(通常为993)").arg(account.emailAddress));
            qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Loading local cached emails...";
            this->loadLocalCachedEmails();
            qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Connection failed handler COMPLETE";
            return;
        }
        
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Connection SUCCESS";
        
        // 步骤7：选择邮箱
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 7 - Selecting mailbox...";
        if (this->m_progressDialog) {
            this->m_progressDialog->setValue(40);
            this->m_progressDialog->setLabelText("正在打开收件箱...");
            qApp->processEvents();
        }
        
        if (!this->m_pythonEmailClient->selectMailbox("INBOX")) {
            qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 7 - Select mailbox FAILED";
            this->cleanupProgressDialog();
            this->m_pythonEmailClient->disconnect();

            // 重新连接邮件选择信号槽
            connect(m_pythonEmailClient, &PythonEmailClient::mailboxSelected, this, &EmailBoxWindow::onMailboxSelected);

            QMessageBox::warning(this, "错误",
                QString("无法打开收件箱：%1").arg(account.emailAddress));
            this->loadLocalCachedEmails();
            return;
        }
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 7 - Select mailbox SUCCESS";

        // 断开邮件选择信号槽，避免重复获取邮件
        disconnect(m_pythonEmailClient, &PythonEmailClient::mailboxSelected, this, &EmailBoxWindow::onMailboxSelected);

        // 步骤8：获取邮件列表（获取更多邮件）
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 8 - Fetching email list...";
        if (this->m_progressDialog) {
            this->m_progressDialog->setValue(60);
            this->m_progressDialog->setLabelText("正在获取邮件列表...");
            qApp->processEvents();
        }
        
        // 获取更多邮件（500封），确保第一次刷新保存更多邮件
        QString jsonResult = this->m_pythonEmailClient->fetchEmailListAsJsonFast(1, 500);
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 8 - Fetched" << jsonResult.length() << "bytes";
        
        // 步骤9：保存到数据库
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 9 - Saving to database...";
        if (this->m_progressDialog) {
            this->m_progressDialog->setValue(80);
            this->m_progressDialog->setLabelText("正在保存邮件...");
            qApp->processEvents();
        }
        
        this->saveFetchedEmailsFromJson(jsonResult);
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 9 - Save complete";
        
        // 步骤10：延迟断开连接，避免影响查看邮件功能
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 10 - Scheduling disconnect in 5 seconds...";
        QTimer::singleShot(5000, this, [this]() {
            qDebug() << "[EmailBoxWindow] refreshSingleAccount: Delayed disconnect triggered";
            this->m_pythonEmailClient->disconnect();
        });
        
        // 步骤11：更新UI
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 11 - Updating UI...";
        if (this->m_progressDialog) {
            this->m_progressDialog->setValue(95);
            this->m_progressDialog->setLabelText("正在刷新界面...");
            qApp->processEvents();
        }
        
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 11 - Loading emails from account...";
        this->loadEmailsFromAccountEmailBox();
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 11 - UI update complete";
        
        // 步骤12：清理
        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda - Step 12 - Cleaning up...";
        this->cleanupProgressDialog();

        // 重新连接邮件选择信号槽
        connect(m_pythonEmailClient, &PythonEmailClient::mailboxSelected, this, &EmailBoxWindow::onMailboxSelected);

        qDebug() << "[EmailBoxWindow] refreshSingleAccount: Lambda COMPLETE - Thread ID:" << QThread::currentThreadId();
    });
    
    qDebug() << "[EmailBoxWindow] refreshSingleAccount: END - Timer set, waiting for lambda execution";
}

void EmailBoxWindow::refreshAllAccounts() {
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: START - accounts count:" << m_accounts.size();
    
    // 防御性检查：验证对象是否仍然有效
    if (!this->m_emailDataManager) {
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: ERROR - m_emailDataManager is nullptr!";
        return;
    }
    
    if (!this->m_pythonEmailClient) {
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: ERROR - m_pythonEmailClient is nullptr!";
        return;
    }
    
    // 步骤1：设置UI状态
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 1 - Updating UI state...";
    this->m_refreshBtn->setEnabled(false);
    this->m_refreshBtn->setText("加载中...");
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 1 - UI state updated";
    
    // 步骤2：显示进度对话框
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 2 - Creating progress dialog...";
    this->m_progressDialog = new QProgressDialog("正在刷新所有邮箱...", "取消", 0, 100, this);
    this->m_progressDialog->setWindowModality(Qt::WindowModal);
    this->m_progressDialog->setValue(5);
    this->m_progressDialog->show();
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 2 - Progress dialog created";
    
    // 步骤3：遍历所有账户，分别刷新
    int totalAccounts = this->m_accounts.size();
    int currentAccountIndex = 0;
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3 - Starting account loop, total:" << totalAccounts;
    
    for (const EmailAccount& account : this->m_accounts) {
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Loop iteration - account:" << account.emailAddress;
        
        // 检查是否取消
        if (this->m_progressDialog && this->m_progressDialog->wasCanceled()) {
            qDebug() << "[EmailBoxWindow] refreshAllAccounts: User cancelled at account" << currentAccountIndex;
            break;
        }
        
        currentAccountIndex++;
        
        // 计算进度：每个账户占 80/totalAccounts，合并占 20
        int progress = 5 + (currentAccountIndex * 80) / totalAccounts;
        if (this->m_progressDialog) {
            this->m_progressDialog->setValue(progress);
            this->m_progressDialog->setLabelText(QString("正在刷新 %1/%2: %3")
                .arg(currentAccountIndex)
                .arg(totalAccounts)
                .arg(account.emailAddress));
            qApp->processEvents();
        }
        
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Processing account:" << account.emailAddress;
        
        // 步骤3a：准备该账户的数据库
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3a - Preparing database...";
        this->m_emailDataManager->prepareEmailBoxForRefresh(account.emailAddress);
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3a - Database prepared";
        
        // 步骤3b：断开之前的连接
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3b - Checking connection...";
        if (this->m_pythonEmailClient->isConnected()) {
            qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3b - Disconnecting...";
            this->m_pythonEmailClient->disconnect();
        }
        
        // 步骤3c：连接到服务器
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3c - Connecting to server...";
        bool connected = this->m_pythonEmailClient->connectToServer(
            account.imapServer,
            account.imapPort,
            account.emailAddress,
            account.password,
            true
        );
        
        if (!connected) {
            qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3c - Connection FAILED for:" << account.emailAddress;
            continue;
        }
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3c - Connection SUCCESS";
        
        // 步骤3d：选择邮箱
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3d - Selecting mailbox...";
        if (!this->m_pythonEmailClient->selectMailbox("INBOX")) {
            qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3d - Select mailbox FAILED";
            this->m_pythonEmailClient->disconnect();
            continue;
        }
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3d - Select mailbox SUCCESS";

        // 断开邮件选择信号槽，避免重复获取邮件
        disconnect(m_pythonEmailClient, &PythonEmailClient::mailboxSelected, this, &EmailBoxWindow::onMailboxSelected);

        // 步骤3e：获取邮件列表
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3e - Fetching email list...";
        QString jsonResult = this->m_pythonEmailClient->fetchEmailListAsJsonFast(1, 500);
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3e - Fetched" << jsonResult.length() << "bytes";
        
        if (!jsonResult.isEmpty() && jsonResult != "{}" && jsonResult != "[]") {
            this->m_currentAccount = account;
            qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3e - Saving emails to database...";
            this->saveFetchedEmailsFromJson(jsonResult);
            qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3e - Emails saved";
        } else {
            qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3e - No emails to save (empty result)";
        }
        
        // 步骤3f：断开连接
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3f - Disconnecting...";
        this->m_pythonEmailClient->disconnect();
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 3f - Disconnected";
        
        // 处理事件，避免UI冻结
        qApp->processEvents();
    }
    
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 4 - Merging all email boxes...";
    
    // 步骤4：合并所有账户的邮件到AllEmailBox
    if (this->m_progressDialog) {
        this->m_progressDialog->setValue(90);
        this->m_progressDialog->setLabelText("正在合并邮件...");
        qApp->processEvents();
    }
    
    bool mergeSuccess = this->m_emailDataManager->mergeAllEmailBoxesToAllEmailBox(this->m_accounts);
    
    if (!mergeSuccess) {
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 4 - Merge FAILED, using fallback";
    } else {
        qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 4 - Merge SUCCESS";
    }
    
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 5 - Updating UI...";
    
    // 步骤5：更新UI
    if (this->m_progressDialog) {
        this->m_progressDialog->setValue(98);
        this->m_progressDialog->setLabelText("正在刷新界面...");
        qApp->processEvents();
    }
    
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 5 - Loading emails from AllEmailBox...";
    this->loadEmailsFromAllEmailBox();
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 5 - UI update complete";
    
    // 步骤6：清理
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: Step 6 - Cleaning up...";
    if (this->m_progressDialog) {
        this->m_progressDialog->setValue(100);
    }
    this->cleanupProgressDialog();

    // 重新连接邮件选择信号槽
    connect(m_pythonEmailClient, &PythonEmailClient::mailboxSelected, this, &EmailBoxWindow::onMailboxSelected);

    this->m_refreshBtn->setEnabled(true);
    this->m_refreshBtn->setText("刷新");
    
    qDebug() << "[EmailBoxWindow] refreshAllAccounts: COMPLETE";
}

// 废弃函数，已被 refreshAllAccounts 替代
// void EmailBoxWindow::fetchEmailsForAllAccounts() {
//     if (m_accounts.isEmpty()) {
//         return;
//     }
// 
//     m_refreshBtn->setEnabled(false);
//     m_refreshBtn->setText("加载中...");
// 
//     m_progressDialog = new QProgressDialog("正在加载所有邮箱...", "取消", 0, 100, this);
//     m_progressDialog->setWindowModality(Qt::WindowModal);
//     m_progressDialog->setValue(10);
//     m_progressDialog->show();
// 
//     int totalAccounts = m_accounts.size();
//     int currentAccount = 0;
// 
//     for (const EmailAccount& account : m_accounts) {
//         if (m_progressDialog->wasCanceled()) {
//             break;
//         }
// 
//         currentAccount++;
//         int progress = (currentAccount * 100) / totalAccounts;
//         m_progressDialog->setValue(progress);
//         m_progressDialog->setLabelText(QString("正在加载账户 %1/%2: %3").arg(currentAccount).arg(totalAccounts).arg(account.emailAddress));
// 
//         m_currentAccount = account;
//         qDebug() << "[DEBUG] Fetching emails for account:" << account.emailAddress;
// 
//         if (m_pythonEmailClient->isConnected()) {
//             m_pythonEmailClient->disconnect();
//         }
// 
//         bool connected = m_pythonEmailClient->connectToServer(
//             account.imapServer,
//             account.imapPort,
//             account.emailAddress,
//             account.password,
//             true
//         );
// 
//         if (!connected) {
//             qDebug() << "[ERROR] Failed to connect to account:" << account.emailAddress;
//             continue;
//         }
// 
//         if (!m_pythonEmailClient->selectMailbox("INBOX")) {
//             qDebug() << "[ERROR] Failed to select mailbox for account:" << account.emailAddress;
//             m_pythonEmailClient->disconnect();
//             continue;
//         }
// 
//         QString jsonResult = m_pythonEmailClient->fetchEmailListAsJsonFast(1, 100);
// 
//         if (!jsonResult.isEmpty() && jsonResult != "{}" && jsonResult != "[]") {
//             saveFetchedEmailsFromJson(jsonResult);
//         }
// 
//         m_pythonEmailClient->disconnect();
// 
//         qApp->processEvents();
//     }
// 
//     m_emailDataManager->mergeEmailBoxesToAll(m_accounts);
// 
//     cleanupProgressDialog();
// 
//     loadEmailsFromAllEmailBox();
// 
//     m_refreshBtn->setEnabled(true);
//     m_refreshBtn->setText("刷新");
// }

// 废弃函数，已被 refreshSingleAccount 替代
// void EmailBoxWindow::fetchEmailsFromServer() {
//     if (m_currentAccount.id == 0) {
//         return;
//     }
// 
//     qDebug() << "=== [DEBUG] Email Account Information ===";
//     qDebug() << "Email:" << m_currentAccount.emailAddress;
//     qDebug() << "Display Name:" << m_currentAccount.displayName;
//     qDebug() << "IMAP Server:" << m_currentAccount.imapServer << "Port:" << m_currentAccount.imapPort;
//     qDebug() << "SMTP Server:" << m_currentAccount.smtpServer << "Port:" << m_currentAccount.smtpPort;
//     qDebug() << "Username:" << m_currentAccount.username;
//     qDebug() << "Password Length:" << m_currentAccount.password.length() << "chars";
//     qDebug() << "Account ID:" << m_currentAccount.id;
//     qDebug() << "Is Default:" << m_currentAccount.isDefault;
//     qDebug() << "=== [DEBUG] End Account Info ===";
// 
//     if (m_currentAccount.imapServer.isEmpty()) {
//         qDebug() << "[ERROR] IMAP Server is empty! Cannot connect.";
//         QMessageBox::warning(this, "配置错误",
//             "IMAP服务器地址为空！\n\n请重新编辑邮箱账户，填写正确的IMAP服务器地址。\n\nQQ邮箱的IMAP服务器地址是：imap.qq.com");
//         return;
//     }
// 
//     m_refreshBtn->setEnabled(false);
//     m_refreshBtn->setText("连接中...");
// 
//     m_progressDialog = new QProgressDialog("正在连接邮箱服务器...", "取消", 0, 100, this);
//     m_progressDialog->setWindowModality(Qt::WindowModal);
//     m_progressDialog->setValue(10);
//     m_progressDialog->show();
// 
//     m_fetchStep = FetchStep::Connecting;
// 
//     qDebug() << "[UI] Starting connection to IMAP server using Python";
// 
//     setupImapThread();
// 
//     QTimer::singleShot(100, this, [this]() {
//         qDebug() << "[DEBUG] Calling connectToServer with:";
//         qDebug() << "  Host:" << m_currentAccount.imapServer;
//         qDebug() << "  Port:" << m_currentAccount.imapPort;
//         qDebug() << "  Username (emailAddress):" << m_currentAccount.emailAddress;
//         qDebug() << "  Password:" << (m_currentAccount.password.isEmpty() ? "(empty)" : QString("(length: %1)").arg(m_currentAccount.password.length()));
//         
//         bool success = m_pythonEmailClient->connectToServer(
//             m_currentAccount.imapServer,
//             m_currentAccount.imapPort,
//             m_currentAccount.emailAddress,
//             m_currentAccount.password,
//             true
//         );
//         
//         if (!success) {
//             qDebug() << "[UI] Connection failed immediately";
//             cleanupProgressDialog();
//             QMessageBox::warning(this, "连接失败",
//                 "无法连接到邮箱服务器\n\n请检查:\n1. 网络连接是否正常\n2. IMAP服务器地址是否正确\n3. IMAP端口是否正确(通常为993)");
//             loadEmails();
//         }
//     });
// }

// 废弃函数，已被 refreshSingleAccount 替代
// void EmailBoxWindow::performConnection() {
//     qDebug() << "[DEBUG] performConnection called with:";
//     qDebug() << "  Username:" << m_currentAccount.emailAddress;
//     qDebug() << "  Password:" << (m_currentAccount.password.isEmpty() ? "(empty)" : QString("(length: %1)").arg(m_currentAccount.password.length()));
//     m_pythonEmailClient->login(m_currentAccount.emailAddress, m_currentAccount.password);
// }

void EmailBoxWindow::onConnectionResult(bool success) {
    if (!success) {
        qDebug() << "[UI] Connection failed";
        cleanupProgressDialog();
        QMessageBox::warning(this, "连接失败",
            "无法连接到邮箱服务器\n\n请检查:\n1. 网络连接是否正常\n2. IMAP服务器地址是否正确\n3. IMAP端口是否正确(通常为993)");
        loadEmails();
        return;
    }

    qDebug() << "[UI] Connection successful, proceeding to authentication";
    m_fetchStep = FetchStep::Authenticating;

    if (m_progressDialog) {
        m_progressDialog->setLabelText("正在登录邮箱服务器...");
        m_progressDialog->setValue(30);
    }

    // performConnection已废弃，不再调用
    // QTimer::singleShot(100, this, [this]() {
    //     performConnection();
    // });
}

void EmailBoxWindow::onImapConnected() {
    qDebug() << "[EmailBoxWindow] onImapConnected called!";
    if (m_progressDialog) {
        m_progressDialog->setLabelText("正在登录邮箱服务器...");
        m_progressDialog->setValue(30);
    }

    m_fetchStep = FetchStep::Authenticating;
    
    // performConnection已废弃，不再调用
    // QTimer::singleShot(100, this, [this]() {
    //     performConnection();
    // });
}

void EmailBoxWindow::onImapAuthenticated() {
    if (m_progressDialog) {
        m_progressDialog->setLabelText("正在打开收件箱...");
        m_progressDialog->setValue(50);
    }

    m_fetchStep = FetchStep::SelectingMailbox;

    QTimer::singleShot(100, this, [this]() {
        m_pythonEmailClient->selectMailbox("INBOX");
    });
}

void EmailBoxWindow::onMailboxSelected(const QString& mailboxName) {
    Q_UNUSED(mailboxName);

    if (m_progressDialog) {
        m_progressDialog->setLabelText("正在获取邮件列表...");
        m_progressDialog->setValue(60);
    }

    m_fetchStep = FetchStep::FetchingEmails;

    QTimer::singleShot(100, this, [this]() {
        QString jsonResult = m_pythonEmailClient->fetchEmailListAsJsonFast(1, 500);
        onEmailListFetchedAsJson(jsonResult);
    });
}

void EmailBoxWindow::onFetchProgress(const QString& operation, int current, int total) {
    Q_UNUSED(operation);
    Q_UNUSED(current);
    Q_UNUSED(total);

    if (m_progressDialog) {
        m_progressDialog->setValue(m_progressDialog->value() + 5);
    }
}

void EmailBoxWindow::onEmailListFetchedAsJson(const QString& jsonResult) {
    qDebug() << "[UI] Received JSON from server, length:" << jsonResult.length();

    if (m_progressDialog) {
        m_progressDialog->setLabelText("正在保存邮件...");
        m_progressDialog->setValue(80);
    }

    saveFetchedEmailsFromJson(jsonResult);

    if (m_progressDialog) {
        m_progressDialog->setLabelText("正在刷新界面...");
        m_progressDialog->setValue(95);
    }

    loadEmailsFromEmailBox();

    cleanupProgressDialog();

    qDebug() << "[UI] Email fetch completed";
}

void EmailBoxWindow::onImapError(PythonEmailClient::EmailError error, const QString& message) {
    qDebug() << "[UI] IMAP Error:" << message;

    cleanupProgressDialog();

    QString errorType;
    switch (error) {
        case PythonEmailClient::EmailError::ConnectionFailed:
            errorType = "连接失败";
            break;
        case PythonEmailClient::EmailError::AuthenticationFailed:
            errorType = "认证失败";
            break;
        case PythonEmailClient::EmailError::NetworkError:
            errorType = "网络错误";
            break;
        default:
            errorType = "未知错误";
            break;
    }

    QMessageBox::critical(this, errorType, 
        QString("%1\n\n%2").arg(errorType).arg(message));

    loadEmails();
}

void EmailBoxWindow::saveFetchedEmails(const QVector<PythonEmailClient::EmailInfo>& emailList) {
    for (const PythonEmailClient::EmailInfo& info : emailList) {
        Email email;
        email.accountId = m_currentAccount.id;
        email.messageId = QString::number(info.messageNumber);
        email.subject = info.subject;
        email.fromAddress = info.from;
        email.fromName = "";
        email.toAddress = info.to;
        if (!info.cc.isEmpty()) {
            email.ccAddresses.append(info.cc);
        }
        email.sentAt = info.date;
        email.receivedAt = info.date;
        email.body = info.body;
        email.htmlBody = info.htmlBody;
        email.isRead = info.isRead;
        email.isStarred = false;
        email.folder = EmailFolder::Inbox;
        email.createdAt = QDateTime::currentDateTime();
        email.modifiedAt = QDateTime::currentDateTime();

        for (const QString& attachmentFilename : info.attachments) {
            EmailAttachment att;
            att.filename = attachmentFilename;
            att.size = 0;
            email.attachments.append(att);
        }

        if (!m_emailDataManager->addEmail(email)) {
            qDebug() << "[ERROR] Failed to add email:" << email.subject;
        }
    }
}

void EmailBoxWindow::saveFetchedEmailsFromJson(const QString& jsonResult) {
    if (jsonResult.isEmpty() || jsonResult == "{}") {
        qDebug() << "[ERROR] saveFetchedEmailsFromJson: Empty or invalid JSON result";
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonResult.toUtf8());
    if (doc.isNull() || !doc.isArray()) {
        qDebug() << "[ERROR] saveFetchedEmailsFromJson: Invalid JSON document";
        return;
    }
    
    QJsonArray emailArray = doc.array();
    qDebug() << "[DEBUG] Parsed" << emailArray.size() << "emails from JSON";
    
    for (int i = 0; i < emailArray.size(); ++i) {
        QJsonObject emailObj = emailArray[i].toObject();
        
        Email email;
        email.accountId = m_currentAccount.id;
        email.messageId = emailObj.value("id").toString();
        email.subject = emailObj.value("subject").toString();
        email.fromAddress = emailObj.value("from").toString();
        email.fromName = "";
        email.toAddress = emailObj.value("to").toString();
        
        QString cc = emailObj.value("cc").toString();
        if (!cc.isEmpty()) {
            email.ccAddresses.append(cc);
        }
        
        QString dateStr = emailObj.value("date").toString();
        QDateTime emailDate = QDateTime::fromString(dateStr, Qt::ISODate);
        if (!emailDate.isValid()) {
            emailDate = QDateTime::currentDateTime();
        }
        
        email.sentAt = emailDate;
        email.receivedAt = emailDate;
        email.body = emailObj.value("body").toString();
        email.htmlBody = emailObj.value("html_body").toString();
        email.isRead = false;
        email.isStarred = false;
        email.folder = EmailFolder::Inbox;
        email.createdAt = QDateTime::currentDateTime();
        email.modifiedAt = QDateTime::currentDateTime();
        
        QJsonArray attachments = emailObj.value("attachments").toArray();
        for (const QJsonValue& att : attachments) {
            QJsonObject attObj = att.toObject();
            EmailAttachment emailAtt;
            emailAtt.filename = attObj.value("filename").toString();
            emailAtt.contentType = attObj.value("content_type").toString();
            emailAtt.size = attObj.value("size").toInt();
            email.attachments.append(emailAtt);
        }
        
        email.rawJson = QJsonDocument(emailObj).toJson(QJsonDocument::Compact);

        if (!m_emailDataManager->saveEmailToEmailBox(email, m_currentAccount.emailAddress)) {
            qDebug() << "[ERROR] Failed to save email to email box:" << email.subject;
        } else {
            qDebug() << "[DEBUG] Successfully saved email to email box:" << email.subject;
        }
    }
}

void EmailBoxWindow::loadEmailsFromEmailBox() {
    if (m_currentAccount.id == 0) {
        qDebug() << "[DEBUG] loadEmailsFromEmailBox: current account is null, loading all accounts";
        m_emailDataManager->mergeEmailBoxesToAll(m_accounts);
        loadEmailsFromAllEmailBox();
    } else {
        loadEmailsFromAccountEmailBox();
    }
}

void EmailBoxWindow::loadEmailsFromAccountEmailBox() {
    QVector<Email> emails = m_emailDataManager->getEmailsFromEmailBox(m_currentAccount.emailAddress);
    displayEmails(emails);
}

void EmailBoxWindow::loadEmailsFromAllEmailBox() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString allEmailBoxPath = appDataPath + "/AllEmailBox.db";

    if (!QFile::exists(allEmailBoxPath)) {
        qDebug() << "[DEBUG] AllEmailBox.db does not exist, loading from all account email boxes";
        QVector<Email> allEmails;
        for (const EmailAccount& account : m_accounts) {
            QVector<Email> accountEmails = m_emailDataManager->getEmailsFromEmailBox(account.emailAddress);
            allEmails.append(accountEmails);
        }
        std::sort(allEmails.begin(), allEmails.end(), [](const Email& a, const Email& b) {
            return a.sentAt > b.sentAt;
        });
        displayEmails(allEmails);
    } else {
        QSqlDatabase allEmailBoxDb = QSqlDatabase::addDatabase("QSQLITE", "all_email_box_load");
        allEmailBoxDb.setDatabaseName(allEmailBoxPath);

        if (allEmailBoxDb.open()) {
            QSqlQuery query(allEmailBoxDb);
            query.prepare("SELECT * FROM emails WHERE folder = :folder ORDER BY sent_at DESC");
            query.bindValue(":folder", static_cast<int>(EmailFolder::Inbox));

            QVector<Email> emails;
            if (query.exec()) {
                while (query.next()) {
                    emails.append(m_emailDataManager->parseEmailFromQuery(query));
                }
            }
            allEmailBoxDb.close();
            displayEmails(emails);
        } else {
            qDebug() << "[ERROR] Failed to open AllEmailBox.db";
            QVector<Email> allEmails;
            for (const EmailAccount& account : m_accounts) {
                QVector<Email> accountEmails = m_emailDataManager->getEmailsFromEmailBox(account.emailAddress);
                allEmails.append(accountEmails);
            }
            std::sort(allEmails.begin(), allEmails.end(), [](const Email& a, const Email& b) {
                return a.sentAt > b.sentAt;
            });
            displayEmails(allEmails);
        }
    }
}

void EmailBoxWindow::displayEmails(const QVector<Email>& emails) {
    /**
     * @brief 显示邮件列表到UI
     * @param emails 要显示的邮件向量
     * 
     * 函数执行流程：
     * 1. 防御性检查 - 验证布局对象有效性
     * 2. 清空现有邮件项 - 释放旧控件资源
     * 3. 创建新的邮件项 - 批量创建并添加到布局
     * 
     * 注意：
     * - 最大显示数量限制为12封邮件 (MAX_DISPLAY_EMAILS)
     * - 每创建20个控件后处理一次事件循环，防止UI卡顿
     * - 在主线程中执行
     */
    qDebug() << "[EmailBoxWindow] displayEmails: START - Thread ID:" << QThread::currentThreadId();
    qDebug() << "[EmailBoxWindow] displayEmails: Received" << emails.size() << "emails to display";
    
    // 防御性检查：验证对象是否仍然有效
    if (!this->m_emailListLayout) {
        qDebug() << "[EmailBoxWindow] displayEmails: ERROR - m_emailListLayout is nullptr!";
        return;
    }
    
    // 步骤1：清空现有邮件项 - 遍历并删除所有旧的邮件列表项控件，防止内存泄漏
    qDebug() << "[EmailBoxWindow] displayEmails: Step 1 - Clearing existing items, count:" << this->m_emailItems.size();
    for (EmailListItemNew* item : this->m_emailItems) {
        this->m_emailListLayout->removeWidget(item);  // 从布局中移除
        delete item;                                  // 释放内存
    }
    this->m_emailItems.clear();
    this->m_emailItemMap.clear();
    qDebug() << "[EmailBoxWindow] displayEmails: Step 1 - Existing items cleared";

    // 步骤2：设置显示限制 - 最多显示12封邮件，避免一次性创建过多控件导致性能问题
    const int MAX_DISPLAY_EMAILS = 12;
    int displayCount = qMin(emails.size(), MAX_DISPLAY_EMAILS);
    qDebug() << "[EmailBoxWindow] displayEmails: Step 2 - Will display" << displayCount << "emails (max limit:" << MAX_DISPLAY_EMAILS << ")";

    // 步骤3：批量创建邮件项 - 每创建BATCH_SIZE个控件后处理事件循环，保持UI响应
    const int BATCH_SIZE = 20;
    int successCount = 0;
    
    for (int i = 0; i < displayCount; ++i) {
        const Email& email = emails[i];
        
        qDebug() << "[EmailBoxWindow] displayEmails: Creating item" << (i + 1) << "/" << displayCount << "- Subject:" << email.subject;

        try {
            // 创建邮件列表项控件，this作为父对象自动管理生命周期
            EmailListItemNew* item = new EmailListItemNew(this);
            
            qDebug() << "[EmailBoxWindow] displayEmails: Setting email for item" << (i + 1);
            item->setEmail(email);

            qDebug() << "[EmailBoxWindow] displayEmails: Connecting signals for item" << (i + 1);
            connect(item, &EmailListItemNew::viewClicked, this, &EmailBoxWindow::onEmailViewClicked);
            connect(item, &EmailListItemNew::deleteClicked, this, &EmailBoxWindow::onEmailDeleteClicked);

            this->m_emailItems.append(item);
            this->m_emailItemMap[email.id] = item;
            
            qDebug() << "[EmailBoxWindow] displayEmails: Inserting item" << (i + 1) << "to layout";
            this->m_emailListLayout->insertWidget(this->m_emailListLayout->count() - 1, item);
            successCount++;

            if ((i + 1) % BATCH_SIZE == 0) {
                qDebug() << "[EmailBoxWindow] displayEmails: Processing events batch" << ((i + 1) / BATCH_SIZE);
                qApp->processEvents();
            }
        } catch (const std::exception& e) {
            qDebug() << "[EmailBoxWindow] displayEmails: ERROR - Exception creating item" << (i + 1) << ":" << e.what();
        } catch (...) {
            qDebug() << "[EmailBoxWindow] displayEmails: ERROR - Unknown exception creating item" << (i + 1);
        }
    }

    qDebug() << "[EmailBoxWindow] displayEmails: Step 3 - Display complete, success:" << successCount << "items";
    qDebug() << "[EmailBoxWindow] displayEmails: END";
}

void EmailBoxWindow::cleanupProgressDialog() {
    qDebug() << "[EmailBoxWindow] cleanupProgressDialog: START - Thread ID:" << QThread::currentThreadId();
    
    // 防御性检查：验证对象是否仍然有效
    if (!this->m_refreshBtn) {
        qDebug() << "[EmailBoxWindow] cleanupProgressDialog: ERROR - m_refreshBtn is nullptr!";
        return;
    }
    
    qDebug() << "[EmailBoxWindow] cleanupProgressDialog: Enabling refresh button...";
    this->m_refreshBtn->setEnabled(true);
    this->m_refreshBtn->setText("刷新");
    qDebug() << "[EmailBoxWindow] cleanupProgressDialog: Refresh button enabled";

    if (this->m_progressDialog) {
        qDebug() << "[EmailBoxWindow] cleanupProgressDialog: Closing progress dialog...";
        this->m_progressDialog->close();
        qDebug() << "[EmailBoxWindow] cleanupProgressDialog: Progress dialog closed, scheduling delete...";
        this->m_progressDialog->deleteLater();
        this->m_progressDialog = nullptr;
        qDebug() << "[EmailBoxWindow] cleanupProgressDialog: Progress dialog deleted";
    } else {
        qDebug() << "[EmailBoxWindow] cleanupProgressDialog: m_progressDialog is already nullptr";
    }

    this->m_fetchStep = FetchStep::Idle;
    qDebug() << "[EmailBoxWindow] cleanupProgressDialog: Set m_fetchStep to Idle";
    
    qDebug() << "[EmailBoxWindow] cleanupProgressDialog: END";
}

void EmailBoxWindow::onAccountSelected(int index) {
    if (index < 0) {
        return;
    }

    if (index == 0) {
        m_currentAccount = EmailAccount();
        m_emailDataManager->mergeEmailBoxesToAll(m_accounts);
        loadEmailsFromAllEmailBox();
        return;
    }

    int accountIndex = index - 1;
    if (accountIndex < 0 || accountIndex >= m_accounts.size()) {
        return;
    }

    m_currentAccount = m_accounts[accountIndex];

    // 断开之前的连接（如果有）
    if (m_pythonEmailClient->isConnected()) {
        m_pythonEmailClient->disconnect();
    }

    // 仅加载本地缓存，不触发网络请求
    loadLocalCachedEmails();
}

void EmailBoxWindow::downloadAttachment(const Email& email, const EmailAttachment& attachment, QDialog* parentDialog) {
    qDebug() << "[DEBUG] downloadAttachment:" << attachment.filename;
    
    if (!m_pythonEmailClient->isConnected()) {
        QMessageBox::warning(this, "下载失败", "未连接到邮件服务器，无法下载附件");
        return;
    }
    
    QString defaultFileName = attachment.filename;
    QString saveFilePath = QFileDialog::getSaveFileName(
        this,
        "保存附件",
        defaultFileName,
        "所有文件 (*.*)"
    );
    
    if (saveFilePath.isEmpty()) {
        qDebug() << "[DEBUG] User cancelled save dialog";
        return;
    }
    
    qDebug() << "[DEBUG] Saving attachment to:" << saveFilePath;
    qDebug() << "[DEBUG] Email messageId:" << email.messageId;
    
    QDialog* progressDialog = new QDialog(parentDialog);
    progressDialog->setAttribute(Qt::WA_DeleteOnClose);
    progressDialog->setWindowTitle("正在下载附件");
    progressDialog->setModal(true);
    progressDialog->setFixedSize(400, 150);
    progressDialog->setStyleSheet("QDialog { background-color: white; }");
    
    QVBoxLayout* progressLayout = new QVBoxLayout(progressDialog);
    progressLayout->setContentsMargins(20, 20, 20, 20);
    progressLayout->setSpacing(15);
    
    QLabel* fileNameLabel = new QLabel(QString("正在下载: %1").arg(attachment.filename), progressDialog);
    fileNameLabel->setStyleSheet("font-size: 14px; color: #333;");
    progressLayout->addWidget(fileNameLabel);
    
    QProgressBar* progressBar = new QProgressBar(progressDialog);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setStyleSheet("QProgressBar { border: 1px solid #ccc; border-radius: 4px; height: 25px; } "
                              "QProgressBar::chunk { background-color: #4caf50; border-radius: 3px; }");
    progressLayout->addWidget(progressBar);
    
    QLabel* statusLabel = new QLabel("准备下载...", progressDialog);
    statusLabel->setStyleSheet("font-size: 12px; color: #666;");
    progressLayout->addWidget(statusLabel);
    
    progressLayout->addStretch();
    progressDialog->setLayout(progressLayout);
    progressDialog->show();
    
    progressBar->setValue(30);
    statusLabel->setText("正在从服务器获取附件...");
    QCoreApplication::processEvents();
    
    qDebug() << "[DEBUG] Starting fetch with messageNumber:" << email.messageId.toInt();
    
    int messageNumber = email.messageId.toInt();
    EmailAttachment fetchedAttachment = m_pythonEmailClient->fetchAttachment(messageNumber, attachment.filename);
    
    qDebug() << "[DEBUG] Fetch result - size:" << fetchedAttachment.size << "content empty:" << fetchedAttachment.content.isEmpty();
    
    if (fetchedAttachment.content.isEmpty()) {
        qDebug() << "[DEBUG] Attachment content is empty";
        progressDialog->close();
        QMessageBox::warning(this, "下载失败", "无法从服务器获取附件内容");
        return;
    }
    
    progressBar->setValue(60);
    statusLabel->setText("正在保存文件...");
    QCoreApplication::processEvents();
    
    qDebug() << "[DEBUG] Writing file to:" << saveFilePath;
    
    QFile file(saveFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        qint64 bytesWritten = file.write(fetchedAttachment.content);
        file.close();
        
        qDebug() << "[DEBUG] Bytes written:" << bytesWritten << "expected:" << fetchedAttachment.content.size();
        
        if (bytesWritten == fetchedAttachment.content.size()) {
            progressBar->setValue(100);
            statusLabel->setText("下载完成!");
            QCoreApplication::processEvents();
            
            QTimer::singleShot(500, progressDialog, &QDialog::close);
            QMessageBox::information(this, "下载成功", 
                QString("附件已成功保存到:\n%1\n\n文件大小: %2 KB")
                    .arg(saveFilePath)
                    .arg(fetchedAttachment.size / 1024.0, 0, 'f', 2));
        } else {
            qDebug() << "[DEBUG] Write failed - bytes mismatch";
            progressDialog->close();
            QMessageBox::warning(this, "下载失败", "写入文件时发生错误");
        }
    } else {
        qDebug() << "[DEBUG] Cannot open file for writing";
        progressDialog->close();
        QMessageBox::warning(this, "下载失败", 
            QString("无法创建文件:\n%1").arg(saveFilePath));
    }
}

void EmailBoxWindow::fetchAndShowEmailBody(Email& email) {
    qDebug() << "[DEBUG] fetchAndShowEmailBody: Starting...";
    
    int messageNumber = email.messageId.toInt();
    qDebug() << "[DEBUG] fetchAndShowEmailBody: Fetching email body for messageNumber:" << messageNumber;
    
    PythonEmailClient::EmailContent content = m_pythonEmailClient->fetchEmailBody(messageNumber);
    
    qDebug() << "[DEBUG] fetchAndShowEmailBody: content.subject length=" << content.subject.length();
    qDebug() << "[DEBUG] fetchAndShowEmailBody: content.body length=" << content.body.length();
    qDebug() << "[DEBUG] fetchAndShowEmailBody: content.htmlBody length=" << content.htmlBody.length();
    
    if (!content.subject.isEmpty() || !content.body.isEmpty() || !content.htmlBody.isEmpty()) {
        email.body = content.body;
        email.htmlBody = content.htmlBody;
        
        qDebug() << "[DEBUG] fetchAndShowEmailBody: Updating email in database...";
        m_emailDataManager->updateEmail(email);
        qDebug() << "[DEBUG] fetchAndShowEmailBody: Email body fetched and updated successfully";
    } else {
        qDebug() << "[ERROR] fetchAndShowEmailBody: Failed to fetch email body - all fields are empty";
    }
}

void EmailBoxWindow::onEmailViewClicked(const Email& email) {
    qDebug() << "[DEBUG] ===== onEmailViewClicked: START =====";
    qDebug() << "[DEBUG] onEmailViewClicked: Email subject:" << email.subject;
    qDebug() << "[DEBUG] onEmailViewClicked: Email id:" << email.id;
    qDebug() << "[DEBUG] onEmailViewClicked: Email messageId:" << email.messageId;
    qDebug() << "[DEBUG] onEmailViewClicked: Email accountId:" << email.accountId;
    qDebug() << "[DEBUG] onEmailViewClicked: Email body isEmpty:" << email.body.isEmpty();
    qDebug() << "[DEBUG] onEmailViewClicked: Email htmlBody isEmpty:" << email.htmlBody.isEmpty();
    qDebug() << "[DEBUG] onEmailViewClicked: m_currentAccount.emailAddress:" << m_currentAccount.emailAddress;
    qDebug() << "[DEBUG] onEmailViewClicked: m_currentAccount.id:" << m_currentAccount.id;
    
    Email emailToShow = email;
    
    qDebug() << "[DEBUG] onEmailViewClicked: Querying local database for email id:" << email.id;
    Email localEmail = m_emailDataManager->getEmailById(email.id);
    qDebug() << "[DEBUG] onEmailViewClicked: Local email query complete";
    qDebug() << "[DEBUG] onEmailViewClicked: localEmail.id:" << localEmail.id;
    qDebug() << "[DEBUG] onEmailViewClicked: localEmail.body.isEmpty:" << localEmail.body.isEmpty();
    qDebug() << "[DEBUG] onEmailViewClicked: localEmail.body.length:" << localEmail.body.length();
    qDebug() << "[DEBUG] onEmailViewClicked: localEmail.htmlBody.isEmpty:" << localEmail.htmlBody.isEmpty();
    qDebug() << "[DEBUG] onEmailViewClicked: localEmail.htmlBody.length:" << localEmail.htmlBody.length();
    
    if (!localEmail.body.isEmpty() || !localEmail.htmlBody.isEmpty()) {
        qDebug() << "[DEBUG] onEmailViewClicked: Using local cached email body (length:" << localEmail.body.length() << "/" << localEmail.htmlBody.length() << ")";
        emailToShow.body = localEmail.body;
        emailToShow.htmlBody = localEmail.htmlBody;
        qDebug() << "[DEBUG] onEmailViewClicked: Local cache found, skip server fetch, showing email directly";
    } else if (emailToShow.body.isEmpty() && emailToShow.htmlBody.isEmpty()) {
        qDebug() << "[DEBUG] onEmailViewClicked: Email body is empty, fetching from server...";
        
        QProgressDialog loadingDialog(this, Qt::Dialog | Qt::WindowTitleHint);
        loadingDialog.setWindowTitle("加载中");
        loadingDialog.setLabelText("正在获取邮件内容，请稍候...");
        loadingDialog.setRange(0, 0);
        loadingDialog.setCancelButton(nullptr);
        loadingDialog.setModal(true);
        loadingDialog.show();
        QApplication::processEvents();
        
        qDebug() << "[DEBUG] onEmailViewClicked: Checking IMAP connection state...";
        qDebug() << "[DEBUG] onEmailViewClicked: m_pythonEmailClient pointer:" << m_pythonEmailClient;
        qDebug() << "[DEBUG] onEmailViewClicked: isConnected():" << m_pythonEmailClient->isConnected();
        
        bool needReconnect = !m_pythonEmailClient->isConnected();
        qDebug() << "[DEBUG] onEmailViewClicked: needReconnect:" << needReconnect;
        
        if (needReconnect) {
            qDebug() << "[DEBUG] onEmailViewClicked: Reconnecting to IMAP server...";
            qDebug() << "[DEBUG] onEmailViewClicked: Server:" << m_currentAccount.imapServer << ":" << (m_currentAccount.imapPort > 0 ? m_currentAccount.imapPort : 993);
            qDebug() << "[DEBUG] onEmailViewClicked: Username:" << m_currentAccount.emailAddress;
            qDebug() << "[DEBUG] onEmailViewClicked: Password length:" << m_currentAccount.password.length();
            
            loadingDialog.setLabelText("正在获取邮件内容...");
            QApplication::processEvents();
            
            bool connectSuccess = m_pythonEmailClient->connectToServer(
                m_currentAccount.imapServer,
                m_currentAccount.imapPort > 0 ? m_currentAccount.imapPort : 993,
                m_currentAccount.emailAddress,
                m_currentAccount.password,
                true
            );
            
            qDebug() << "[DEBUG] onEmailViewClicked: connectToServer returned:" << connectSuccess;
            qDebug() << "[DEBUG] onEmailViewClicked: isConnected() after connect:" << m_pythonEmailClient->isConnected();
            
            if (!connectSuccess || !m_pythonEmailClient->isConnected()) {
                qDebug() << "[ERROR] onEmailViewClicked: Connection failed";
                loadingDialog.close();
                QMessageBox::warning(this, "错误", "连接服务器失败，请检查网络连接");
                return;
            }
            
            qDebug() << "[DEBUG] onEmailViewClicked: Connected successfully, ready to fetch email body";
        } else {
            qDebug() << "[DEBUG] onEmailViewClicked: Already connected, checking mailbox selection...";
        }
        
        // 确保邮箱已选择（状态为Selected）
        qDebug() << "[DEBUG] onEmailViewClicked: Checking mailbox selection state...";
        qDebug() << "[DEBUG] onEmailViewClicked: m_pythonEmailClient->state():" << static_cast<int>(m_pythonEmailClient->state());
        
        if (m_pythonEmailClient->state() != PythonEmailClient::ConnectionState::Selected) {
            qDebug() << "[DEBUG] onEmailViewClicked: Mailbox not selected, selecting INBOX...";
            loadingDialog.setLabelText("正在打开收件箱...");
            QApplication::processEvents();
            
            if (!m_pythonEmailClient->selectMailbox("INBOX")) {
                qDebug() << "[ERROR] onEmailViewClicked: Failed to select mailbox";
                loadingDialog.close();
                QMessageBox::warning(this, "错误", "无法打开收件箱，请重试");
                return;
            }
            qDebug() << "[DEBUG] onEmailViewClicked: Mailbox selected successfully";
        } else {
            qDebug() << "[DEBUG] onEmailViewClicked: Mailbox already selected";
        }
        
        int messageNumber = emailToShow.messageId.toInt();
        qDebug() << "[DEBUG] onEmailViewClicked: messageNumber:" << messageNumber;
        qDebug() << "[DEBUG] onEmailViewClicked: Fetching email body for messageNumber:" << messageNumber;
        
        loadingDialog.setLabelText("正在获取邮件内容...");
        QApplication::processEvents();
        
        PythonEmailClient::EmailContent content = m_pythonEmailClient->fetchEmailBody(messageNumber);
        
        qDebug() << "[DEBUG] onEmailViewClicked: fetchEmailBody returned, content.subject:" << content.subject;
        qDebug() << "[DEBUG] onEmailViewClicked: content.body.length:" << content.body.length();
        qDebug() << "[DEBUG] onEmailViewClicked: content.htmlBody.length:" << content.htmlBody.length();
        
        if (!content.subject.isEmpty() || !content.body.isEmpty() || !content.htmlBody.isEmpty()) {
            emailToShow.body = content.body;
            emailToShow.htmlBody = content.htmlBody;
            
            qDebug() << "[DEBUG] onEmailViewClicked: Updating email in database...";
            m_emailDataManager->updateEmail(emailToShow);
            qDebug() << "[DEBUG] onEmailViewClicked: Email body fetched and updated in database";
        } else {
            qDebug() << "[ERROR] onEmailViewClicked: Failed to fetch email body - all fields are empty";
        }
        
        loadingDialog.close();
        qDebug() << "[DEBUG] onEmailViewClicked: Loading dialog closed";
    } else {
        qDebug() << "[DEBUG] onEmailViewClicked: Email has body/HTML content, showing directly";
    }
    
    qDebug() << "[DEBUG] onEmailViewClicked: Creating email detail dialog...";
    qDebug() << "[DEBUG] onEmailViewClicked: emailToShow.subject:" << emailToShow.subject;
    qDebug() << "[DEBUG] onEmailViewClicked: emailToShow.body.isEmpty:" << emailToShow.body.isEmpty();
    qDebug() << "[DEBUG] onEmailViewClicked: emailToShow.htmlBody.isEmpty:" << emailToShow.htmlBody.isEmpty();
    qDebug() << "[DEBUG] onEmailViewClicked: emailToShow.fromAddress:" << emailToShow.fromAddress;
    qDebug() << "[DEBUG] onEmailViewClicked: emailToShow.toAddress:" << emailToShow.toAddress;
    qDebug() << "[DEBUG] onEmailViewClicked: emailToShow.attachments.count:" << emailToShow.attachments.size();
    
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("邮件详情");
    dialog->setMinimumSize(900, 700);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    QWidget* headerWidget = new QWidget(dialog);
    headerWidget->setStyleSheet("background-color: #f8f9fa; border-bottom: 2px solid #e0e0e0; padding: 15px;");
    QVBoxLayout* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);
    
    QLabel* subjectLabel = new QLabel(dialog);
    subjectLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1a1a1a;");
    subjectLabel->setText(emailToShow.subject.isEmpty() ? "(无主题)" : emailToShow.subject);
    subjectLabel->setWordWrap(true);
    headerLayout->addWidget(subjectLabel);
    
    QLabel* infoLabel = new QLabel(dialog);
    infoLabel->setStyleSheet("font-size: 14px; color: #555;");
    QDateTime displayDate = emailToShow.sentAt.isNull() ? emailToShow.receivedAt : emailToShow.sentAt;
    QString statusText = emailToShow.isRead ? "已读" : "未读";
    if (emailToShow.isDeleted) {
        statusText = "已删除";
    }
    infoLabel->setText(QString("From %1 to %2 in %3 [%4]").arg(emailToShow.fromAddress).arg(emailToShow.toAddress).arg(displayDate.toString("yyyy-MM-dd hh:mm")).arg(statusText));
    headerLayout->addWidget(infoLabel);
    
    layout->addWidget(headerWidget);
    
    QScrollArea* scrollArea = new QScrollArea(dialog);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: white; }");
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    QWidget* contentWidget = new QWidget(scrollArea);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(10);
    
    if (!emailToShow.htmlBody.isEmpty()) {
        QTextBrowser* contentBrowser = new QTextBrowser(contentWidget);
        contentBrowser->setStyleSheet(
            "QTextBrowser { background-color: white; border: none; font-size: 14px; line-height: 1.6; }"
        );
        contentBrowser->setOpenExternalLinks(true);
        contentBrowser->setHtml(emailToShow.htmlBody);
        contentLayout->addWidget(contentBrowser);
    } else if (!emailToShow.body.isEmpty()) {
        QTextBrowser* textBrowser = new QTextBrowser(contentWidget);
        textBrowser->setStyleSheet(
            "QTextBrowser { background-color: white; border: none; font-size: 14px; line-height: 1.8; color: #333; }"
        );
        
        QString displayText = emailToShow.body.toHtmlEscaped();
        displayText.replace("\n", "<br/>");
        textBrowser->setHtml(QString("<pre style='font-family: \"Microsoft YaHei\", \"SimSun\", sans-serif; white-space: pre-wrap; word-wrap: break-word;'>%1</pre>").arg(displayText));
        contentLayout->addWidget(textBrowser);
    } else {
        QLabel* noContentLabel = new QLabel(contentWidget);
        noContentLabel->setStyleSheet("color: #999; font-size: 16px; padding: 50px;");
        noContentLabel->setAlignment(Qt::AlignCenter);
        noContentLabel->setText("(无内容)");
        contentLayout->addWidget(noContentLabel);
    }
    
    scrollArea->setWidget(contentWidget);
    layout->addWidget(scrollArea, 1);
    
    if (!email.attachments.isEmpty()) {
        QGroupBox* attachmentGroup = new QGroupBox(QString("附件 (%1)").arg(email.attachments.size()), dialog);
        attachmentGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 10px; } "
                                      "QGroupBox::title { subcontrol-origin: margin; padding: 5px 10px; }");
        QVBoxLayout* attLayout = new QVBoxLayout(attachmentGroup);
        
        for (int i = 0; i < email.attachments.size(); ++i) {
            const EmailAttachment& attachment = email.attachments[i];
            
            QWidget* attWidget = new QWidget(dialog);
            attWidget->setStyleSheet("background-color: #f5f5f5; border-radius: 4px; margin: 2px;");
            QHBoxLayout* attHLayout = new QHBoxLayout(attWidget);
            attHLayout->setContentsMargins(10, 5, 10, 5);
            attHLayout->setSpacing(10);
            
            QLabel* attIcon = new QLabel("📎", attWidget);
            attIcon->setStyleSheet("font-size: 16px;");
            attHLayout->addWidget(attIcon);
            
            QLabel* attName = new QLabel(attachment.filename, attWidget);
            attName->setStyleSheet("color: #1976d2; font-size: 14px;");
            attName->setTextInteractionFlags(Qt::TextSelectableByMouse);
            attName->setCursor(Qt::PointingHandCursor);
            attHLayout->addWidget(attName, 1);
            
            QString sizeStr;
            if (attachment.size > 1024 * 1024) {
                sizeStr = QString::number(attachment.size / (1024.0 * 1024.0), 'f', 2) + " MB";
            } else if (attachment.size > 1024) {
                sizeStr = QString::number(attachment.size / 1024.0, 'f', 2) + " KB";
            } else {
                sizeStr = QString::number(attachment.size) + " B";
            }
            QLabel* attSize = new QLabel(sizeStr, attWidget);
            attSize->setStyleSheet("color: #888; font-size: 12px;");
            attHLayout->addWidget(attSize);
            
            QPushButton* downloadBtn = new QPushButton("下载", attWidget);
            downloadBtn->setFixedSize(60, 28);
            downloadBtn->setStyleSheet("QPushButton { background-color: #4caf50; color: white; border: none; "
                                      "border-radius: 4px; font-size: 12px; } "
                                      "QPushButton:hover { background-color: #45a049; }");
            attHLayout->addWidget(downloadBtn);
            
            connect(downloadBtn, &QPushButton::clicked, this, [this, email, attachment, dialog]() {
                this->downloadAttachment(email, attachment, dialog);
            });
            
            attHLayout->addStretch();
            attLayout->addWidget(attWidget);
        }
        
        layout->addWidget(attachmentGroup);
    }
    
    QPushButton* closeBtn = new QPushButton("关闭", dialog);
    closeBtn->setFixedSize(120, 40);
    closeBtn->setStyleSheet("QPushButton { background-color: #666; color: white; border: none; "
                           "border-radius: 6px; font-size: 14px; font-weight: bold; } "
                           "QPushButton:hover { background-color: #555; }");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    layout->addLayout(buttonLayout);
    
    dialog->setLayout(layout);
    dialog->exec();
    
    delete dialog;
}

void EmailBoxWindow::onEmailDeleteClicked(const Email& email) {
    qDebug() << "[DEBUG] onEmailDeleteClicked:" << email.subject;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除",
        QString("确定要删除邮件 \"%1\" 吗？").arg(email.subject),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        qDebug() << "[DEBUG] Deleting email from IMAP server, messageId:" << email.messageId;
        
        int messageNumber = email.messageId.toInt();
        bool success = m_pythonEmailClient->deleteEmail(messageNumber);
        
        if (success) {
            qDebug() << "[DEBUG] Email deleted from IMAP server successfully";
            
            Email updatedEmail = email;
            updatedEmail.isDeleted = true;
            updatedEmail.modifiedAt = QDateTime::currentDateTime();
            m_emailDataManager->updateEmail(updatedEmail);
            
            loadEmails();
            
            QMessageBox::information(this, "删除成功", "邮件已成功删除");
        } else {
            qDebug() << "[ERROR] Failed to delete email from IMAP server";
            QMessageBox::warning(this, "删除失败", "无法从服务器删除邮件");
        }
    }
}

void EmailBoxWindow::updateAccountSelector() {
    m_accountSelector->blockSignals(true);

    m_accountSelector->clear();

    m_accountSelector->addItem("全部账户", -1);

    for (const EmailAccount& account : m_accounts) {
        QString displayText = account.displayName.isEmpty() ?
                             account.emailAddress :
                             account.displayName;
        m_accountSelector->addItem(displayText, account.id);
    }

    if (!m_accounts.isEmpty()) {
        int defaultIndex = -1;
        for (int i = 0; i < m_accounts.size(); ++i) {
            if (m_accounts[i].isDefault) {
                defaultIndex = i;
                break;
            }
        }

        if (defaultIndex >= 0) {
            m_accountSelector->setCurrentIndex(defaultIndex);
        }
    }

    m_accountSelector->blockSignals(false);
}

QString EmailBoxWindow::formatEmailDate(const QDateTime& dateTime) {
    if (!dateTime.isValid()) {
        return "";
    }

    QDateTime now = QDateTime::currentDateTime();
    QDate today = now.date();
    QDate emailDate = dateTime.date();

    if (emailDate == today) {
        return dateTime.toString("hh:mm");
    } else if (emailDate == today.addDays(-1)) {
        return "昨天 " + dateTime.toString("hh:mm");
    } else if (today.year() == emailDate.year()) {
        return dateTime.toString("MM-dd hh:mm");
    } else {
        return dateTime.toString("yyyy-MM-dd");
    }
}

void EmailBoxWindow::applyModernStyle() {
    setStyleSheet(R"(
        QWidget {
            font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif;
            font-size: 13px;
        }

        #toolBar {
            background-color: #fafafa;
            border-bottom: 2px solid #e0e0e0;
        }

        #emailScrollArea {
            background-color: #ffffff;
        }

        #emailListContainer {
            background-color: transparent;
        }

        QScrollBar:vertical {
            border: none;
            background: #f0f0f0;
            width: 10px;
            border-radius: 5px;
        }

        QScrollBar::handle:vertical {
            background: #c0c0c0;
            border-radius: 5px;
            min-height: 30px;
        }

        QScrollBar::handle:vertical:hover {
            background: #a0a0a0;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            height: 0px;
        }

        #emailListItem {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            margin: 5px 0;
        }

        #emailListItem:hover {
            background-color: #f8f9fa;
            border-color: #d0d0d0;
        }

        #emailListItem[selected=true] {
            background-color: #e3f2fd;
            border-color: #1976d2;
        }

        #expandBtn {
            background-color: transparent;
            border: none;
            color: #666;
            font-size: 16px;
            padding: 4px;
        }

        #expandBtn:hover {
            color: #1976d2;
        }

        #detailsWidget {
            background-color: transparent;
        }

        #infoGroup {
            font-weight: bold;
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            margin-top: 8px;
            padding-top: 8px;
            background-color: #ffffff;
        }

        #infoGroup::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            color: #1976d2;
            font-size: 13px;
        }

        QGroupBox {
            font-weight: bold;
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            margin-top: 8px;
            padding-top: 8px;
            background-color: #ffffff;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            color: #1976d2;
            font-size: 13px;
        }

        QLabel#senderLabel {
            font-weight: bold;
            color: #333;
        }

        QLabel#recipientLabel {
            color: #666;
        }

        QLabel#timeLabel {
            color: #999;
            font-size: 12px;
        }

        QLabel#subjectLabel {
            color: #444;
            font-size: 14px;
        }

        QCheckBox {
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 20px;
            height: 20px;
            border: 2px solid #d0d0d0;
            border-radius: 4px;
            background-color: white;
        }

        QCheckBox::indicator:checked {
            background-color: #1976d2;
            border-color: #1976d2;
        }

        QCheckBox::indicator:hover {
            border-color: #1976d2;
        }

        #accountSelector {
            border: 1px solid #d0d0d0;
            border-radius: 6px;
            padding: 8px 12px;
        }

        #accountSelector:hover {
            border-color: #1976d2;
        }

        #bodyText {
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            padding: 8px;
            background-color: #fafafa;
        }

        #attachmentsLabel {
            color: #1976d2;
            font-size: 13px;
        }
    )");
}