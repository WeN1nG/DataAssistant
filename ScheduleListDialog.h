#ifndef SCHEDULELISTDIALOG_H
#define SCHEDULELISTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDate>
#include <QPropertyAnimation>
#include <QMap>
#include <QSet>
#include <QTimer>
#include "DatabaseManager.h"

class ScheduleListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduleListDialog(QWidget* parent = nullptr);

private slots:
    void onEditSchedule(int scheduleId);
    void onDeleteSchedule(int scheduleId);
    void onBatchDelete();
    void onSelectAll();
    void onDeselectAll();
    void onDeleteAll();
    void refreshScheduleList();
    void toggleMinimize(int scheduleId);
    void toggleScheduleSelection(int scheduleId, bool selected);
    void updateBatchDelete();

private:
    void setupUI();
    void loadSchedules();
    void clearLayout(QLayout* layout);
    QWidget* createScheduleItem(const Schedule& schedule);
    void updateScheduleItemVisibility(QWidget* itemWidget, bool minimized);
    void showLoadingState(bool show);
    void showDeleteFeedback(bool success, int count);

private:
    QVBoxLayout* m_mainLayout;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    QWidget* m_batchActionBar;
    QPushButton* m_batchDelete;
    QPushButton* m_selectAll;
    QPushButton* m_deselectAll;
    QLabel* m_selectionCountLabel;
    QWidget* m_loadingOverlay;
    QLabel* m_loadingLabel;
    DatabaseManager* dbManager;
    QMap<int, bool> m_minimizedState;  // 存储每个日程项的最小化状态
    QMap<int, QPropertyAnimation*> m_animations;  // 存储动画对象
    QSet<int> m_selectedScheduleIds;  // 存储选中的日程ID
    QMap<int, QWidget*> m_scheduleItemWidgets;  // 日程ID到UI组件的映射
};

#endif // SCHEDULELISTDIALOG_H
