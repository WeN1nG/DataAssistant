#ifndef CALENDARDELEGATE_H
#define CALENDARDELEGATE_H

#include <QStyledItemDelegate>
#include <QDate>
#include <QPainter>
#include <QRect>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QVector>
#include <QEvent>
#include "DatabaseManager.h"

class CalendarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    CalendarDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    
    void setCurrentMonth(int year, int month);
    void setSchedules(const QVector<Schedule>& schedules);
    void setToday(const QDate& date);
    QDate dateFromIndex(const QModelIndex& index) const;

signals:
    void dateClicked(const QDate& date);

private:
    int m_currentYear;
    int m_currentMonth;
    QDate m_today;
    QVector<Schedule> m_schedules;
    
    bool isCurrentMonth(const QDate& date) const;
    bool hasSchedule(const QDate& date) const;
    int getMaxPriority(const QDate& date) const;
};

#endif // CALENDARDELEGATE_H
