#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QCalendarWidget>
#include <QTableView>
#include <QStackedWidget>
#include <QtAwesome.h>
#include "DatabaseManager.h"
#include "ReminderManager.h"
#include "WeatherManager.h"
#include "ScheduleDetailDialog.h"
#include "ScheduleListDialog.h"
#include "ScheduleDialog.h"
#include "CalendarDelegate.h"
#include "LunarCalendar.h"
#include "EmailBoxWindow.h"
#include "EmailCountWindow.h"
#include "EmailSendWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

enum WindowType {
    Window_Calendar = 0,
    Window_Inbox = 1,
    Window_SendEmail = 2,
    Window_UserManagement = 3,
    Window_AddSchedule = 4,
    Window_ScheduleList = 5,
    Window_Settings = 6
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    
    void switchToWindow(WindowType windowType);

private slots:
    void on_addScheduleButton_clicked();
    void on_viewAllSchedulesButton_clicked();
    void on_settingsButton_clicked();
    void on_emailButton_clicked();
    void on_calendarButton_clicked();
    void on_sendEmailButton_clicked();
    void on_userManagementButton_clicked();
    void on_calendarView_selectionChanged();
    void onCalendarViewClicked(const QDate &date);
    void onDateClickedFromDelegate(const QDate &date);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::Widget *ui;
    DatabaseManager *dbManager;
    ReminderManager *reminderManager;
    WeatherManager *weatherManager;
    fa::QtAwesome *m_awesome;
    QCalendarWidget *m_calendarWidget;
    CalendarDelegate *m_calendarDelegate;
    QTableView* m_calendarTableView;
    LunarCalendar *m_lunarCalendar;
    EmailBoxWindow *m_emailWindow;
    EmailCountWindow *m_emailCountWindow;
    EmailSendWindow *m_emailSendWindow;
    QWidget *m_inboxWidget;
    QWidget *m_sendEmailWidget;
    QWidget *m_userManagementWidget;
    QWidget *m_addScheduleWidget;
    QWidget *m_scheduleListWidget;
    QWidget *m_settingsWidget;
    QStackedWidget *m_windowStack;
    WindowType m_currentWindow;
    bool m_needsRefresh[7];

    void setupCalendar();
    void updateCalendarMarks();
    void updateWeatherDisplay();
    void updateDateInfo();
    void setupToolBarButtons();
    void setupAwesomeIcons();
    void handleCalendarClick(const QDate& date);
    void setupInboxWidget();
    void setupSendEmailWidget();
    void setupUserManagementWidget();
    void setupAddScheduleWidget();
    void setupScheduleListWidget();
    void setupSettingsWidget();
    void setupWindowStack();
    void markWindowAsRefreshed(WindowType windowType);
};
#endif // WIDGET_H
