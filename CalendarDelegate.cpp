#include "CalendarDelegate.h"
#include <QApplication>
#include <QPainterPath>
#include <QMouseEvent>

CalendarDelegate::CalendarDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
    , m_currentYear(0)
    , m_currentMonth(0)
{
}

void CalendarDelegate::setCurrentMonth(int year, int month)
{
    m_currentYear = year;
    m_currentMonth = month;
}

void CalendarDelegate::setSchedules(const QVector<Schedule>& schedules)
{
    m_schedules = schedules;
}

void CalendarDelegate::setToday(const QDate& date)
{
    m_today = date;
}

QDate CalendarDelegate::dateFromIndex(const QModelIndex& index) const
{
    int row = index.row();
    int col = index.column();
    
    // 验证索引有效性
    if (!index.isValid()) {
        return QDate();
    }
    
    // 如果年份和月份未设置，使用当前日期
    int year = m_currentYear;
    int month = m_currentMonth;
    if (year == 0 || month == 0) {
        year = QDate::currentDate().year();
        month = QDate::currentDate().month();
    }
    
    int firstDayOfWeek = 1;
    int referenceDayOfWeek = QDate(year, month, 1).dayOfWeek();
    int daysToSubtract = (referenceDayOfWeek - firstDayOfWeek + 7) % 7;
    
    int dayOffset = row * 7 + col;
    QDate date = QDate(year, month, 1).addDays(dayOffset - daysToSubtract);
    
    return date;
}

bool CalendarDelegate::isCurrentMonth(const QDate& date) const
{
    if (m_currentMonth == 0) return true;
    return date.month() == m_currentMonth;
}

bool CalendarDelegate::hasSchedule(const QDate& date) const
{
    for (const Schedule& schedule : m_schedules) {
        if (schedule.datetime.date() == date) {
            return true;
        }
    }
    return false;
}

int CalendarDelegate::getMaxPriority(const QDate& date) const
{
    int maxPriority = -1;
    for (const Schedule& schedule : m_schedules) {
        if (schedule.datetime.date() == date) {
            if (schedule.priority > maxPriority) {
                maxPriority = schedule.priority;
            }
        }
    }
    return maxPriority;
}

bool CalendarDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_UNUSED(model);
    Q_UNUSED(option);
    
    // 只在鼠标释放时触发点击事件，避免重复触发
    if (event->type() == QEvent::MouseButtonRelease) {
        // 检查index是否有效
        if (index.isValid()) {
            QDate date = dateFromIndex(index);
            if (date.isValid()) {
                // 添加调试信息
                qDebug() << "CalendarDelegate: Date clicked:" << date 
                         << "Row:" << index.row() << "Col:" << index.column();
                emit dateClicked(date);
            }
        }
    }
    
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void CalendarDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QDate date = dateFromIndex(index);
    bool isCurrentMonth = (date.month() == m_currentMonth);
    bool isToday = (date == m_today);
    int maxPriority = getMaxPriority(date);
    
    painter->save();
    
    QRect rect = option.rect;
    QRect contentRect = rect.adjusted(2, 2, -2, -2);
    
    QColor backgroundColor;
    QColor textColor;
    QFont font = option.font;
    bool drawBorder = false;
    
    // 根据优先级和是否为今天设置背景色
    if (isToday) {
        // 今天需要显示边框，同时保留优先级颜色
        font.setBold(true);
        drawBorder = true;
        
        // 今天的背景色根据优先级变化
        if (maxPriority == 2) {
            // 紧急：淡红色背景
            backgroundColor = QColor(255, 180, 180);
            textColor = QColor(51, 51, 51);
        } else if (maxPriority == 1) {
            // 重要：淡黄色背景
            backgroundColor = QColor(255, 255, 180);
            textColor = QColor(51, 51, 51);
        } else if (maxPriority == 0) {
            // 一般：淡蓝色背景
            backgroundColor = QColor(180, 200, 255);
            textColor = QColor(51, 51, 51);
        } else {
            // 无日程：使用默认的浅灰色
            backgroundColor = QColor(245, 245, 245);
            textColor = QColor(51, 51, 51);
        }
    } else if (!isCurrentMonth) {
        backgroundColor = QColor(238, 238, 238);
        textColor = QColor(51, 51, 51, 150);
        font.setBold(false);
    } else if (maxPriority == 2) {
        // 紧急：淡红色
        backgroundColor = QColor(255, 180, 180);
        textColor = QColor(51, 51, 51);
        font.setBold(false);
    } else if (maxPriority == 1) {
        // 重要：淡黄色
        backgroundColor = QColor(255, 255, 180);
        textColor = QColor(51, 51, 51);
        font.setBold(false);
    } else if (maxPriority == 0) {
        // 一般：淡蓝色
        backgroundColor = QColor(180, 200, 255);
        textColor = QColor(51, 51, 51);
        font.setBold(false);
    } else {
        // 无日程
        backgroundColor = QColor(250, 250, 250);
        textColor = QColor(51, 51, 51);
        font.setBold(false);
    }
    
    painter->setBrush(backgroundColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);
    
    if (drawBorder) {
        QPen borderPen(QColor(51, 51, 51), 2);
        painter->setPen(borderPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(contentRect);
    }
    
    painter->setFont(font);
    
    // 周六周日设置红色文字
    int dayOfWeek = date.dayOfWeek();
    if (dayOfWeek == 6 || dayOfWeek == 7) {
        painter->setPen(QColor(200, 50, 50));
    } else {
        painter->setPen(textColor);
    }
    
    QString dayText = QString::number(date.day());
    painter->drawText(rect, Qt::AlignCenter, dayText);
    
    painter->restore();
}
