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
    , m_batchActionBar(nullptr)
    , m_batchDelete(nullptr)
    , m_selectAll(nullptr)
    , m_deselectAll(nullptr)
    , m_selectionCountLabel(nullptr)
    , m_loadingOverlay(nullptr)
    , m_loadingLabel(nullptr)
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
    
    // 创建批量操作工具栏
    m_batchActionBar = new QWidget();
    m_batchActionBar->setStyleSheet("background-color: #FFFFFF; border-bottom: 1px solid #E8EAED;");
    QHBoxLayout* batchBarLayout = new QHBoxLayout(m_batchActionBar);
    batchBarLayout->setContentsMargins(16, 12, 16, 12);
    batchBarLayout->setSpacing(12);
    
    // 全选按钮
    m_selectAll = new QPushButton("☑ 全选");
    m_selectAll->setCursor(Qt::PointingHandCursor);
    m_selectAll->setStyleSheet(R"(
        QPushButton {
            background-color: #4A90E2;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #357ABD;
        }
        QPushButton:pressed {
            background-color: #2A6AAF;
        }
    )");
    connect(m_selectAll, &QPushButton::clicked, this, &ScheduleListDialog::onSelectAll);
    batchBarLayout->addWidget(m_selectAll);
    
    // 取消全选按钮
    m_deselectAll = new QPushButton("☐ 取消全选");
    m_deselectAll->setCursor(Qt::PointingHandCursor);
    m_deselectAll->setStyleSheet(R"(
        QPushButton {
            background-color: #F0F2F5;
            color: #333333;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #E0E2E5;
        }
        QPushButton:pressed {
            background-color: #D0D2D5;
        }
    )");
    connect(m_deselectAll, &QPushButton::clicked, this, &ScheduleListDialog::onDeselectAll);
    batchBarLayout->addWidget(m_deselectAll);
    
    batchBarLayout->addStretch();
    
    // 选中数量标签
    m_selectionCountLabel = new QLabel("已选择 0 项");
    m_selectionCountLabel->setStyleSheet("font-size: 13px; color: #666666; font-weight: 500;");
    batchBarLayout->addWidget(m_selectionCountLabel);
    
    // 一键删除所有日程按钮
    QPushButton* deleteAll = new QPushButton("🗑 清空所有");
    deleteAll->setCursor(Qt::PointingHandCursor);
    deleteAll->setStyleSheet(R"(
        QPushButton {
            background-color: #6C757D;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #5A6268;
        }
        QPushButton:pressed {
            background-color: #4A5056;
        }
    )");
    connect(deleteAll, &QPushButton::clicked, this, &ScheduleListDialog::onDeleteAll);
    batchBarLayout->addWidget(deleteAll);
    
    // 批量删除按钮
    m_batchDelete = new QPushButton("🗑 批量删除");
    m_batchDelete->setCursor(Qt::PointingHandCursor);
    m_batchDelete->setEnabled(false);
    m_batchDelete->setStyleSheet(R"(
        QPushButton {
            background-color: #DC3545;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover:enabled {
            background-color: #C82333;
        }
        QPushButton:pressed:enabled {
            background-color: #BD2130;
        }
        QPushButton:disabled {
            background-color: #E0E0E0;
            color: #999999;
        }
    )");
    connect(m_batchDelete, &QPushButton::clicked, this, &ScheduleListDialog::onBatchDelete);
    batchBarLayout->addWidget(m_batchDelete);
    
    m_mainLayout->addWidget(m_batchActionBar);
    
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
    
    // 创建加载覆盖层
    m_loadingOverlay = new QWidget(this);
    m_loadingOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.5);");
    m_loadingOverlay->hide();
    
    QVBoxLayout* loadingLayout = new QVBoxLayout(m_loadingOverlay);
    loadingLayout->setAlignment(Qt::AlignCenter);
    
    m_loadingLabel = new QLabel("正在删除日程...");
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setStyleSheet(R"(
        QLabel {
            background-color: white;
            color: #333333;
            border-radius: 8px;
            padding: 20px 40px;
            font-size: 14px;
            font-weight: 500;
        }
    )");
    loadingLayout->addWidget(m_loadingLabel);
    
    setLayout(m_mainLayout);
}

void ScheduleListDialog::loadSchedules()
{
    // 先清空现有内容
    clearLayout(m_contentLayout);
    
    // 清空日程项映射
    m_scheduleItemWidgets.clear();
    
    QVector<Schedule> allSchedules = dbManager->getSchedules();
    
    // 排序：所有日程按日期升序排列
    std::sort(allSchedules.begin(), allSchedules.end(),
        [](const Schedule& a, const Schedule& b) {
            return a.datetime < b.datetime;
        });
    
    // 用于记录已显示的批量日程（避免重复显示）
    QSet<int> displayedBatchIds;
    
    // 添加日程项，批量日程合并显示
    for (const Schedule& schedule : allSchedules) {
        // 如果是批量日程且已经显示过（同一批次的其他日期），则跳过
        if (schedule.isBatch && schedule.id != -1) {
            // 使用一个唯一标识符来识别同一批次的日程
            // 由于同一批次的所有日程都有相同的isBatch=true和相似的datetime格式
            // 我们通过title和endDatetime来判断是否是同一批次
            
            bool alreadyDisplayed = false;
            for (int displayedId : displayedBatchIds) {
                Schedule displayedSchedule = dbManager->getScheduleById(displayedId);
                if (displayedSchedule.id != -1 && 
                    displayedSchedule.title == schedule.title &&
                    displayedSchedule.endDatetime == schedule.endDatetime &&
                    displayedSchedule.priority == schedule.priority) {
                    alreadyDisplayed = true;
                    break;
                }
            }
            
            if (alreadyDisplayed) {
                continue; // 跳过已显示的批量日程条目
            }
            
            // 标记这个批次为已显示
            displayedBatchIds.insert(schedule.id);
        }
        
        QWidget* itemWidget = createScheduleItem(schedule);
        m_contentLayout->addWidget(itemWidget);
        
        // 存储日程项映射
        m_scheduleItemWidgets[schedule.id] = itemWidget;
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
    
    // 根据是否为批量日程设置不同的样式
    QString cardStyle = schedule.isBatch ? R"(
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
            border: 2px solid #4A90E2;
        }
        QFrame#cardFrame:hover {
            border-color: #357ABD;
            background-color: #FAFBFF;
        }
    )" : R"(
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
    )";
    
    itemWidget->setStyleSheet(cardStyle);
    
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
    
    // 左侧：复选框 + 日期和优先级标识
    QHBoxLayout* leftLayout = new QHBoxLayout();
    leftLayout->setSpacing(12);
    
    // 复选框
    QCheckBox* checkbox = new QCheckBox();
    checkbox->setFixedWidth(26);
    checkbox->setChecked(m_selectedScheduleIds.contains(schedule.id));
    checkbox->setStyleSheet(R"(
        QCheckBox {
            spacing: 0px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 4px;
            border: 2px solid #CCCCCC;
            background-color: white;
        }
        QCheckBox::indicator:checked {
            background-color: #4A90E2;
            border-color: #4A90E2;
            image: url(data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIxNiIgaGVpZ2h0PSIxNiIgdmlld0JveD0iMCAwIDI0IDI0IiBmaWxsPSJ3aGl0ZSI+PHBhdGggZD0iTTEwIDZMOC41NSA0LjVMOSA0bC0xLjUgMS41TDExIDE0bDIuNS0yLjVMMTQgMTIuNUwxNSAxM0w4LjUgNS40eiIvPjwvc3ZnPg==);
        }
        QCheckBox::indicator:hover {
            border-color: #4A90E2;
        }
    )");
    
    // 连接复选框信号
    int scheduleId = schedule.id;
    connect(checkbox, &QCheckBox::toggled, this, [this, scheduleId, checkbox](bool checked) {
        toggleScheduleSelection(scheduleId, checked);
        
        // 更新视觉效果
        QWidget* itemWidget = checkbox->window();
        if (checked) {
            itemWidget->setStyleSheet(R"(
                QWidget#scheduleCard {
                    background-color: #E8F4FD;
                    border: 2px solid #4A90E2;
                    border-radius: 12px;
                }
                QWidget#scheduleCard:hover {
                    background-color: #E8F4FD;
                }
            )");
        } else {
            itemWidget->setStyleSheet(R"(
                QWidget#scheduleCard {
                    background-color: #FFFFFF;
                    border: none;
                    border-radius: 12px;
                }
                QWidget#scheduleCard:hover {
                    background-color: #FFFFFF;
                }
            )");
        }
    });
    
    leftLayout->addWidget(checkbox);
    
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
    QString dateText;
    
    if (schedule.isBatch && schedule.endDatetime.isValid() && schedule.endDatetime > schedule.datetime) {
        // 批量添加的日程：显示完整日期范围（更清晰）
        dateText = QString("%1/%2 - %3/%4 (%5天)")
            .arg(scheduleDate.month(), 2, 10, QChar('0'))
            .arg(scheduleDate.day(), 2, 10, QChar('0'))
            .arg(schedule.endDatetime.date().month(), 2, 10, QChar('0'))
            .arg(schedule.endDatetime.date().day(), 2, 10, QChar('0'))
            .arg(scheduleDate.daysTo(schedule.endDatetime.date()) + 1);
    } else {
        // 普通日程：显示单个日期
        dateText = QString("%1 月 %2 日").arg(scheduleDate.month()).arg(scheduleDate.day());
    }
    
    QLabel* dateLabel = new QLabel(dateText);
    dateLabel->setStyleSheet("font-size: 13px; color: #999999; font-weight: 500;");
    dateLabel->setFixedWidth(schedule.isBatch ? 150 : 70);
    leftLayout->addWidget(dateLabel);
    
    // 如果是批量日程，添加一个醒目的标签标识
    if (schedule.isBatch) {
        QLabel* batchLabel = new QLabel("📅 批量");
        batchLabel->setStyleSheet("font-size: 12px; color: #FFFFFF; background-color: #4A90E2; padding: 4px 10px; border-radius: 6px; font-weight: bold;");
        leftLayout->addWidget(batchLabel);
    }
    
    topLayout->addLayout(leftLayout);
    
    topLayout->addStretch();
    
    // 右侧：操作按钮组
    QHBoxLayout* GroupLayout = new QHBoxLayout();
    GroupLayout->setSpacing(8);
    
    // 最小化/展开按钮
    QPushButton* minimize = new QPushButton();
    minimize->setCursor(Qt::PointingHandCursor);
    minimize->setFixedSize(28, 28);
    minimize->setToolTip(schedule.datetime.toString("yyyy-MM-dd HH:mm"));
    minimize->setStyleSheet(R"(
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
    bool isMinimized = m_minimizedState.value(scheduleId, false);
    
    // 创建动画
    QPropertyAnimation* animation = new QPropertyAnimation(cardFrame, "minimumHeight");
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    m_animations[scheduleId] = animation;
    
    connect(minimize, &QPushButton::clicked, this, [this, scheduleId, itemWidget, animation, minimize]() {
        bool minimized = m_minimizedState.value(scheduleId, false);
        
        // 切换状态
        minimized = !minimized;
        m_minimizedState[scheduleId] = minimized;
        
        // 更新按钮图标
        minimize->setText(minimized ? "📋" : "📝");
        
        // 执行动画
        animation->stop();
        animation->setStartValue(minimized ? itemWidget->height() : 80);
        animation->setEndValue(minimized ? 80 : itemWidget->height());
        animation->start();
        
        // 更新可见性
        updateScheduleItemVisibility(itemWidget, minimized);
    });
    
    // 初始化按钮状态
    minimize->setText(isMinimized ? "📋" : "📝");
    GroupLayout->addWidget(minimize);
    
    // 编辑按钮
    QPushButton* editBtn = new QPushButton("编辑");
    editBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setFixedHeight(28);
    editBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    editBtn->setStyleSheet(R"(
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
    connect(editBtn, &QPushButton::clicked, this, [this, schedule]() {
        onEditSchedule(schedule.id);
    });
    GroupLayout->addWidget(editBtn);
    
    // 删除按钮
    QPushButton* deleteBtn = new QPushButton("删除");
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setFixedHeight(28);
    deleteBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    deleteBtn->setStyleSheet(R"(
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
    connect(deleteBtn, &QPushButton::clicked, this, [this, schedule]() {
        onDeleteSchedule(schedule.id);
    });
    GroupLayout->addWidget(deleteBtn);
    
    topLayout->addLayout(GroupLayout);
    cardLayout->addLayout(topLayout);
    
    // 中间区域：日程标题（始终显示）
    QString titleText = schedule.title;
    if (schedule.isBatch && schedule.endDatetime.isValid() && schedule.endDatetime > schedule.datetime) {
        // 批量日程标题添加日期范围信息
        titleText += QString(" (%1-%2)")
            .arg(schedule.datetime.date().toString("yyyy/MM/dd"))
            .arg(schedule.endDatetime.date().toString("yyyy/MM/dd"));
    }
    
    QLabel* titleLabel = new QLabel(titleText);
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

void ScheduleListDialog::toggleScheduleSelection(int scheduleId, bool selected)
{
    if (selected) {
        m_selectedScheduleIds.insert(scheduleId);
    } else {
        m_selectedScheduleIds.remove(scheduleId);
    }
    updateBatchDelete();
}

void ScheduleListDialog::updateBatchDelete()
{
    int count = m_selectedScheduleIds.size();
    m_selectionCountLabel->setText(QString("已选择 %1 项").arg(count));
    m_batchDelete->setEnabled(count > 0);
    
    if (count > 0) {
        m_batchDelete->setText(QString("🗑 批量删除 (%1)").arg(count));
    } else {
        m_batchDelete->setText("🗑 批量删除");
    }
}

void ScheduleListDialog::onSelectAll()
{
    // 获取所有日程ID
    QVector<Schedule> allSchedules = dbManager->getSchedules();
    for (const Schedule& schedule : allSchedules) {
        m_selectedScheduleIds.insert(schedule.id);
    }
    updateBatchDelete();
    loadSchedules();  // 重新加载以更新UI
}

void ScheduleListDialog::onDeselectAll()
{
    m_selectedScheduleIds.clear();
    updateBatchDelete();
    loadSchedules();  // 重新加载以更新UI
}

void ScheduleListDialog::onBatchDelete()
{
    if (m_selectedScheduleIds.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的日程");
        return;
    }
    
    // 获取删除预览信息
    QVector<int> ids;
    for (int id : m_selectedScheduleIds) {
        ids.append(id);
    }
    
    ::DeleteResult preview = dbManager->getDeletePreview(ids);
    
    if (preview.totalSchedules == 0) {
        QMessageBox::warning(this, "错误", "无法获取删除预览信息，请重试");
        return;
    }
    
    // 构建详细的确认信息
    QString message = "确定要删除以下日程吗？\n\n";
    message += QString("📊 共计: %1 个日程\n").arg(preview.totalSchedules);
    message += QString("📅 批量日程: %1 批（共 %2 天）\n").arg(preview.batchCount).arg(preview.batchDaysCount);
    message += QString("📝 普通日程: %1 个\n\n").arg(preview.scheduleCount);
    
    // 显示每个批量日程的详细信息
    if (!preview.batchDetails.isEmpty()) {
        message += "批量日程详情：\n";
        for (const QString& detail : preview.batchDetails) {
            message += "  • " + detail + "\n";
        }
        message += "\n";
    }
    
    message += "⚠️ 此操作不可撤销！";
    
    // 显示确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        message,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // 显示加载状态
    showLoadingState(true);
    m_loadingLabel->setText(QString("正在删除 %1 个日程...").arg(preview.totalSchedules));
    
    // 延迟一小段时间以确保UI更新
    QTimer::singleShot(100, this, [this, ids, preview]() {
        // 执行删除并获取详细信息
        ::DeleteResult result = dbManager->deleteSchedulesWithDetails(ids);
        
        // 隐藏加载状态
        showLoadingState(false);
        
        // 显示结果反馈
        if (result.actualDeleted > 0) {
            QString successMessage = QString("✅ 已成功删除 %1 个日程\n\n").arg(result.actualDeleted);
            successMessage += QString("批量日程: %1 批\n").arg(result.batchCount);
            successMessage += QString("普通日程: %1 个").arg(result.scheduleCount);
            
            QMessageBox::information(this, "删除成功", successMessage);
        } else {
            QMessageBox::warning(this, "删除失败", "删除操作未能成功执行，请稍后重试。");
        }
        
        // 清空选择
        m_selectedScheduleIds.clear();
        updateBatchDelete();
        
        // 刷新列表
        loadSchedules();
    });
}

void ScheduleListDialog::onDeleteAll()
{
    // 获取所有日程数量
    QVector<Schedule> allSchedules = dbManager->getSchedules();
    if (allSchedules.isEmpty()) {
        QMessageBox::information(this, "提示", "当前没有日程可删除");
        return;
    }
    
    // 获取删除预览信息
    QVector<int> allIds;
    for (const Schedule& schedule : allSchedules) {
        allIds.append(schedule.id);
    }
    
    ::DeleteResult preview = dbManager->getDeletePreview(allIds);
    
    // 构建详细的确认信息
    QString message = "⚠️ 确定要清空所有日程吗？\n\n";
    message += QString("📊 共计: %1 个日程\n").arg(preview.totalSchedules);
    message += QString("📅 批量日程: %1 批（共 %2 天）\n").arg(preview.batchCount).arg(preview.batchDaysCount);
    message += QString("📝 普通日程: %1 个\n\n").arg(preview.scheduleCount);
    
    // 显示每个批量日程的详细信息
    if (!preview.batchDetails.isEmpty()) {
        message += "批量日程详情：\n";
        for (const QString& detail : preview.batchDetails) {
            message += "  • " + detail + "\n";
        }
        message += "\n";
    }
    
    message += "⚠️ 此操作不可撤销，将删除所有日程！";
    
    // 确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认清空",
        message,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // 显示加载状态
    showLoadingState(true);
    m_loadingLabel->setText(QString("正在清空 %1 个日程...").arg(preview.totalSchedules));
    
    // 延迟执行删除
    QTimer::singleShot(100, this, [this, preview]() {
        // 执行删除所有日程
        bool success = dbManager->deleteAllSchedules();
        
        // 隐藏加载状态
        showLoadingState(false);
        
        // 显示结果反馈
        if (success) {
            QString successMessage = QString("✅ 已成功清空所有 %1 个日程\n\n").arg(preview.totalSchedules);
            successMessage += QString("批量日程: %1 批\n").arg(preview.batchCount);
            successMessage += QString("普通日程: %1 个").arg(preview.scheduleCount);
            
            QMessageBox::information(this, "清空成功", successMessage);
        } else {
            QMessageBox::warning(
                this,
                "清空失败",
                "清空日程失败，请稍后重试。"
            );
        }
        
        // 清空选择
        m_selectedScheduleIds.clear();
        updateBatchDelete();
        
        // 刷新列表
        loadSchedules();
    });
}

void ScheduleListDialog::showLoadingState(bool show)
{
    if (show) {
        m_loadingOverlay->setGeometry(0, 0, this->width(), this->height());
        m_loadingOverlay->raise();
        m_loadingOverlay->show();
        m_loadingLabel->setText("正在删除日程...");
        this->setDisabled(true);
    } else {
        m_loadingOverlay->hide();
        this->setEnabled(true);
    }
}

void ScheduleListDialog::showDeleteFeedback(bool success, int count)
{
    if (success) {
        QMessageBox::information(
            this,
            "删除成功",
            QString("已成功删除 %1 项日程！").arg(count)
        );
    } else {
        QMessageBox::warning(
            this,
            "部分删除失败",
            QString("成功删除 %1 项日程，但有部分日程删除失败。\n请稍后重试。").arg(count)
        );
    }
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
    // 先获取日程信息，检查是否为批量日程
    Schedule schedule = dbManager->getScheduleById(scheduleId);
    
    QString message = "确定要删除这个日程吗？";
    if (schedule.isBatch) {
        // 计算该批次包含的天数
        int days = schedule.datetime.date().daysTo(schedule.endDatetime.date()) + 1;
        message = QString("这是一个批量日程（%1天），删除将删除整个批次的所有日程。\n\n确定要继续吗？").arg(days);
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        message,
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (dbManager->deleteSingleSchedule(scheduleId)) {
            refreshScheduleList();
            emit accepted();  // 通知主窗口刷新
            
            // 显示成功提示
            if (schedule.isBatch) {
                QMessageBox::information(this, "删除成功", "批量日程已成功删除！");
            }
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
