#ifndef SCHEDULEDETAILDIALOG_H
#define SCHEDULEDETAILDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QEvent>
#include <QTimer>
#include <QPointer>
#include "DatabaseManager.h"

class ScheduleDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduleDetailDialog(const QDate& date, const QVector<Schedule>& schedules, QWidget* parent = nullptr);
    ~ScheduleDetailDialog();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onEditSchedule(int scheduleId);
    void onDeleteSchedule(int scheduleId);
    void refreshSchedules();

private:
    void setupUI();
    void formatScheduleContent();
    void showScheduleDetails();
    void animateClose();
    void resetState();
    void performClose();

private:
    QDate m_date;
    QVector<Schedule> m_schedules;
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    QPushButton* m_closeButton;
    DatabaseManager* dbManager;
    QPropertyAnimation* m_closeAnimation;
    bool m_isClosing;
    QPointer<QWidget> m_parentWidget;
};

#endif // SCHEDULEDETAILDIALOG_H
