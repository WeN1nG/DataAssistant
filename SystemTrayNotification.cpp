#include "SystemTrayNotification.h"
#include <QGuiApplication>
#include <QApplication>

SystemTrayNotification::SystemTrayNotification(const Schedule &schedule, QWidget *parent)
    : QWidget(parent)
    , m_schedule(schedule)
    , m_posAnimation(nullptr)
    , m_opacityAnimation(nullptr)
    , m_autoCloseTimer(nullptr)
    , m_animationDuration(300)
    , m_autoCloseDuration(6000)
    , m_isVisible(false)
    , m_isClosing(false)
{
    setupUI();
    setupAnimations();
    positionNotification();

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

SystemTrayNotification::~SystemTrayNotification()
{
    m_isClosing = true;

    if (m_autoCloseTimer && m_autoCloseTimer->isActive()) {
        m_autoCloseTimer->stop();
    }

    if (m_posAnimation) {
        m_posAnimation->stop();
        m_posAnimation->deleteLater();
        m_posAnimation = nullptr;
    }

    if (m_opacityAnimation) {
        m_opacityAnimation->stop();
        m_opacityAnimation->deleteLater();
        m_opacityAnimation = nullptr;
    }
}

void SystemTrayNotification::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *contentWidget = new QWidget(this);
    contentWidget->setObjectName("notificationContent");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(16, 12, 12, 12);
    contentLayout->setSpacing(8);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(8);

    QLabel *iconLabel = new QLabel(contentWidget);
    iconLabel->setText("🔔");
    iconLabel->setStyleSheet("font-size: 18px;");
    m_titleLabel = new QLabel(contentWidget);
    m_titleLabel->setText("日程提醒");
    m_titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");

    m_closeButton = new QPushButton(contentWidget);
    m_closeButton->setText("×");
    m_closeButton->setFixedSize(24, 24);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    color: #999;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    color: #666;"
        "    background-color: rgba(0, 0, 0, 0.05);"
        "    border-radius: 4px;"
        "}"
    );
    connect(m_closeButton, &QPushButton::clicked, this, &SystemTrayNotification::onCloseButtonClicked);

    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_closeButton);

    m_timeLabel = new QLabel(contentWidget);
    m_timeLabel->setText(QString("时间：%1").arg(m_schedule.datetime.toString("yyyy-MM-dd HH:mm")));
    m_timeLabel->setStyleSheet("font-size: 12px; color: #666; margin-left: 26px;");

    m_descriptionLabel = new QLabel(contentWidget);
    QString descText = m_schedule.description.isEmpty() ? "无" : m_schedule.description;
    m_descriptionLabel->setText(QString("内容：%1").arg(descText));
    m_descriptionLabel->setStyleSheet(
        "font-size: 12px; color: #555; margin-left: 26px;"
        "max-height: 40px;"
    );
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QLabel *scheduleTitleLabel = new QLabel(contentWidget);
    scheduleTitleLabel->setText(QString("📋 %1").arg(m_schedule.title));
    scheduleTitleLabel->setStyleSheet(
        "font-size: 13px; color: #222; font-weight: bold;"
        "margin-left: 26px; margin-top: 4px;"
    );
    scheduleTitleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QLabel *hintLabel = new QLabel(contentWidget);
    hintLabel->setText("💡 点击查看详情");
    hintLabel->setStyleSheet(
        "font-size: 11px; color: #888; margin-left: 26px; margin-top: 6px;"
    );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    buttonLayout->addStretch();

    QPushButton *completeButton = new QPushButton(contentWidget);
    completeButton->setText("✓ 完成");
    completeButton->setFixedHeight(28);
    completeButton->setCursor(Qt::PointingHandCursor);
    completeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 4px 12px;"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45a049;"
        "}"
    );
    connect(completeButton, &QPushButton::clicked, this, [this]() {
        emit scheduleMarkedAsCompleted(m_schedule.id);
        hideNotification();
    });

    buttonLayout->addWidget(completeButton);

    contentLayout->addLayout(titleLayout);
    contentLayout->addWidget(scheduleTitleLabel);
    contentLayout->addWidget(m_timeLabel);
    contentLayout->addWidget(m_descriptionLabel);
    contentLayout->addWidget(hintLabel);
    contentLayout->addLayout(buttonLayout);

    mainLayout->addWidget(contentWidget);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 60));
    shadow->setOffset(0, 4);
    contentWidget->setGraphicsEffect(shadow);

    applyStyles();
}

void SystemTrayNotification::applyStyles()
{
    setStyleSheet(
        "#notificationContent {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    border: 1px solid rgba(0, 0, 0, 0.08);"
        "}"
    );
}

void SystemTrayNotification::setupAnimations()
{
    m_posAnimation = new QPropertyAnimation(this, "pos", this);
    m_posAnimation->setDuration(m_animationDuration);
    m_posAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_opacityAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_opacityAnimation->setDuration(m_animationDuration);
    m_opacityAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_autoCloseTimer = new QTimer(this);
    m_autoCloseTimer->setSingleShot(true);
    connect(m_autoCloseTimer, &QTimer::timeout, this, &SystemTrayNotification::onAutoCloseTimeout, Qt::UniqueConnection);
}

void SystemTrayNotification::positionNotification()
{
    if (!isScreenAvailable()) {
        qWarning() << "SystemTrayNotification: No screen available for notification";
        return;
    }

    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (!primaryScreen) {
        qWarning() << "SystemTrayNotification: Failed to get primary screen";
        return;
    }

    QRect screenGeometry = primaryScreen->availableGeometry();
    
    int widgetWidth = width();
    int widgetHeight = height();
    
    if (widgetWidth == 0 || widgetHeight == 0) {
        QCoreApplication::processEvents();
        widgetWidth = width();
        widgetHeight = height();
    }
    
    if (widgetWidth == 0) widgetWidth = 320;
    if (widgetHeight == 0) widgetHeight = 180;
    
    int x = screenGeometry.right() - widgetWidth - 20;
    int y = screenGeometry.bottom() - widgetHeight - 20;

    move(x, y);
}

bool SystemTrayNotification::isScreenAvailable() const
{
    QList<QScreen*> screens = QGuiApplication::screens();
    return !screens.isEmpty();
}

void SystemTrayNotification::showNotification()
{
    try {
        if (m_isClosing || m_isVisible) {
            return;
        }

        if (!isScreenAvailable()) {
            qWarning() << "Cannot show notification: no screen available";
            return;
        }

        slideInAnimation();
        positionNotification();

        m_autoCloseTimer->start(m_autoCloseDuration);
        m_isVisible = true;

    } catch (const std::exception &e) {
        qCritical() << "Exception in showNotification:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception in showNotification";
    }
}

void SystemTrayNotification::hideNotification()
{
    if (m_isClosing) {
        return;
    }

    try {
        slideOutAnimation();
    } catch (const std::exception &e) {
        qCritical() << "Exception in hideNotification:" << e.what();
        close();
    } catch (...) {
        qCritical() << "Unknown exception in hideNotification";
        close();
    }
}

void SystemTrayNotification::slideInAnimation()
{
    if (!m_posAnimation || !m_opacityAnimation || m_isClosing) {
        return;
    }

    m_posAnimation->stop();
    m_opacityAnimation->stop();

    show();
    raise();
    
    QCoreApplication::processEvents();
    adjustSize();
    QCoreApplication::processEvents();

    int widgetWidth = width();
    int widgetHeight = height();
    
    if (widgetWidth == 0) widgetWidth = 320;
    if (widgetHeight == 0) widgetHeight = 180;

    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    int screenBottom = primaryScreen ? primaryScreen->availableGeometry().bottom() : QApplication::primaryScreen()->availableGeometry().bottom();
    QPoint finalPos = pos();
    finalPos.setY(screenBottom - widgetHeight - 20);
    finalPos.setX(primaryScreen ? primaryScreen->availableGeometry().right() - widgetWidth - 20 : QApplication::primaryScreen()->availableGeometry().right() - widgetWidth - 20);

    QPoint startPos = finalPos;
    startPos.setY(startPos.y() + widgetHeight + 20);

    move(startPos);
    setWindowOpacity(0);

    m_posAnimation->setStartValue(startPos);
    m_posAnimation->setEndValue(finalPos);
    m_posAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_opacityAnimation->setStartValue(0);
    m_opacityAnimation->setEndValue(1);

    m_posAnimation->start();
    m_opacityAnimation->start();
}

void SystemTrayNotification::slideOutAnimation()
{
    if (!m_posAnimation || !m_opacityAnimation || m_isClosing) {
        close();
        return;
    }

    m_autoCloseTimer->stop();

    m_posAnimation->stop();
    m_opacityAnimation->stop();

    QPoint startPos = pos();
    QPoint endPos = startPos;
    endPos.setY(endPos.y() + height() + 20);

    m_posAnimation->setStartValue(startPos);
    m_posAnimation->setEndValue(endPos);
    m_posAnimation->setEasingCurve(QEasingCurve::InCubic);

    m_opacityAnimation->setStartValue(1);
    m_opacityAnimation->setEndValue(0);

    m_posAnimation->start();
    m_opacityAnimation->start();

    m_isClosing = true;

    QTimer::singleShot(m_animationDuration, this, [this]() {
        if (!this->isHidden()) {
            this->close();
        }
    });
}

void SystemTrayNotification::onCloseButtonClicked()
{
    if (m_isClosing) {
        return;
    }

    try {
        emit notificationClosed();
        hideNotification();
    } catch (const std::exception &e) {
        qCritical() << "Exception in onCloseButtonClicked:" << e.what();
        close();
    } catch (...) {
        qCritical() << "Unknown exception in onCloseButtonClicked";
        close();
    }
}

void SystemTrayNotification::onAutoCloseTimeout()
{
    if (m_isClosing) {
        return;
    }

    try {
        hideNotification();
    } catch (const std::exception &e) {
        qCritical() << "Exception in onAutoCloseTimeout:" << e.what();
        close();
    } catch (...) {
        qCritical() << "Unknown exception in onAutoCloseTimeout";
        close();
    }
}

void SystemTrayNotification::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !m_isClosing) {
        try {
            int scheduleId = m_schedule.id;
            hideNotification();
            emit notificationClicked(scheduleId);
        } catch (const std::exception &e) {
            qCritical() << "Exception in mousePressEvent:" << e.what();
        } catch (...) {
            qCritical() << "Unknown exception in mousePressEvent";
        }
    }
    QWidget::mousePressEvent(event);
}
