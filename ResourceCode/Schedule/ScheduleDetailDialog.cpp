#include "ScheduleDetailDialog.h"
#include "ScheduleDialog.h"
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>
#include <QApplication>
#include <QMouseEvent>
#include <QCloseEvent>

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
    , m_closeAnimation(nullptr)
    , m_isClosing(false)
    , m_parentWidget(parent)
{
    setWindowTitle("日程详情");
    setMinimumWidth(450);
    setMinimumHeight(350);
    setModal(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    setupUI();
    showScheduleDetails();

    if (m_parentWidget) {
        m_parentWidget->installEventFilter(this);
    }

    setStyleSheet(R"(
        ScheduleDetailDialog {
            background-color: #f5f5f5;
        }
    )");
}

ScheduleDetailDialog::~ScheduleDetailDialog()
{
    if (m_parentWidget) {
        m_parentWidget->removeEventFilter(this);
    }
    if (m_closeAnimation) {
        m_closeAnimation->stop();
        delete m_closeAnimation;
        m_closeAnimation = nullptr;
    }
}

bool ScheduleDetailDialog::eventFilter(QObject* watched, QEvent* event)
{
    if (m_isClosing) {
        return QDialog::eventFilter(watched, event);
    }

    if (!m_parentWidget.isNull() && watched == m_parentWidget) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QPointF globalPosF = mouseEvent->globalPosition();
            QPoint dialogPos = mapFromGlobal(globalPosF.toPoint());

            if (!rect().contains(dialogPos)) {
                animateClose();
                return true;
            }
        }
    }

    return QDialog::eventFilter(watched, event);
}

void ScheduleDetailDialog::closeEvent(QCloseEvent* event)
{
    if (!m_isClosing) {
        event->ignore();
        animateClose();
    } else {
        resetState();
        event->accept();
    }
}

void ScheduleDetailDialog::animateClose()
{
    if (m_isClosing) {
        return;
    }
    m_isClosing = true;

    if (m_closeAnimation) {
        m_closeAnimation->stop();
        delete m_closeAnimation;
        m_closeAnimation = nullptr;
    }

    m_closeAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_closeAnimation->setDuration(200);
    m_closeAnimation->setStartValue(1.0);
    m_closeAnimation->setEndValue(0.0);
    m_closeAnimation->setEasingCurve(QEasingCurve::InCurve);

    connect(m_closeAnimation, &QPropertyAnimation::finished, this, &ScheduleDetailDialog::performClose, Qt::UniqueConnection);

    m_closeAnimation->start();
}

void ScheduleDetailDialog::performClose()
{
    if (!m_isClosing) {
        return;
    }
    resetState();
    done(QDialog::Accepted);
}

void ScheduleDetailDialog::resetState()
{
    m_schedules.clear();
    m_isClosing = false;
}

void ScheduleDetailDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    m_titleLabel->setText(m_date.toString("yyyy 年 MM 月 dd 日"));
    m_mainLayout->addWidget(m_titleLabel);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");

    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setSpacing(10);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    
    // 关键修复：设置内容 widget 的尺寸策略，确保单一日程不会被拉高
    // 水平方向 Preferred（可伸展），垂直方向 Minimum（只占用内容所需高度）
    m_contentWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_contentWidget->setMinimumHeight(0);  // 允许高度收缩到 0

    scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(scrollArea);

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

    connect(m_closeButton, &QPushButton::clicked, this, &ScheduleDetailDialog::animateClose);
    m_mainLayout->addWidget(m_closeButton);

    setLayout(m_mainLayout);
}

void ScheduleDetailDialog::showScheduleDetails()
{
    QLayoutItem* item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    if (m_schedules.isEmpty()) {
        QLabel* emptyLabel = new QLabel("📭 当天无日程安排", this);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setFont(QFont("Microsoft YaHei", 14));
        emptyLabel->setStyleSheet("color: #999; padding: 50px 0;");
        m_contentLayout->addWidget(emptyLabel);
    } else {
        for (int i = 0; i < m_schedules.size(); ++i) {
            const Schedule& schedule = m_schedules[i];

            // 统一格式：外层容器
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

            // 顶部区域：优先级 + 日期 + 操作按钮
            QHBoxLayout* topLayout = new QHBoxLayout();
            topLayout->setSpacing(12);

            // 左侧：优先级和日期
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

            int scheduleId = schedule.id;

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
            connect(editButton, &QPushButton::clicked, this, [this, scheduleId]() {
                onEditSchedule(scheduleId);
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
            connect(deleteButton, &QPushButton::clicked, this, [this, scheduleId]() {
                onDeleteSchedule(scheduleId);
            });
            buttonGroupLayout->addWidget(deleteButton);

            topLayout->addLayout(buttonGroupLayout);
            cardLayout->addLayout(topLayout);

            // 中间区域：日程标题
            QLabel* titleLabel = new QLabel(schedule.title);
            titleLabel->setObjectName("titleLabel");
            titleLabel->setStyleSheet("font-size: 16px; color: #333333; font-weight: 600;");
            titleLabel->setWordWrap(true);
            titleLabel->setContentsMargins(26, 0, 0, 0);
            cardLayout->addWidget(titleLabel);

            // 底部区域：日程详情（描述）
            if (!schedule.description.isEmpty() && schedule.description != schedule.title) {
                QLabel* descLabel = new QLabel(schedule.description);
                descLabel->setObjectName("descLabel");
                descLabel->setStyleSheet("font-size: 13px; color: #666666; line-height: 1.6;");
                descLabel->setWordWrap(true);
                descLabel->setContentsMargins(26, 0, 0, 0);
                cardLayout->addWidget(descLabel);
            }

            itemLayout->addWidget(cardFrame);

            m_contentLayout->addWidget(itemWidget);
        }
    }
    
    // 关键修复：添加可伸缩 spacer，确保日程项不会被 QVBoxLayout 拉高
    m_contentLayout->addStretch(1);
    
    // 强制更新布局
    m_contentWidget->adjustSize();
}

void ScheduleDetailDialog::onEditSchedule(int scheduleId)
{
    if (!dbManager) {
        return;
    }
    Schedule schedule = dbManager->getScheduleById(scheduleId);

    ScheduleDialog* dialog = new ScheduleDialog(this, &schedule);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &ScheduleDialog::scheduleSaved, this, [this, dialog]() {
        QMessageBox::information(this, "成功", "日程已更新");
        refreshSchedules();
        dialog->close();
    });
    dialog->show();
}

void ScheduleDetailDialog::onDeleteSchedule(int scheduleId)
{
    if (!dbManager) {
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除这个日程吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (dbManager->deleteSingleSchedule(scheduleId)) {
            refreshSchedules();
            emit accepted();
        } else {
            QMessageBox::warning(this, "错误", "删除失败");
        }
    }
}

void ScheduleDetailDialog::refreshSchedules()
{
    if (!dbManager) {
        return;
    }
    m_schedules = dbManager->getSchedulesByDate(m_date);
    showScheduleDetails();
}
