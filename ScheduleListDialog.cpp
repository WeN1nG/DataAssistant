#include "ScheduleListDialog.h"
#include "ScheduleDialog.h"
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QEasingCurve>

ScheduleListDialog::ScheduleListDialog(QWidget* parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_contentWidget(nullptr)
    , m_contentLayout(nullptr)
    , dbManager(new DatabaseManager())
{
    setWindowTitle("日程表");
    setMinimumSize(650, 550);
    setModal(true);
    
    // 设置窗口标志，提升视觉质感
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    setupUI();
    loadSchedules();
}

void ScheduleListDialog::setupUI()
{
    // 主布局：设置背景色和间距
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 设置对话框背景色
    this->setStyleSheet(R"(
        QDialog {
            background-color: #F5F7FA;
        }
    )");
    
    // 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: transparent;
            border: none;
        }
        QScrollBar:vertical {
            background-color: #F5F7FA;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background-color: #C1C7D0;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #A8AFB8;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");
    
    // 创建内容容器
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setSpacing(16);
    m_contentLayout->setContentsMargins(24, 24, 24, 24);
    
    // 关键修复：设置内容 widget 的尺寸策略，确保单一日程不会被拉高
    // 水平方向 Preferred（可伸展），垂直方向 Minimum（只占用内容所需高度）
    m_contentWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_contentWidget->setMinimumHeight(0);  // 允许高度收缩到 0
    
    scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(scrollArea);
    
    setLayout(m_mainLayout);
}

void ScheduleListDialog::loadSchedules()
{
    // 先清空现有内容
    clearLayout(m_contentLayout);
    
    QVector<Schedule> allSchedules = dbManager->getSchedules();
    
    // 排序：所有日程按日期升序排列
    std::sort(allSchedules.begin(), allSchedules.end(),
        [](const Schedule& a, const Schedule& b) {
            return a.datetime < b.datetime;
        });
    
    // 添加所有日程项（移除"未来日程"和"已过期日程"标题）
    for (const Schedule& schedule : allSchedules) {
        QWidget* itemWidget = createScheduleItem(schedule);
        m_contentLayout->addWidget(itemWidget);
    }
    
    // 如果没有任何日程
    if (allSchedules.isEmpty()) {
        QLabel* emptyLabel = new QLabel("📭 暂无日程安排", this);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setFont(QFont("Microsoft YaHei", 14));
        emptyLabel->setStyleSheet("color: #999999; padding: 80px 0; background-color: transparent;");
        m_contentLayout->addWidget(emptyLabel);
    }
    
    // 关键修复：添加可伸缩 spacer，确保日程项不会被 QVBoxLayout 拉高
    m_contentLayout->addStretch(1);
    
    // 强制更新布局
    m_contentWidget->adjustSize();
}

QWidget* ScheduleListDialog::createScheduleItem(const Schedule& schedule)
{
    // 初始化最小化状态
    if (!m_minimizedState.contains(schedule.id)) {
        m_minimizedState[schedule.id] = false;  // 默认为展开状态
    }
    
    // 创建日程项卡片容器
    QWidget* itemWidget = new QWidget();
    itemWidget->setObjectName("scheduleCard");
    itemWidget->setStyleSheet(R"(
        QWidget#scheduleCard {
            background-color: #FFFFFF;
            border: none;
            border-radius: 12px;
            padding: 0;
        }
        QWidget#scheduleCard:hover {
            background-color: #FFFFFF;
        }
        QFrame#cardFrame {
            background-color: #FFFFFF;
            border-radius: 12px;
            border: 1px solid #E8EAED;
        }
        QFrame#cardFrame:hover {
            border-color: #4A90E2;
            background-color: #FAFBFF;
        }
    )");
    
    // 主布局
    QVBoxLayout* itemLayout = new QVBoxLayout(itemWidget);
    itemLayout->setSpacing(0);
    itemLayout->setContentsMargins(0, 0, 0, 0);
    
    // 卡片框架
    QFrame* cardFrame = new QFrame();
    cardFrame->setObjectName("cardFrame");
    cardFrame->setStyleSheet("background-color: transparent;");
    
    QVBoxLayout* cardLayout = new QVBoxLayout(cardFrame);
    cardLayout->setSpacing(12);
    cardLayout->setContentsMargins(20, 16, 20, 16);
    
    // 顶部区域：日期 + 操作按钮
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->setSpacing(12);
    
    // 左侧：日期和优先级标识
    QHBoxLayout* leftLayout = new QHBoxLayout();
    leftLayout->setSpacing(10);
    
    // 优先级标识（彩色小圆点）
    QLabel* priorityDot = new QLabel("•");
    priorityDot->setFont(QFont("Arial", 20, QFont::Bold));
    priorityDot->setFixedWidth(16);
    priorityDot->setAlignment(Qt::AlignCenter);
    if (schedule.priority == 2) {
        priorityDot->setStyleSheet("color: #FF6B6B;");  // 紧急：珊瑚红
    } else if (schedule.priority == 1) {
        priorityDot->setStyleSheet("color: #FFB347;");  // 重要：温暖橙
    } else {
        priorityDot->setStyleSheet("color: #4A90E2;");  // 一般：天空蓝
    }
    leftLayout->addWidget(priorityDot);
    
    // 日期标签
    QDate scheduleDate = schedule.datetime.date();
    QString dateText = QString("%1 月 %2 日").arg(scheduleDate.month()).arg(scheduleDate.day());
    QLabel* dateLabel = new QLabel(dateText);
    dateLabel->setStyleSheet("font-size: 13px; color: #999999; font-weight: 500;");
    dateLabel->setFixedWidth(60);
    leftLayout->addWidget(dateLabel);
    
    topLayout->addLayout(leftLayout);
    
    topLayout->addStretch();
    
    // 右侧：操作按钮组
    QHBoxLayout* buttonGroupLayout = new QHBoxLayout();
    buttonGroupLayout->setSpacing(8);
    
    // 最小化/展开按钮
    QPushButton* minimizeButton = new QPushButton();
    minimizeButton->setCursor(Qt::PointingHandCursor);
    minimizeButton->setFixedSize(28, 28);
    minimizeButton->setToolTip(schedule.datetime.toString("yyyy-MM-dd HH:mm"));
    minimizeButton->setStyleSheet(R"(
        QPushButton {
            background-color: #F0F2F5;
            border: none;
            border-radius: 6px;
            icon-size: 16px;
        }
        QPushButton:hover {
            background-color: #E0E2E5;
        }
        QPushButton:pressed {
            background-color: #D0D2D5;
        }
    )");
    
    // 设置按钮图标和状态
    int scheduleId = schedule.id;
    bool isMinimized = m_minimizedState.value(scheduleId, false);
    
    // 创建动画
    QPropertyAnimation* animation = new QPropertyAnimation(cardFrame, "minimumHeight");
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    m_animations[scheduleId] = animation;
    
    connect(minimizeButton, &QPushButton::clicked, this, [this, scheduleId, itemWidget, animation, minimizeButton]() {
        bool minimized = m_minimizedState.value(scheduleId, false);
        
        // 切换状态
        minimized = !minimized;
        m_minimizedState[scheduleId] = minimized;
        
        // 更新按钮图标
        minimizeButton->setText(minimized ? "📋" : "📝");
        
        // 执行动画
        animation->stop();
        animation->setStartValue(minimized ? itemWidget->height() : 80);
        animation->setEndValue(minimized ? 80 : itemWidget->height());
        animation->start();
        
        // 更新可见性
        updateScheduleItemVisibility(itemWidget, minimized);
    });
    
    // 初始化按钮状态
    minimizeButton->setText(isMinimized ? "📋" : "📝");
    buttonGroupLayout->addWidget(minimizeButton);
    
    // 编辑按钮
    QPushButton* editButton = new QPushButton("编辑");
    editButton->setCursor(Qt::PointingHandCursor);
    editButton->setFixedHeight(28);
    editButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    editButton->setStyleSheet(R"(
        QPushButton {
            background-color: #4A90E2;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: 500;
            padding: 0 16px;
        }
        QPushButton:hover {
            background-color: #357ABD;
        }
        QPushButton:pressed {
            background-color: #2C67A8;
        }
    )");
    connect(editButton, &QPushButton::clicked, this, [this, schedule]() {
        onEditSchedule(schedule.id);
    });
    buttonGroupLayout->addWidget(editButton);
    
    // 删除按钮
    QPushButton* deleteButton = new QPushButton("删除");
    deleteButton->setCursor(Qt::PointingHandCursor);
    deleteButton->setFixedHeight(28);
    deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    deleteButton->setStyleSheet(R"(
        QPushButton {
            background-color: #FF6B6B;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: 500;
            padding: 0 16px;
        }
        QPushButton:hover {
            background-color: #EE5A5A;
        }
        QPushButton:pressed {
            background-color: #DC4A4A;
        }
    )");
    connect(deleteButton, &QPushButton::clicked, this, [this, schedule]() {
        onDeleteSchedule(schedule.id);
    });
    buttonGroupLayout->addWidget(deleteButton);
    
    topLayout->addLayout(buttonGroupLayout);
    cardLayout->addLayout(topLayout);
    
    // 中间区域：日程标题（始终显示）
    QLabel* titleLabel = new QLabel(schedule.title);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-size: 16px; color: #333333; font-weight: 600;");
    titleLabel->setWordWrap(true);
    titleLabel->setContentsMargins(26, 0, 0, 0);
    cardLayout->addWidget(titleLabel);
    
    // 底部区域：日程详情（可最小化）
    if (!schedule.description.isEmpty()) {
        QLabel* descLabel = new QLabel(schedule.description);
        descLabel->setObjectName("descLabel");
        descLabel->setStyleSheet("font-size: 13px; color: #666666; line-height: 1.6;");
        descLabel->setWordWrap(true);
        descLabel->setContentsMargins(26, 0, 0, 0);
        cardLayout->addWidget(descLabel);
    }
    
    itemLayout->addWidget(cardFrame);
    
    // 初始化可见性
    if (isMinimized) {
        updateScheduleItemVisibility(itemWidget, true);
    }
    
    return itemWidget;
}

void ScheduleListDialog::clearLayout(QLayout* layout)
{
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void ScheduleListDialog::updateScheduleItemVisibility(QWidget* itemWidget, bool minimized)
{
    // 查找卡片框架
    QFrame* cardFrame = itemWidget->findChild<QFrame*>("cardFrame");
    if (!cardFrame) {
        return;
    }
    
    // 查找标题和描述标签
    QLabel* titleLabel = cardFrame->findChild<QLabel*>("titleLabel");
    QLabel* descLabel = cardFrame->findChild<QLabel*>("descLabel");
    
    if (minimized) {
        // 最小化状态：隐藏描述，调整标题样式
        if (descLabel) {
            descLabel->hide();
        }
        if (titleLabel) {
            titleLabel->setStyleSheet("font-size: 14px; color: #333333; font-weight: 600;");
        }
        // 设置卡片最小高度为较小值
        cardFrame->setMinimumHeight(60);
    } else {
        // 展开状态：显示所有内容
        if (descLabel) {
            descLabel->show();
        }
        if (titleLabel) {
            titleLabel->setStyleSheet("font-size: 16px; color: #333333; font-weight: 600;");
        }
        // 恢复卡片高度
        cardFrame->setMinimumHeight(0);  // 0 表示根据内容自适应
    }
    
    // 触发布局更新
    cardFrame->layout()->update();
    itemWidget->layout()->update();
    itemWidget->adjustSize();
}

void ScheduleListDialog::onEditSchedule(int scheduleId)
{
    Schedule schedule = dbManager->getScheduleById(scheduleId);
    
    ScheduleDialog dialog(this, &schedule);
    if (dialog.exec() == QDialog::Accepted) {
        QMessageBox::information(this, "成功", "日程已更新");
        refreshScheduleList();
        emit accepted();  // 通知主窗口刷新
    }
}

void ScheduleListDialog::onDeleteSchedule(int scheduleId)
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除这个日程吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (dbManager->deleteSchedule(scheduleId)) {
            refreshScheduleList();
            emit accepted();  // 通知主窗口刷新
        } else {
            QMessageBox::warning(this, "错误", "删除失败");
        }
    }
}

void ScheduleListDialog::refreshScheduleList()
{
    loadSchedules();
}

void ScheduleListDialog::toggleMinimize(int scheduleId)
{
    // 切换最小化状态
    bool minimized = m_minimizedState.value(scheduleId, false);
    m_minimizedState[scheduleId] = !minimized;
    
    // 刷新列表以应用新状态
    refreshScheduleList();
}
