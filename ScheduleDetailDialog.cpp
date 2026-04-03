#include "ScheduleDetailDialog.h"
#include "ScheduleDialog.h"
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>

ScheduleDetailDialog::ScheduleDetailDialog(const QDate& date, const QVector<Schedule>& schedules, QWidget* parent)
    : QDialog(parent)
    , m_date(date)
    , m_schedules(schedules)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_contentWidget(nullptr)
    , m_contentLayout(nullptr)
    , m_closeButton(nullptr)
    , dbManager(new DatabaseManager())
{
    setWindowTitle("日程详情");
    setMinimumWidth(450);
    setMinimumHeight(350);
    setModal(true);
    
    setupUI();
    showScheduleDetails();
}

void ScheduleDetailDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 标题标签
    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    m_titleLabel->setText(m_date.toString("yyyy 年 MM 月 dd 日"));
    m_mainLayout->addWidget(m_titleLabel);
    
    // 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");
    
    // 创建内容容器
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setSpacing(10);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    
    scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(scrollArea);
    
    // 关闭按钮
    m_closeButton = new QPushButton("关闭", this);
    m_closeButton->setFixedHeight(35);
    m_closeButton->setFont(QFont("Microsoft YaHei", 11));
    m_closeButton->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
    )");
    
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    m_mainLayout->addWidget(m_closeButton);
    
    setLayout(m_mainLayout);
}

void ScheduleDetailDialog::showScheduleDetails()
{
    // 清空现有内容
    QLayoutItem* item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    
    if (m_schedules.isEmpty()) {
        // 无日程时显示提示信息
        QLabel* emptyLabel = new QLabel("📭 当天无日程安排", this);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setFont(QFont("Microsoft YaHei", 14));
        emptyLabel->setStyleSheet("color: #999; padding: 50px 0;");
        m_contentLayout->addWidget(emptyLabel);
    } else {
        // 有日程时显示所有日程详情
        for (int i = 0; i < m_schedules.size(); ++i) {
            const Schedule& schedule = m_schedules[i];
            
            // 创建日程卡片容器
            QWidget* cardWidget = new QWidget();
            cardWidget->setStyleSheet(R"(
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
            
            QVBoxLayout* cardLayout = new QVBoxLayout(cardWidget);
            cardLayout->setSpacing(8);
            cardLayout->setContentsMargins(12, 12, 12, 12);
            
            // 标题和优先级
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
            QLabel* titleLabel = new QLabel(QString("%1. %2").arg(i + 1).arg(schedule.title));
            titleLabel->setFont(QFont("Microsoft YaHei", 13, QFont::Bold));
            titleLabel->setStyleSheet("color: #2c3e50;");
            titleLayout->addWidget(titleLabel);
            
            titleLayout->addStretch();
            cardLayout->addLayout(titleLayout);
            
            // 描述
            if (!schedule.description.isEmpty() && schedule.description != schedule.title) {
                QLabel* descLabel = new QLabel("📝 " + schedule.description);
                descLabel->setWordWrap(true);
                descLabel->setStyleSheet("font-size: 13px; color: #666; margin-left: 24px;");
                cardLayout->addWidget(descLabel);
            }
            
            // 日期时间
            QLabel* dateTimeLabel = new QLabel("📅 " + schedule.datetime.toString("yyyy-MM-dd"));
            dateTimeLabel->setStyleSheet("font-size: 12px; color: #4CAF50; margin-left: 24px;");
            cardLayout->addWidget(dateTimeLabel);
            
            // 优先级标签
            QString priorityText;
            if (schedule.priority == 2) {
                priorityText = "🔴 紧急";
            } else if (schedule.priority == 1) {
                priorityText = "🟠 重要";
            } else {
                priorityText = "🔵 一般";
            }
            QLabel* priorityLabel = new QLabel(priorityText);
            priorityLabel->setStyleSheet("font-size: 12px; color: #888; margin-left: 24px;");
            cardLayout->addWidget(priorityLabel);
            
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
            
            cardLayout->addLayout(buttonLayout);
            
            m_contentLayout->addWidget(cardWidget);
        }
    }
}

void ScheduleDetailDialog::onEditSchedule(int scheduleId)
{
    Schedule schedule = dbManager->getScheduleById(scheduleId);
    
    ScheduleDialog dialog(this, &schedule);
    if (dialog.exec() == QDialog::Accepted) {
        QMessageBox::information(this, "成功", "日程已更新");
        refreshSchedules();
        emit accepted();  // 通知主窗口刷新
    }
}

void ScheduleDetailDialog::onDeleteSchedule(int scheduleId)
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
            refreshSchedules();
            emit accepted();  // 通知主窗口刷新
        } else {
            QMessageBox::warning(this, "错误", "删除失败");
        }
    }
}

void ScheduleDetailDialog::refreshSchedules()
{
    // 重新获取该日期的日程
    m_schedules = dbManager->getSchedulesByDate(m_date);
    showScheduleDetails();
}
