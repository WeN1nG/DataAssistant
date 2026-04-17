#include "EmailSendWindow.h"
#include <QApplication>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include "../Database/DatabaseManager.h"

using namespace fa;

EmailSendWindow::EmailSendWindow(QWidget* parent)
    : QWidget(parent)
    , m_awesome(new QtAwesome(qApp))
{
    m_awesome->initFontAwesome();

    setupUI();
    loadSmtpAccounts();
}

EmailSendWindow::~EmailSendWindow() {
}

void EmailSendWindow::setupUI() {
    setWindowTitle("发送邮件");
    setMinimumSize(700, 500);
    setStyleSheet("background-color: #f5f5f5;");

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(10);

    QWidget* row1Widget = new QWidget(this);
    row1Widget->setObjectName("row1Widget");
    row1Widget->setStyleSheet("background-color: white; border: 1px solid #e0e0e0; border-radius: 8px; padding: 12px;");
    QHBoxLayout* row1Layout = new QHBoxLayout(row1Widget);
    row1Layout->setSpacing(12);

    m_senderAccountCombo = new QComboBox(row1Widget);
    m_senderAccountCombo->setObjectName("senderAccountCombo");
    m_senderAccountCombo->setMinimumHeight(32);
    m_senderAccountCombo->setFixedWidth(200);
    m_senderAccountCombo->setPlaceholderText("选择账户");
    m_senderAccountCombo->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #d0d0d0;
            border-radius: 6px;
            padding: 6px 10px;
            background-color: white;
            font-size: 14px;
        }
        QComboBox:hover {
            border-color: #1976d2;
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #666;
        }
    )");
    row1Layout->addWidget(m_senderAccountCombo);

    connect(m_senderAccountCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EmailSendWindow::onSenderAccountSelected);

    QLabel* recipientTitle = new QLabel("发送到:", row1Widget);
    recipientTitle->setStyleSheet("font-weight: bold; color: #555; font-size: 14px;");
    recipientTitle->setFixedWidth(80);
    recipientTitle->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    row1Layout->addWidget(recipientTitle);

    m_recipientEdit = new QLineEdit(row1Widget);
    m_recipientEdit->setObjectName("recipientEdit");
    m_recipientEdit->setPlaceholderText("收件人邮箱");
    m_recipientEdit->setMinimumHeight(32);
    QRegularExpressionValidator* emailValidator = new QRegularExpressionValidator(
        QRegularExpression("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"), this);
    m_recipientEdit->setValidator(emailValidator);
    m_recipientEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #d0d0d0;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border-color: #1976d2;
        }
    )");
    row1Layout->addWidget(m_recipientEdit, 3);

    m_mainLayout->addWidget(row1Widget);

    QWidget* row2Widget = new QWidget(this);
    row2Widget->setObjectName("row2Widget");
    row2Widget->setStyleSheet("background-color: white; border: 1px solid #e0e0e0; border-radius: 8px; padding: 12px;");
    QHBoxLayout* row2Layout = new QHBoxLayout(row2Widget);
    row2Layout->setSpacing(15);

    m_subjectEdit = new QLineEdit(row2Widget);
    m_subjectEdit->setObjectName("subjectEdit");
    m_subjectEdit->setPlaceholderText("邮件主题");
    m_subjectEdit->setMinimumHeight(32);
    m_subjectEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #d0d0d0;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border-color: #1976d2;
        }
    )");
    row2Layout->addWidget(m_subjectEdit, 1);

    m_addAttachmentBtn = new QPushButton(row2Widget);
    m_addAttachmentBtn->setObjectName("addAttachmentBtn");
    m_addAttachmentBtn->setIcon(m_awesome->icon(fa_solid, fa_plus));
    m_addAttachmentBtn->setIconSize(QSize(18, 18));
    m_addAttachmentBtn->setToolTip("添加附件");
    m_addAttachmentBtn->setMinimumHeight(32);
    m_addAttachmentBtn->setMinimumWidth(40);
    m_addAttachmentBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4caf50;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 6px 10px;
            font-weight: bold;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
    )");
    connect(m_addAttachmentBtn, &QPushButton::clicked, this, &EmailSendWindow::onAddAttachment);
    row2Layout->addWidget(m_addAttachmentBtn);

    m_mainLayout->addWidget(row2Widget);

    QWidget* row3Widget = new QWidget(this);
    row3Widget->setObjectName("row3Widget");
    row3Widget->setStyleSheet("background-color: white; border: 1px solid #e0e0e0; border-radius: 8px; padding: 12px;");
    QVBoxLayout* row3Layout = new QVBoxLayout(row3Widget);
    row3Layout->setSpacing(10);

    m_contentTextEdit = new QTextEdit(row3Widget);
    m_contentTextEdit->setObjectName("contentTextEdit");
    m_contentTextEdit->setPlaceholderText("邮件内容");
    m_contentTextEdit->setMinimumHeight(180);
    m_contentTextEdit->setStyleSheet(R"(
        QTextEdit {
            border: 1px solid #d0d0d0;
            border-radius: 6px;
            padding: 8px;
            font-size: 14px;
            background-color: white;
        }
        QTextEdit:focus {
            border-color: #1976d2;
        }
    )");
    row3Layout->addWidget(m_contentTextEdit);

    m_attachmentListWidget = new QListWidget(row3Widget);
    m_attachmentListWidget->setObjectName("attachmentListWidget");
    m_attachmentListWidget->setMaximumHeight(50);
    m_attachmentListWidget->setStyleSheet(R"(
        QListWidget {
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            background-color: #fafafa;
            padding: 5px;
            font-size: 13px;
        }
        QListWidget::item {
            padding: 3px;
        }
    )");
    row3Layout->addWidget(m_attachmentListWidget);

    m_mainLayout->addWidget(row3Widget);

    QWidget* row4Widget = new QWidget(this);
    row4Widget->setObjectName("row4Widget");
    QHBoxLayout* row4Layout = new QHBoxLayout(row4Widget);
    row4Layout->setContentsMargins(0, 10, 0, 0);
    row4Layout->setSpacing(15);

    row4Layout->addStretch();

    m_cancelBtn = new QPushButton("取消", row4Widget);
    m_cancelBtn->setObjectName("cancelBtn");
    m_cancelBtn->setIcon(m_awesome->icon(fa_solid, fa_xmark));
    m_cancelBtn->setIconSize(QSize(16, 16));
    m_cancelBtn->setMinimumHeight(38);
    m_cancelBtn->setMinimumWidth(100);
    m_cancelBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #f44336;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            font-weight: bold;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #d32f2f;
        }
    )");
    connect(m_cancelBtn, &QPushButton::clicked, this, &EmailSendWindow::onCancelSend);
    row4Layout->addWidget(m_cancelBtn);

    m_sendBtn = new QPushButton("发送", row4Widget);
    m_sendBtn->setObjectName("sendBtn");
    m_sendBtn->setIcon(m_awesome->icon(fa_solid, fa_paper_plane));
    m_sendBtn->setIconSize(QSize(16, 16));
    m_sendBtn->setMinimumHeight(38);
    m_sendBtn->setMinimumWidth(100);
    m_sendBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1976d2;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            font-weight: bold;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #1565c0;
        }
    )");
    row4Layout->addWidget(m_sendBtn);

    m_mainLayout->addWidget(row4Widget);
}

void EmailSendWindow::loadSmtpAccounts() {
    DatabaseManager dbManager;
    QVector<EmailAccount> accounts = dbManager.getSmtpAccounts();

    m_senderAccountCombo->clear();
    for (const EmailAccount& account : accounts) {
        QString displayText = account.displayName.isEmpty() ?
                             account.emailAddress :
                             QString("%1 (%2)").arg(account.displayName).arg(account.emailAddress);
        m_senderAccountCombo->addItem(displayText, account.id);
    }
}

void EmailSendWindow::updateAttachmentList() {
    m_attachmentListWidget->clear();
}

void EmailSendWindow::onAddAttachment() {
}

void EmailSendWindow::onRemoveAttachment() {
}

void EmailSendWindow::onCancelSend() {
    clear();
    QMessageBox::information(this, "取消", "已取消发送邮件");
}

void EmailSendWindow::clear() {
    m_senderAccountCombo->setCurrentIndex(0);
    m_recipientEdit->clear();
    m_subjectEdit->clear();
    m_contentTextEdit->clear();
    m_attachmentListWidget->clear();
}

void EmailSendWindow::onSenderAccountSelected(int index) {
    if (index < 0) {
        return;
    }
    int accountId = m_senderAccountCombo->currentData().toInt();
    qDebug() << "Sender account selected:" << accountId << "at index" << index;
}
