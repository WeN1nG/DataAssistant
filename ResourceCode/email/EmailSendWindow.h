#ifndef EMAILSENDWINDOW_H
#define EMAILSENDWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QListWidget>

#include "EmailModels.h"
#include <QtAwesome.h>

class EmailSendWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EmailSendWindow(QWidget* parent = nullptr);
    ~EmailSendWindow();

private slots:
    void onAddAttachment();
    void onRemoveAttachment();
    void onCancelSend();
    void onSenderAccountSelected(int index);

private:
    void setupUI();
    void loadSmtpAccounts();
    void updateAttachmentList();
    void clear();

private:
    QVBoxLayout* m_mainLayout;
    fa::QtAwesome* m_awesome;

    QComboBox* m_senderAccountCombo;
    QLineEdit* m_recipientEdit;
    QLineEdit* m_subjectEdit;
    QPushButton* m_addAttachmentBtn;
    QListWidget* m_attachmentListWidget;
    QTextEdit* m_contentTextEdit;
    QPushButton* m_cancelBtn;
    QPushButton* m_sendBtn;
};

#endif
