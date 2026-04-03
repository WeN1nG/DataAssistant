#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "DatabaseManager.h"
#include "ReminderManager.h"
#include "WeatherManager.h"
#include "ScheduleDetailDialog.h"
#include "ScheduleListDialog.h"

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

private:
    Ui::Widget *ui;
    DatabaseManager *dbManager;
    ReminderManager *reminderManager;
    WeatherManager *weatherManager;
    
    void setupCalendar();
    void updateCalendarMarks();
    void updateWeatherDisplay();
    void setupToolBarButtons();
};
#endif // WIDGET_H
