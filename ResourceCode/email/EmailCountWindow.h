#ifndef EMAILCOUNTWINDOW_H
#define EMAILCOUNTWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>
#include <QSignalMapper>
#include <QInputDialog>
#include <QMessageBox>
#include <QMap>

#include "EmailModels.h"
#include <QtAwesome.h>

class EmailAccountItem : public QFrame
{
    Q_OBJECT

public:
    explicit EmailAccountItem(QWidget* parent = nullptr);
    ~EmailAccountItem();

    void setAccount(const EmailAccount& account);
    EmailAccount getAccount() const { return m_account; }

signals:
    void editClicked(const EmailAccount& account);
    void deleteClicked(const EmailAccount& account);

private slots:

private:
    void updateDisplay();
    void mousePressEvent(QMouseEvent* event) override;

private:
    EmailAccount m_account;

    QLabel* m_accountNameLabel;
    QPushButton* m_editBtn;
    QPushButton* m_deleteBtn;
    fa::QtAwesome* m_awesome;
};

class EmailCountWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EmailCountWindow(QWidget* parent = nullptr);
    ~EmailCountWindow();

private slots:
    void onAddImapAccount();
    void onAddSmtpAccount();
    void onEditImapAccount(const EmailAccount& account);
    void onEditSmtpAccount(const EmailAccount& account);
    void onDeleteImapAccount(const EmailAccount& account);
    void onDeleteSmtpAccount(const EmailAccount& account);
    void onSaveAccount();

private:
    void setupUI();
    void setupImapSection();
    void setupSmtpSection();
    void loadAccounts();
    void updateImapAccountList();
    void updateSmtpAccountList();

private:
    QVBoxLayout* m_mainLayout;
    fa::QtAwesome* m_awesome;

    QWidget* m_imapSection;
    QWidget* m_imapAccountListContainer;
    QVBoxLayout* m_imapAccountListLayout;

    QWidget* m_smtpSection;
    QWidget* m_smtpAccountListContainer;
    QVBoxLayout* m_smtpAccountListLayout;

    QMap<int, EmailAccountItem*> m_imapItemMap;
    QMap<int, EmailAccountItem*> m_smtpItemMap;
};

#endif
