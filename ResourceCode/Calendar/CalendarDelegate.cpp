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
    bool isPastDate = isCurrentMonth && (date < m_today) && !isToday;
    int maxPriority = getMaxPriority(date);
    int dayOfWeek = date.dayOfWeek();
    
    painter->save();
    
    QRect rect = option.rect;
    QRect contentRect = rect.adjusted(2, 2, -2, -2);
    
    QColor backgroundColor;
    QColor textColor;
    QFont font = option.font;
    
    // 根据是否为今天、是否为过去日期、是否为当月日期设置基础背景色
    if (isToday) {
        // 今天是白色背景，除非有日程优先级
        if (maxPriority >= 0) {
            // 有日程：显示优先级颜色
            if (maxPriority == 2) {
                backgroundColor = QColor(255, 180, 180);
            } else if (maxPriority == 1) {
                backgroundColor = QColor(255, 255, 180);
            } else {
                backgroundColor = QColor(180, 200, 255);
            }
        } else {
            // 无日程：白色背景
            backgroundColor = QColor(255, 255, 255);
        }
        textColor = QColor(51, 51, 51);
        font.setBold(true);
    } else if (isPastDate) {
        // 当月已过去的日期：浅灰色背景
        if (maxPriority >= 0) {
            // 有日程：显示优先级颜色
            if (maxPriority == 2) {
                backgroundColor = QColor(255, 180, 180);
            } else if (maxPriority == 1) {
                backgroundColor = QColor(255, 255, 180);
            } else {
                backgroundColor = QColor(180, 200, 255);
            }
        } else {
            backgroundColor = QColor(245, 245, 245);
        }
        textColor = QColor(51, 51, 51);
        font.setBold(false);
    } else if (!isCurrentMonth) {
        // 非当月日期：深灰色背景
        backgroundColor = QColor(224, 224, 224);
        textColor = QColor(51, 51, 51, 150);
        font.setBold(false);
    } else {
        // 当月未来日期：白色背景
        if (maxPriority >= 0) {
            // 有日程：显示优先级颜色
            if (maxPriority == 2) {
                backgroundColor = QColor(255, 180, 180);
            } else if (maxPriority == 1) {
                backgroundColor = QColor(255, 255, 180);
            } else {
                backgroundColor = QColor(180, 200, 255);
            }
        } else {
            backgroundColor = QColor(255, 255, 255);
        }
        textColor = QColor(51, 51, 51);
        font.setBold(false);
    }
    
    // 绘制背景
    painter->setBrush(backgroundColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);
    
    // 设置字体
    painter->setFont(font);
    
    // 设置文字颜色：周末红色，本日和其他日期使用基础颜色
    if (dayOfWeek == 6 || dayOfWeek == 7) {
        // 周末日期：红色字体 #FF0000
        painter->setPen(QColor(255, 0, 0));
    } else {
        painter->setPen(textColor);
    }
    
    // 绘制日期数字
    QString dayText = QString::number(date.day());
    painter->drawText(rect, Qt::AlignCenter, dayText);
    
    painter->restore();
}
