#ifndef EMAILBOXWINDOW_H
#define EMAILBOXWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QTextEdit>
#include <QTextBrowser>
#include <QTabWidget>
#include <QVector>
#include <QMap>
#include <QSignalMapper>
#include <QDateTime>
#include <QGroupBox>
#include <QComboBox>
#include <QMouseEvent>
#include <QProgressDialog>
#include <QTimer>
#include <QShowEvent>
#include <QCloseEvent>
#include <QThread>
#include <QDialog>
#include <QtConcurrent>

#include "EmailDataManager.h"
#include "EmailModels.h"
#include "../../PythonEmail/PythonEmailClient.h"
#include "EmailListItemNew.h"
#include <QtAwesome.h>

class EmailBoxWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EmailBoxWindow(QWidget* parent = nullptr);
    ~EmailBoxWindow();

private slots:
    void onRefreshEmails();
    void onAccountSelected(int index);
    void onEmailViewClicked(const Email& email);
    void onEmailDeleteClicked(const Email& email);
    void onImapConnected();
    void onImapAuthenticated();
    void onImapError(PythonEmailClient::EmailError error, const QString& message);
    void onMailboxSelected(const QString& mailboxName);
    void onFetchProgress(const QString& operation, int current, int total);
    void onConnectionResult(bool success);
    void onEmailListFetchedAsJson(const QString& jsonResult);


private:
    enum class FetchStep {
        Idle,
        Connecting,
        Authenticating,
        SelectingMailbox,
        FetchingEmails
    };

    void setupUI();
    void setupToolBar();
    void setupEmailListArea();
    void loadEmails();
    void updateEmailList();
    void updateAccountSelector();
    void applyModernStyle();
    QString formatEmailDate(const QDateTime& dateTime);
    
    // ==================== 刷新相关函数 ====================
    void refreshSingleAccount(const EmailAccount& account);    // 单账户刷新
    void refreshAllAccounts();                                  // 全部账户刷新
    void loadLocalCachedEmails();                              // 加载本地缓存（不触发网络请求）
    
    // 以下函数标记为废弃，不再直接调用
    // void fetchEmailsFromServer();  // 已废弃，使用 refreshSingleAccount 替代
    // void performConnection();      // 已废弃，逻辑合并到 refreshSingleAccount
    // void fetchEmailsForAllAccounts(); // 已废弃，使用 refreshAllAccounts 替代
    
    void saveFetchedEmails(const QVector<PythonEmailClient::EmailInfo>& emailList);
    void saveFetchedEmailsFromJson(const QString& jsonResult);
    void cleanupProgressDialog();
    void setupImapThread();
    void cleanupImapThread();
    void downloadAttachment(const Email& email, const EmailAttachment& attachment, QDialog* parentDialog);
    void fetchAndShowEmailBody(Email& email);
    void loadEmailsFromEmailBox();
    void loadEmailsFromAccountEmailBox();
    void loadEmailsFromAllEmailBox();
    void displayEmails(const QVector<Email>& emails);

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    EmailDataManager* m_emailDataManager;
    PythonEmailClient* m_pythonEmailClient;
    QThread* m_imapThread;
    fa::QtAwesome* m_awesome;
    QProgressDialog* m_progressDialog;
    FetchStep m_fetchStep;
    QVector<PythonEmailClient::EmailInfo> m_fetchedEmailList;

    QVBoxLayout* m_mainLayout;

    QWidget* m_toolBar;
    QPushButton* m_refreshBtn;
    QComboBox* m_accountSelector;

    QScrollArea* m_emailScrollArea;
    QWidget* m_emailListContainer;
    QVBoxLayout* m_emailListLayout;

    QVector<EmailListItemNew*> m_emailItems;
    QMap<int, EmailListItemNew*> m_emailItemMap;

    EmailAccount m_currentAccount;
    QVector<EmailAccount> m_accounts;
    Email m_currentEmail;
    bool m_isInitialized;
    bool m_isSyncingAllAccounts;
};

#endif