/**
 * 文件名：LunarCalendar.h
 * 功能：农历日期转换和计算
 * 创建时间：2026-04-05
 */

#ifndef LUNARCALENDAR_H
#define LUNARCALENDAR_H

#include <QDate>
#include <QString>
#include <QVector>

class LunarCalendar
{
public:
    struct LunarDate {
        int year;       // 农历年
        int month;      // 农历月（1-12，闰月为负数表示）
        int day;        // 农历日（1-30）
        bool isLeap;    // 是否闰月
        
        QString toString() const;
        bool isValid() const;
    };
    
    struct SolarTermInfo {
        QString name;           // 节气名称
        QDate date;             // 节气日期
        bool isValid;
    };
    
    LunarCalendar();
    
    LunarDate gregorianToLunar(const QDate &date) const;
    QDate lunarToGregorian(int lunarYear, int lunarMonth, int lunarDay, bool isLeap = false) const;
    
    QString getLunarDateString(const QDate &date) const;
    QString getSolarTerm(const QDate &date) const;
    QString getHoliday(const QDate &date) const;
    
    bool isLunarLeapMonth(int year, int month) const;
    int getLunarMonthDays(int year, int month) const;
    int getLunarYearDays(int year) const;
    
    static const QStringList lunarMonths;
    static const QStringList lunarDays;
    static const QStringList solarTerms;
    static const QStringList lunarFestivals;
    static const QStringList gregorianFestivals;
    
private:
    QVector<int> m_lunarInfo;
    
    void initLunarInfo();
    int getLunarInfo(int year) const;
    QDate getLunarNewYear(int year) const;
    int calculateLunarDays(const QDate &date) const;
    QDate getSolarTermDate(int year, int index) const;
    
    static const int LUNAR_BASE_YEAR = 1900;
    static const int LUNAR_END_YEAR = 2100;
};

#endif // LUNARCALENDAR_H
