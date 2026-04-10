#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QCalendarWidget>
#include <QTableView>
#include <QtAwesome.h>
#include "DatabaseManager.h"
#include "ReminderManager.h"
#include "WeatherManager.h"
#include "ScheduleDetailDialog.h"
#include "ScheduleListDialog.h"
#include "CalendarDelegate.h"
#include "LunarCalendar.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_addScheduleButton_clicked();
    void on_viewAllSchedulesButton_clicked();
    void on_settingsButton_clicked();
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

    void setupCalendar();
    void updateCalendarMarks();
    void updateWeatherDisplay();
    void updateDateInfo();
    void setupToolBarButtons();
    void setupAwesomeIcons();
    void handleCalendarClick(const QDate& date);
};
#endif // WIDGET_H
