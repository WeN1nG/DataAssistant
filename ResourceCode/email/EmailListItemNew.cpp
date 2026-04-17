#include "EmailListItemNew.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

EmailListItemNew::EmailListItemNew(QWidget* parent)
    : QFrame(parent)
{
    // 设置基础属性
    setObjectName("emailListItem");
    setFrameShape(QFrame::StyledPanel);
    setLineWidth(1);
    setMinimumHeight(110);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 初始化主布局 - 垂直布局管理器
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 15, 20, 15);  // 左右20px，上下15px的外边距
    m_mainLayout->setSpacing(0);  // 各行之间的间距为20px

    // ========== 第一行布局：发件人、收件人、时间 ==========
    QHBoxLayout* row1Layout = new QHBoxLayout();
    row1Layout->setSpacing(10);

    // 发件人标签
    QLabel* fromLabel = new QLabel("from", this);
    fromLabel->setStyleSheet("color: #555; font-size: 14px; font-weight: bold;");

    // 发件人信息标签
    m_senderLabel = new QLabel(this);
    m_senderLabel->setObjectName("senderLabel");
    m_senderLabel->setStyleSheet("color: #333; font-size: 14px;");
    m_senderLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);  // 支持鼠标选中文本
    m_senderLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    // 收件人标签
    QLabel* toLabel = new QLabel("to", this);
    toLabel->setStyleSheet("color: #555; font-size: 14px; font-weight: bold;");

    // 收件人信息标签
    m_recipientLabel = new QLabel(this);
    m_recipientLabel->setObjectName("recipientLabel");
    m_recipientLabel->setStyleSheet("color: #333; font-size: 14px;");
    m_recipientLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_recipientLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    // 时间标签
    m_timeLabel = new QLabel(this);
    m_timeLabel->setObjectName("timeLabel");
    m_timeLabel->setStyleSheet("color: #888; font-size: 13px;");
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);  // 右对齐，垂直居中

    // 将第一行的所有组件添加到布局中
    row1Layout->addWidget(fromLabel);
    row1Layout->addWidget(m_senderLabel, 1);  // stretch factor为1，可拉伸
    row1Layout->addWidget(toLabel);
    row1Layout->addWidget(m_recipientLabel, 1);  // stretch factor为1，可拉伸
    row1Layout->addWidget(m_timeLabel);  // 不拉伸，靠右显示

    m_mainLayout->addLayout(row1Layout);

    // ========== 第二行布局：邮件主题 ==========
    QHBoxLayout* row2Layout = new QHBoxLayout();
    row2Layout->setSpacing(12);

    // 主题标签
    m_subjectLabel = new QLabel(this);
    m_subjectLabel->setObjectName("subjectLabel");
    m_subjectLabel->setStyleSheet("color: #1a1a1a; font-size: 16px;");
    m_subjectLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    row2Layout->addWidget(m_subjectLabel, 1);  // stretch factor为1，占据剩余空间

    m_mainLayout->addLayout(row2Layout);

    // ========== 第三行布局：操作按钮和状态标签 ==========
    QHBoxLayout* row3Layout = new QHBoxLayout();
    row3Layout->setSpacing(10);

    // 状态标签 - 显示"未读"、"已读"或"已删除"
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setStyleSheet("color: #444; font-size: 13px; padding: 4px 12px; border: 1px solid #ccc; border-radius: 4px; background-color: #f5f5f5;");
    m_statusLabel->setFixedWidth(70);  // 固定宽度70px
    m_statusLabel->setAlignment(Qt::AlignCenter);  // 居中对齐

    // 查看按钮 - 绿色背景
    m_viewBtn = new QPushButton("查看", this);
    m_viewBtn->setObjectName("viewBtn");
    m_viewBtn->setFixedSize(80, 32);  // 固定尺寸：宽80px，高32px
    m_viewBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 6px; font-size: 14px; font-weight: bold; } QPushButton:hover { background-color: #45a049; }");
    connect(m_viewBtn, &QPushButton::clicked, this, &EmailListItemNew::onViewClicked);

    // 删除按钮 - 红色背景
    m_deleteBtn = new QPushButton("删除", this);
    m_deleteBtn->setObjectName("deleteBtn");
    m_deleteBtn->setFixedSize(80, 32);
    m_deleteBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 6px; font-size: 14px; font-weight: bold; } QPushButton:hover { background-color: #da190b; }");
    connect(m_deleteBtn, &QPushButton::clicked, this, &EmailListItemNew::onDeleteClicked);

    // 将第三行的组件添加到布局中，各组件之间间距20px
    row3Layout->addWidget(m_deleteBtn);
    row3Layout->addSpacing(20);  // 删除按钮和查看按钮之间的间距
    row3Layout->addWidget(m_viewBtn);
    row3Layout->addSpacing(20);  // 查看按钮和状态标签之间的间距
    row3Layout->addWidget(m_statusLabel);

    m_mainLayout->addLayout(row3Layout);

    setLayout(m_mainLayout);
}

EmailListItemNew::~EmailListItemNew() {
}

void EmailListItemNew::setEmail(const Email& email) {
    m_email = email;
    updateDisplay();
}

void EmailListItemNew::updateDisplay() {
    QString senderText = m_email.fromName.isEmpty() ? m_email.fromAddress : m_email.fromName;
    if (senderText.length() > 40) {
        senderText = senderText.left(40) + "...";
    }
    m_senderLabel->setText(senderText);

    QString recipientText = m_email.toAddress;
    if (recipientText.length() > 40) {
        recipientText = recipientText.left(40) + "...";
    }
    m_recipientLabel->setText(recipientText);

    m_subjectLabel->setText(m_email.subject.isEmpty() ? "(无主题)" : m_email.subject);

    if (!m_email.isRead) {
        m_subjectLabel->setStyleSheet("color: #1a1a1a; font-size: 16px; font-weight: bold;");
    } else {
        m_subjectLabel->setStyleSheet("color: #444; font-size: 16px; font-weight: normal;");
    }

    QDateTime displayDate = m_email.sentAt.isNull() ? m_email.receivedAt : m_email.sentAt;
    m_timeLabel->setText(formatDisplayDate(displayDate));

    if (m_email.isDeleted) {
        m_statusLabel->setText("已删除");
        m_statusLabel->setStyleSheet("color: #999; font-size: 13px; padding: 4px 12px; border: 1px solid #ccc; border-radius: 4px; background-color: #f0f0f0;");
    } else if (m_email.isRead) {
        m_statusLabel->setText("已读");
        m_statusLabel->setStyleSheet("color: #666; font-size: 13px; padding: 4px 12px; border: 1px solid #ccc; border-radius: 4px; background-color: #e8e8e8;");
    } else {
        m_statusLabel->setText("未读");
        m_statusLabel->setStyleSheet("color: #1976d2; font-size: 13px; padding: 4px 12px; border: 1px solid #1976d2; border-radius: 4px; background-color: #e3f2fd; font-weight: bold;");
    }
}

QString EmailListItemNew::formatDisplayDate(const QDateTime& dateTime) {
    if (!dateTime.isValid()) {
        return "";
    }

    QDateTime now = QDateTime::currentDateTime();
    QDate today = now.date();
    QDate emailDate = dateTime.date();

    if (emailDate == today) {
        return dateTime.toString("hh:mm");
    } else if (emailDate == today.addDays(-1)) {
        return "昨天";
    } else if (emailDate.year() == today.year()) {
        return dateTime.toString("MM-dd");
    } else {
        return dateTime.toString("yyyy-MM");
    }
}

void EmailListItemNew::onViewClicked() {
    emit viewClicked(m_email);
}

void EmailListItemNew::onDeleteClicked() {
    emit deleteClicked(m_email);
}
