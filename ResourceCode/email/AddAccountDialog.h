#ifndef ADDACCOUNTDIALOG_H
#define ADDACCOUNTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

#include "EmailModels.h"
#include <QtAwesome.h>

class AddImapAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddImapAccountDialog(QWidget* parent = nullptr);
    ~AddImapAccountDialog();

    EmailAccount getAccount() const;

private slots:
    void onSave();
    void onCancel();

private:
    QLineEdit* m_emailEdit;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_serverEdit;
    QSpinBox* m_portSpin;
    QComboBox* m_encryptionCombo;
    QLineEdit* m_passwordEdit;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
    fa::QtAwesome* m_awesome;
};

class AddSmtpAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddSmtpAccountDialog(QWidget* parent = nullptr);
    ~AddSmtpAccountDialog();

    EmailAccount getAccount() const;

private slots:
    void onSave();
    void onCancel();

private:
    QLineEdit* m_emailEdit;
    QLineEdit* m_serverEdit;
    QSpinBox* m_portSpin;
    QComboBox* m_encryptionCombo;
    QLineEdit* m_passwordEdit;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
    fa::QtAwesome* m_awesome;
};

class EditImapAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditImapAccountDialog(const EmailAccount& account, QWidget* parent = nullptr);
    ~EditImapAccountDialog();

    EmailAccount getAccount() const;

private slots:
    void onSave();
    void onCancel();

private:
    EmailAccount m_account;
    QLineEdit* m_emailEdit;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_serverEdit;
    QSpinBox* m_portSpin;
    QComboBox* m_encryptionCombo;
    QLineEdit* m_passwordEdit;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
    fa::QtAwesome* m_awesome;
};

class EditSmtpAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditSmtpAccountDialog(const EmailAccount& account, QWidget* parent = nullptr);
    ~EditSmtpAccountDialog();

    EmailAccount getAccount() const;

private slots:
    void onSave();
    void onCancel();

private:
    EmailAccount m_account;
    QLineEdit* m_emailEdit;
    QLineEdit* m_serverEdit;
    QSpinBox* m_portSpin;
    QComboBox* m_encryptionCombo;
    QLineEdit* m_passwordEdit;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
    fa::QtAwesome* m_awesome;
};

#endif
