#ifndef SCHEDULELISTDIALOG_H
#define SCHEDULELISTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QDate>
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

private:
    void setupUI();
    void loadSchedules();
    void clearLayout(QLayout* layout);
    QWidget* createScheduleItem(const Schedule& schedule);

private:
    QVBoxLayout* m_mainLayout;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    DatabaseManager* dbManager;
};

#endif // SCHEDULELISTDIALOG_H
