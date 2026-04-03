#ifndef SCHEDULEDETAILDIALOG_H
#define SCHEDULEDETAILDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "DatabaseManager.h"

class ScheduleDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduleDetailDialog(const QDate& date, const QVector<Schedule>& schedules, QWidget* parent = nullptr);

private slots:
    void onEditSchedule(int scheduleId);
    void onDeleteSchedule(int scheduleId);
    void refreshSchedules();

private:
    void setupUI();
    void formatScheduleContent();
    void showScheduleDetails();

private:
    QDate m_date;
    QVector<Schedule> m_schedules;
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    QPushButton* m_closeButton;
    DatabaseManager* dbManager;
};

#endif // SCHEDULEDETAILDIALOG_H
