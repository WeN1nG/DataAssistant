#ifndef SYSTEMTRAYNOTIFICATION_H
#define SYSTEMTRAYNOTIFICATION_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QScreen>
#include <QApplication>
#include "DatabaseManager.h"

class SystemTrayNotification : public QWidget
{
    Q_OBJECT

public:
    explicit SystemTrayNotification(const Schedule &schedule, QWidget *parent = nullptr);
    ~SystemTrayNotification();

    void showNotification();
    void hideNotification();

signals:
    void notificationClicked(int scheduleId);
    void notificationClosed();
    void scheduleMarkedAsCompleted(int scheduleId);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onCloseButtonClicked();
    void onAutoCloseTimeout();

private:
    void setupUI();
    void setupAnimations();
    void slideInAnimation();
    void slideOutAnimation();
    void positionNotification();
    void applyStyles();
    bool isScreenAvailable() const;

    Schedule m_schedule;
    QLabel *m_titleLabel;
    QLabel *m_timeLabel;
    QLabel *m_descriptionLabel;
    QPushButton *m_closeButton;
    QPropertyAnimation *m_posAnimation;
    QPropertyAnimation *m_opacityAnimation;
    QTimer *m_autoCloseTimer;
    int m_animationDuration;
    int m_autoCloseDuration;
    bool m_isVisible;
    bool m_isClosing;
};

#endif
