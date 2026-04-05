#ifndef SCHEDULELISTDIALOG_H
#define SCHEDULELISTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QDate>
#include <QPropertyAnimation>
#include <QMap>
#include "DatabaseManager.h"

class ScheduleListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduleListDialog(QWidget* parent = nullptr);

private slots:
    void onEditSchedule(int scheduleId);
    void onDeleteSchedule(int scheduleId);
    void refreshScheduleList();
    void toggleMinimize(int scheduleId);

private:
    void setupUI();
    void loadSchedules();
    void clearLayout(QLayout* layout);
    QWidget* createScheduleItem(const Schedule& schedule);
    void updateScheduleItemVisibility(QWidget* itemWidget, bool minimized);

private:
    QVBoxLayout* m_mainLayout;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    DatabaseManager* dbManager;
    QMap<int, bool> m_minimizedState;  // 存储每个日程项的最小化状态
    QMap<int, QPropertyAnimation*> m_animations;  // 存储动画对象
};

#endif // SCHEDULELISTDIALOG_H
