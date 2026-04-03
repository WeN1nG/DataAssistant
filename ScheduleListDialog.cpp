#include "ScheduleListDialog.h"
#include "ScheduleDialog.h"
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>

ScheduleListDialog::ScheduleListDialog(QWidget* parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_contentWidget(nullptr)
    , m_contentLayout(nullptr)
    , dbManager(new DatabaseManager())
{
    setWindowTitle("日程表");
    setMinimumSize(600, 500);
    setModal(true);
    
    setupUI();
    loadSchedules();
}

void ScheduleListDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: white; border: none; }");
    
    // 创建内容容器
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setSpacing(10);
    m_contentLayout->setContentsMargins(15, 15, 15, 15);
    m_contentLayout->addStretch();
    
    scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(scrollArea);
    
    setLayout(m_mainLayout);
}

void ScheduleListDialog::loadSchedules()
{
    // 先清空现有内容
    clearLayout(m_contentLayout);
    
    QVector<Schedule> allSchedules = dbManager->getSchedules();
    
    // 分离未来日程和已过期日程
    QVector<Schedule> futureSchedules;
    QVector<Schedule> pastSchedules;
    QDate today = QDate::currentDate();
    
    for (const Schedule& schedule : allSchedules) {
        if (schedule.datetime.date() >= today) {
            futureSchedules.append(schedule);
        } else {
            pastSchedules.append(schedule);
        }
    }
    
    // 排序：未来日程按日期升序，已过期日程按日期降序
    std::sort(futureSchedules.begin(), futureSchedules.end(),
        [](const Schedule& a, const Schedule& b) {
            return a.datetime < b.datetime;
        });
    
    std::sort(pastSchedules.begin(), pastSchedules.end(),
        [](const Schedule& a, const Schedule& b) {
            return a.datetime > b.datetime;
        });
    
    // 添加标题 - 未来日程
    if (!futureSchedules.isEmpty()) {
        QLabel* futureTitle = new QLabel("📅 未来日程", this);
        futureTitle->setFont(QFont("Microsoft YaHei", 13, QFont::Bold));
        futureTitle->setStyleSheet("color: #2c3e50; padding: 10px 5px;");
        m_contentLayout->addWidget(futureTitle);
        
        // 添加未来日程项
        for (const Schedule& schedule : futureSchedules) {
            QWidget* itemWidget = createScheduleItem(schedule);
            m_contentLayout->addWidget(itemWidget);
        }
    }
    
    // 添加标题 - 已过期日程
    if (!pastSchedules.isEmpty()) {
        QLabel* pastTitle = new QLabel("🕒 已过期日程", this);
        pastTitle->setFont(QFont("Microsoft YaHei", 13, QFont::Bold));
        pastTitle->setStyleSheet("color: #95a5a6; padding: 10px 5px;");
        m_contentLayout->addWidget(pastTitle);
        
        // 添加已过期日程项
        for (const Schedule& schedule : pastSchedules) {
            QWidget* itemWidget = createScheduleItem(schedule);
            m_contentLayout->addWidget(itemWidget);
        }
    }
    
    // 如果没有任何日程
    if (allSchedules.isEmpty()) {
        QLabel* emptyLabel = new QLabel("📭 暂无日程安排", this);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setFont(QFont("Microsoft YaHei", 14));
        emptyLabel->setStyleSheet("color: #95a5a6; padding: 50px 0;");
        m_contentLayout->addWidget(emptyLabel);
    }
}

QWidget* ScheduleListDialog::createScheduleItem(const Schedule& schedule)
{
    // 创建日程项容器
    QWidget* itemWidget = new QWidget();
    itemWidget->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            padding: 12px;
            margin: 2px 0;
        }
        QWidget:hover {
            background-color: #f8f9fa;
            border-color: #4CAF50;
        }
    )");
    
    QVBoxLayout* itemLayout = new QVBoxLayout(itemWidget);
    itemLayout->setSpacing(8);
    itemLayout->setContentsMargins(12, 12, 12, 12);
    
    // 判断是否已过期
    QDate today = QDate::currentDate();
    bool isPast = schedule.datetime.date() < today;
    
    // 标题和优先级标识
    QHBoxLayout* titleLayout = new QHBoxLayout();
    
    // 优先级标识
    QLabel* priorityIndicator = new QLabel("●");
    priorityIndicator->setFont(QFont("Arial", 16));
    if (schedule.priority == 2) {
        priorityIndicator->setStyleSheet("color: #e74c3c;");  // 紧急：红色
    } else if (schedule.priority == 1) {
        priorityIndicator->setStyleSheet("color: #f39c12;");  // 重要：橙色
    } else {
        priorityIndicator->setStyleSheet("color: #3498db;");  // 一般：蓝色
    }
    titleLayout->addWidget(priorityIndicator);
    
    // 标题
    QLabel* titleLabel = new QLabel(schedule.title);
    QString titleStyle = isPast ? 
        "font-size: 15px; font-weight: bold; color: #95a5a6;" :
        "font-size: 15px; font-weight: bold; color: #2c3e50;";
    titleLabel->setStyleSheet(titleStyle);
    titleLayout->addWidget(titleLabel);
    
    titleLayout->addStretch();
    itemLayout->addLayout(titleLayout);
    
    // 描述
    if (!schedule.description.isEmpty() && schedule.description != schedule.title) {
        QLabel* descLabel = new QLabel(schedule.description);
        QString descStyle = isPast ?
            "font-size: 13px; color: #95a5a6; margin-left: 24px;" :
            "font-size: 13px; color: #666; margin-left: 24px;";
        descLabel->setStyleSheet(descStyle);
        descLabel->setWordWrap(true);
        itemLayout->addWidget(descLabel);
    }
    
    // 日期
    QLabel* dateLabel = new QLabel(schedule.datetime.toString("yyyy-MM-dd"));
    QString dateStyle = isPast ?
        "font-size: 12px; color: #95a5a6; margin-left: 24px;" :
        "font-size: 12px; color: #4CAF50; margin-left: 24px;";
    dateLabel->setStyleSheet(dateStyle);
    itemLayout->addWidget(dateLabel);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(24, 8, 12, 0);
    
    buttonLayout->addStretch();
    
    // 编辑按钮
    QPushButton* editButton = new QPushButton("编辑");
    editButton->setFixedSize(70, 28);
    editButton->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
    )");
    connect(editButton, &QPushButton::clicked, this, [this, schedule]() {
        onEditSchedule(schedule.id);
    });
    buttonLayout->addWidget(editButton);
    
    // 删除按钮
    QPushButton* deleteButton = new QPushButton("删除");
    deleteButton->setFixedSize(70, 28);
    deleteButton->setStyleSheet(R"(
        QPushButton {
            background-color: #e74c3c;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #c0392b;
        }
        QPushButton:pressed {
            background-color: #a93226;
        }
    )");
    connect(deleteButton, &QPushButton::clicked, this, [this, schedule]() {
        onDeleteSchedule(schedule.id);
    });
    buttonLayout->addWidget(deleteButton);
    
    itemLayout->addLayout(buttonLayout);
    
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
            QMessageBox::information(this, "成功", "日程已删除");
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
