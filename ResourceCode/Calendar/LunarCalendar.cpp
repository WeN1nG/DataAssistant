/**
 * 文件名：LunarCalendar.cpp
 * 功能：农历日期转换和计算实现
 * 创建时间：2026-04-05
 */

#include "LunarCalendar.h"
#include <QDebug>

const QStringList LunarCalendar::lunarMonths = {
    "正", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊"
};

const QStringList LunarCalendar::lunarDays = {
    "初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十",
    "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十",
    "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"
};

const QStringList LunarCalendar::solarTerms = {
    "小寒", "大寒",   // 1月
    "立春", "雨水", "惊蛰", "春分",  // 2月
    "清明", "谷雨",   // 3月
    "立夏", "小满", "芒种", "夏至",  // 4月
    "小暑", "大暑",   // 5月
    "立秋", "处暑",   // 6月
    "白露", "秋分",   // 7月
    "寒露", "霜降",   // 8月
    "立冬", "小雪",   // 9月
    "大雪", "冬至"    // 10月
};

const QStringList LunarCalendar::lunarFestivals = {
    "春节", "", "元宵节", "", "", "", "", "", "", "",  // 1月：春节(1.1), 元宵节(1.15)
    "", "", "", "", "", "", "", "", "", "",           // 2月
    "", "", "", "", "", "", "清明节", "", "", "",     // 3月：清明节(清明当日)
    "", "", "", "", "端午节", "", "", "", "", "",     // 4月：端午节(5.5)
    "", "", "", "", "中秋节", "", "", "", "", "",     // 5月：中秋节(8.15，但农历)
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "重阳节", "",     // 9月：重阳节(9.9)
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "除夕"         // 12月：除夕(腊月最后一天)
};

const QStringList LunarCalendar::gregorianFestivals = {
    "元旦", "", "", "", "", "", "", "", "", "",          // 1月
    "春节", "情人节", "", "", "", "", "", "", "", "",  // 2月
    "", "", "", "", "", "", "", "", "", "",
    "愚人节", "", "", "", "", "", "", "", "", "",      // 4月
    "劳动节", "", "", "", "", "", "", "", "", "",       // 5月
    "儿童节", "", "", "", "", "", "", "", "", "",       // 6月
    "建党节", "", "", "", "", "", "", "", "", "",       // 7月
    "建军节", "", "", "", "", "", "", "", "", "",       // 8月
    "教师节", "", "", "", "", "", "", "", "", "",       // 9月
    "国庆节", "", "", "", "", "", "", "", "", "",       // 10月
    "", "", "", "", "", "", "", "", "", "",
    "圣诞节", "", "", "", "", "", "", "", "", ""        // 12月
};

LunarCalendar::LunarCalendar()
{
    initLunarInfo();
}

void LunarCalendar::initLunarInfo()
{
    m_lunarInfo.resize(LUNAR_END_YEAR - LUNAR_BASE_YEAR + 1);
    
    m_lunarInfo[0] = 0x4bd8;      // 1900 闰六月
    m_lunarInfo[1] = 0x4ae0;      // 1901
    m_lunarInfo[2] = 0xa570;      // 1902 闰五月
    m_lunarInfo[3] = 0x54d5;      // 1903
    m_lunarInfo[4] = 0xd260;      // 1904
    m_lunarInfo[5] = 0xd950;      // 1905 闰五月
    m_lunarInfo[6] = 0x6554;      // 1906
    m_lunarInfo[7] = 0x56a0;      // 1907
    m_lunarInfo[8] = 0x9ad0;      // 1908
    m_lunarInfo[9] = 0x55d2;      // 1909 闰二月
    m_lunarInfo[10] = 0x4ae0;     // 1910
    m_lunarInfo[11] = 0xa5b6;     // 1911 闰二月
    m_lunarInfo[12] = 0xa4d0;     // 1912 闰二月
    m_lunarInfo[13] = 0xd250;     // 1913
    m_lunarInfo[14] = 0xd255;     // 1914 闰五月
    m_lunarInfo[15] = 0xb540;     // 1915
    m_lunarInfo[16] = 0xd6a0;     // 1916
    m_lunarInfo[17] = 0xada2;     // 1917 闰五月
    m_lunarInfo[18] = 0x95b0;     // 1918
    m_lunarInfo[19] = 0x4977;     // 1919 闰二月
    m_lunarInfo[20] = 0x4970;     // 1920
    m_lunarInfo[21] = 0xa4b0;     // 1921 闰六月
    m_lunarInfo[22] = 0xb4b5;     // 1922
    m_lunarInfo[23] = 0x6a50;     // 1923 闰二月
    m_lunarInfo[24] = 0x6d40;     // 1924
    m_lunarInfo[25] = 0xabe6;     // 1925 闰二月
    m_lunarInfo[26] = 0x5270;     // 1926
    m_lunarInfo[27] = 0x5e26;     // 1927 闰二月
    m_lunarInfo[28] = 0x4e60;     // 1928 闰四月
    m_lunarInfo[29] = 0x4f13;     // 1929
    m_lunarInfo[30] = 0x5370;     // 1930 闰五月
    m_lunarInfo[31] = 0x55a8;     // 1931
    m_lunarInfo[32] = 0xaa5a;     // 1932 闰二月
    m_lunarInfo[33] = 0x5b52;     // 1933
    m_lunarInfo[34] = 0x9366;     // 1934 闰九月
    m_lunarInfo[35] = 0x4938;     // 1935
    m_lunarInfo[36] = 0x5475;     // 1936 闰五月
    m_lunarInfo[37] = 0xa630;     // 1937
    m_lunarInfo[38] = 0x7464;     // 1938 闰六月
    m_lunarInfo[39] = 0xe4f2;     // 1939
    m_lunarInfo[40] = 0xd150;     // 1940
    m_lunarInfo[41] = 0x5a7f;     // 1941 闰六月
    m_lunarInfo[42] = 0x5b57;     // 1942
    m_lunarInfo[43] = 0x4d9c;     // 1943
    m_lunarInfo[44] = 0xda50;     // 1944
    m_lunarInfo[45] = 0x7545;     // 1945 闰二月
    m_lunarInfo[46] = 0x56da;     // 1946
    m_lunarInfo[47] = 0x55b2;     // 1947 闰二月
    m_lunarInfo[48] = 0xa558;     // 1948 闰七月
    m_lunarInfo[49] = 0xd4d;     // 1949
    m_lunarInfo[50] = 0x4dd5;     // 1950
    m_lunarInfo[51] = 0x4ad0;     // 1951
    m_lunarInfo[52] = 0xa4d0;     // 1952 闰二月
    m_lunarInfo[53] = 0xd4d4;     // 1953
    m_lunarInfo[54] = 0xd250;     // 1954 闰二月
    m_lunarInfo[55] = 0xd558;     // 1955 闰三月
    m_lunarInfo[56] = 0xb540;     // 1956
    m_lunarInfo[57] = 0x56a0;     // 1957
    m_lunarInfo[58] = 0x96d5;     // 1958 闰五月
    m_lunarInfo[59] = 0x55af;     // 1959
    m_lunarInfo[60] = 0x4e60;     // 1960
    m_lunarInfo[61] = 0x4ea6;     // 1961 闰三月
    m_lunarInfo[62] = 0x6b58;     // 1962
    m_lunarInfo[63] = 0x5ac0;     // 1963 闰二月
    m_lunarInfo[64] = 0xab60;     // 1964
    m_lunarInfo[65] = 0x9570;     // 1965
    m_lunarInfo[66] = 0x4af5;     // 1966
    m_lunarInfo[67] = 0x4970;     // 1967 闰七月
    m_lunarInfo[68] = 0xa4b0;     // 1968
    m_lunarInfo[69] = 0xaa99;     // 1969 闰五月
    m_lunarInfo[70] = 0x5b57;     // 1970
    m_lunarInfo[71] = 0x5a57;     // 1971
    m_lunarInfo[72] = 0x5d36;     // 1972 闰三月
    m_lunarInfo[73] = 0xa5b8;     // 1973
    m_lunarInfo[74] = 0x5e25;     // 1974 闰四月
    m_lunarInfo[75] = 0x4e65;     // 1975
    m_lunarInfo[76] = 0x56a0;     // 1976 闰六月
    m_lunarInfo[77] = 0x76d4;     // 1977
    m_lunarInfo[78] = 0x4e56;     // 1978
    m_lunarInfo[79] = 0x4ee3;     // 1979 闰六月
    m_lunarInfo[80] = 0x5350;     // 1980 闰四月
    m_lunarInfo[81] = 0x5533;     // 1981
    m_lunarInfo[82] = 0xaa5a;     // 1982 闰三月
    m_lunarInfo[83] = 0x5b52;     // 1983
    m_lunarInfo[84] = 0x9666;     // 1984 闰十月
    m_lunarInfo[85] = 0x49d8;     // 1985
    m_lunarInfo[86] = 0x54d5;     // 1986
    m_lunarInfo[87] = 0xa630;     // 1987 闰六月
    m_lunarInfo[88] = 0x7465;     // 1988
    m_lunarInfo[89] = 0xe4f2;     // 1989 闰五月
    m_lunarInfo[90] = 0xd150;     // 1990
    m_lunarInfo[91] = 0x5a7e;     // 1991 闰三月
    m_lunarInfo[92] = 0xab5b;     // 1992 闰三月
    m_lunarInfo[93] = 0x4d9c;     // 1993
    m_lunarInfo[94] = 0xda50;     // 1994 闰八月
    m_lunarInfo[95] = 0x7545;     // 1995
    m_lunarInfo[96] = 0x56da;     // 1996 闰八月
    m_lunarInfo[97] = 0x55b5;     // 1997
    m_lunarInfo[98] = 0xa5b7;     // 1998 闰五月
    m_lunarInfo[99] = 0xd4d;     // 1999
    m_lunarInfo[100] = 0x465a;    // 2000 闰四月
    m_lunarInfo[101] = 0xcb5;    // 2001
    m_lunarInfo[102] = 0x45aa;    // 2002 闰四月
    m_lunarInfo[103] = 0xb53;    // 2003
    m_lunarInfo[104] = 0x2666;    // 2004 闰二月
    m_lunarInfo[105] = 0xccd;    // 2005
    m_lunarInfo[106] = 0x799a;    // 2006 闰七月
    m_lunarInfo[107] = 0xb35;    // 2007
    m_lunarInfo[108] = 0x566a;    // 2008 闰五月
    m_lunarInfo[109] = 0xcd5;    // 2009
    m_lunarInfo[110] = 0x459a;    // 2010 闰四月
    m_lunarInfo[111] = 0xd35;    // 2011
    m_lunarInfo[112] = 0x496a;    // 2012 闰四月
    m_lunarInfo[113] = 0xad5;    // 2013
    m_lunarInfo[114] = 0x95a6;    // 2014 闰九月
    m_lunarInfo[115] = 0xd4d;    // 2015
    m_lunarInfo[116] = 0x365a;    // 2016 闰三月
    m_lunarInfo[117] = 0x6ad5;    // 2017 闰六月
    m_lunarInfo[118] = 0x6aa;    // 2018
    m_lunarInfo[119] = 0x4b35;    // 2019 闰四月
    m_lunarInfo[120] = 0x49aa;    // 2020 闰四月
    m_lunarInfo[121] = 0xb4d;    // 2021
    m_lunarInfo[122] = 0x465a;    // 2022 闰四月
    m_lunarInfo[123] = 0x24d5;    // 2023 闰二月
    m_lunarInfo[124] = 0xb55;    // 2024
    m_lunarInfo[125] = 0x665a;    // 2025 闰六月
    m_lunarInfo[126] = 0xab5;    // 2026
    m_lunarInfo[127] = 0x596a;    // 2027 闰五月
    m_lunarInfo[128] = 0xb55;    // 2028
    m_lunarInfo[129] = 0x3596;    // 2029 闰三月
    m_lunarInfo[130] = 0xd2d;    // 2030
    m_lunarInfo[131] = 0x665a;    // 2031 闰六月
    m_lunarInfo[132] = 0xcb5;    // 2032
    m_lunarInfo[133] = 0x396a;    // 2033 闰三月
    m_lunarInfo[134] = 0xb4d;    // 2034
    m_lunarInfo[135] = 0x669a;    // 2035 闰六月
    m_lunarInfo[136] = 0xd33;    // 2036
    m_lunarInfo[137] = 0x596a;    // 2037 闰五月
    m_lunarInfo[138] = 0x2cd;    // 2038
    m_lunarInfo[139] = 0x46b5;    // 2039 闰四月
    m_lunarInfo[140] = 0x56a;    // 2040
    m_lunarInfo[141] = 0x2ad3;    // 2041 闰二月
    m_lunarInfo[142] = 0xda6;    // 2042
    m_lunarInfo[143] = 0x669a;    // 2043 闰六月
    m_lunarInfo[144] = 0xcb5;    // 2044
    m_lunarInfo[145] = 0x556a;    // 2045 闰五月
    m_lunarInfo[146] = 0xb4d;    // 2046
    m_lunarInfo[147] = 0x3a5a;    // 2047 闰三月
    m_lunarInfo[148] = 0xab5;    // 2048
    m_lunarInfo[149] = 0x7566;    // 2049 闰七月
    m_lunarInfo[150] = 0xb4d;    // 2050
    m_lunarInfo[151] = 0x559a;    // 2051 闰五月
    m_lunarInfo[152] = 0xd35;    // 2052
    m_lunarInfo[153] = 0x466a;    // 2053 闰四月
    m_lunarInfo[154] = 0xcd3;    // 2054
    m_lunarInfo[155] = 0x2596;    // 2055 闰二月
    m_lunarInfo[156] = 0xd2d;    // 2056
    m_lunarInfo[157] = 0x665a;    // 2057 闰六月
    m_lunarInfo[158] = 0xcb5;    // 2058
    m_lunarInfo[159] = 0x456a;    // 2059 闰四月
    m_lunarInfo[160] = 0xad5;    // 2060
    m_lunarInfo[161] = 0x469a;    // 2061 闰四月
    m_lunarInfo[162] = 0xb2d;    // 2062
    m_lunarInfo[163] = 0x8a6a;    // 2063 闰八月
    m_lunarInfo[164] = 0xad5;    // 2064
    m_lunarInfo[165] = 0x55a6;    // 2065 闰五月
    m_lunarInfo[166] = 0xd4d;    // 2066
    m_lunarInfo[167] = 0x365a;    // 2067 闰三月
    m_lunarInfo[168] = 0xcb5;    // 2068
    m_lunarInfo[169] = 0x756a;    // 2069 闰七月
    m_lunarInfo[170] = 0xb4d;    // 2070
    m_lunarInfo[171] = 0x565a;    // 2071 闰五月
    m_lunarInfo[172] = 0xcb5;    // 2072
    m_lunarInfo[173] = 0x356a;    // 2073 闰三月
    m_lunarInfo[174] = 0xad3;    // 2074
    m_lunarInfo[175] = 0x6596;    // 2075 闰六月
    m_lunarInfo[176] = 0xd2d;    // 2076
    m_lunarInfo[177] = 0x565a;    // 2077 闰五月
    m_lunarInfo[178] = 0xcd5;    // 2078
    m_lunarInfo[179] = 0x22ad;    // 2079 闰二月
    m_lunarInfo[180] = 0x6b5;    // 2080
    m_lunarInfo[181] = 0x7d6a;    // 2081 闰七月
    m_lunarInfo[182] = 0xd56;    // 2082
    m_lunarInfo[183] = 0x559a;    // 2083 闰五月
    m_lunarInfo[184] = 0xb35;    // 2084
    m_lunarInfo[185] = 0x466a;    // 2085 闰四月
    m_lunarInfo[186] = 0xccd;    // 2086
    m_lunarInfo[187] = 0x2ab5;    // 2087 闰二月
    m_lunarInfo[188] = 0xd66;    // 2088
    m_lunarInfo[189] = 0x665a;    // 2089 闰六月
    m_lunarInfo[190] = 0xcb5;    // 2090
    m_lunarInfo[191] = 0x556a;    // 2091 闰五月
    m_lunarInfo[192] = 0xad3;    // 2092
    m_lunarInfo[193] = 0x3d9a;    // 2093 闰三月
    m_lunarInfo[194] = 0x666;    // 2094
    m_lunarInfo[195] = 0x6ccd;    // 2095 闰六月
    m_lunarInfo[196] = 0x59a;    // 2096
    m_lunarInfo[197] = 0x5b35;    // 2097 闰五月
    m_lunarInfo[198] = 0x666;    // 2098
    m_lunarInfo[199] = 0x2ccd;    // 2099 闰二月
    m_lunarInfo[200] = 0x55a;    // 2100
}

int LunarCalendar::getLunarInfo(int year) const
{
    if (year < LUNAR_BASE_YEAR || year > LUNAR_END_YEAR) {
        return 0;
    }
    return m_lunarInfo[year - LUNAR_BASE_YEAR];
}

int LunarCalendar::getLunarMonthDays(int year, int month) const
{
    int info = getLunarInfo(year);
    int leapMonth = (info >> 12) & 0x0F;
    
    if (month < 1 || month > 12) {
        return 0;
    }
    
    int monthBit = (info >> (month - 1)) & 0x01;
    return monthBit ? 30 : 29;
}

int LunarCalendar::getLunarYearDays(int year) const
{
    int info = getLunarInfo(year);
    int days = 0;
    
    for (int i = 0; i < 12; ++i) {
        days += (info >> i) & 0x01 ? 30 : 29;
    }
    
    int leapMonth = (info >> 12) & 0x0F;
    if (leapMonth > 0) {
        days += getLunarMonthDays(year, leapMonth);
    }
    
    return days;
}

bool LunarCalendar::isLunarLeapMonth(int year, int month) const
{
    int info = getLunarInfo(year);
    int leapMonth = (info >> 12) & 0x0F;
    return (leapMonth == month);
}

QDate LunarCalendar::getLunarNewYear(int year) const
{
    QDate baseDate(1900, 1, 31);
    
    if (year < LUNAR_BASE_YEAR || year > LUNAR_END_YEAR) {
        return QDate();
    }
    
    int days = 0;
    for (int y = LUNAR_BASE_YEAR; y < year; ++y) {
        days += getLunarYearDays(y);
    }
    
    return baseDate.addDays(days);
}

int LunarCalendar::calculateLunarDays(const QDate &date) const
{
    QDate baseDate(1900, 1, 31);
    return baseDate.daysTo(date);
}

LunarCalendar::LunarDate LunarCalendar::gregorianToLunar(const QDate &date) const
{
    LunarDate lunar;
    lunar.year = 0;
    lunar.month = 0;
    lunar.day = 0;
    lunar.isLeap = false;
    
    if (!date.isValid()) {
        return lunar;
    }
    
    int days = calculateLunarDays(date);
    int lunarYear = LUNAR_BASE_YEAR;
    int lunarMonth = 1;
    int lunarDay = 1;
    
    while (days >= getLunarYearDays(lunarYear)) {
        days -= getLunarYearDays(lunarYear);
        lunarYear++;
    }
    
    int info = getLunarInfo(lunarYear);
    int leapMonth = (info >> 12) & 0x0F;
    
    lunarMonth = 1;
    while (lunarMonth <= 12) {
        int monthDays = getLunarMonthDays(lunarYear, lunarMonth);
        
        if (days < monthDays) {
            break;
        }
        
        days -= monthDays;
        lunarMonth++;
        
        if (lunarMonth - 1 == leapMonth && days > 0) {
            int leapMonthDays = getLunarMonthDays(lunarYear, leapMonth);
            if (days < leapMonthDays) {
                lunar.isLeap = true;
                lunarDay = days + 1;
                lunar.month = leapMonth;
                break;
            }
            days -= leapMonthDays;
            lunarMonth++;
        }
    }
    
    if (lunar.month == 0) {
        lunar.month = lunarMonth;
    }
    if (lunar.day == 0) {
        lunar.day = days + 1;
    }
    lunar.year = lunarYear;
    
    return lunar;
}

QDate LunarCalendar::lunarToGregorian(int lunarYear, int lunarMonth, int lunarDay, bool isLeap) const
{
    if (lunarYear < LUNAR_BASE_YEAR || lunarYear > LUNAR_END_YEAR) {
        return QDate();
    }
    
    if (lunarMonth < 1 || lunarMonth > 12 || lunarDay < 1) {
        return QDate();
    }
    
    int days = 0;
    
    for (int y = LUNAR_BASE_YEAR; y < lunarYear; ++y) {
        days += getLunarYearDays(y);
    }
    
    int info = getLunarInfo(lunarYear);
    int leapMonth = (info >> 12) & 0x0F;
    
    for (int m = 1; m < lunarMonth; ++m) {
        days += getLunarMonthDays(lunarYear, m);
    }
    
    if (isLeap && lunarMonth == leapMonth) {
        days += getLunarMonthDays(lunarYear, leapMonth);
    }
    
    int monthDays = getLunarMonthDays(lunarYear, lunarMonth);
    if (lunarDay > monthDays) {
        return QDate();
    }
    
    days += lunarDay - 1;
    
    QDate baseDate(1900, 1, 31);
    return baseDate.addDays(days);
}

QString LunarCalendar::LunarDate::toString() const
{
    if (!isValid()) {
        return "无效日期";
    }
    
    QString monthStr = lunarMonths.value(month - 1, "正");
    if (isLeap) {
        monthStr = "闰" + monthStr;
    }
    
    QString dayStr = lunarDays.value(day - 1, "初一");
    return QString("%1月%2").arg(monthStr).arg(dayStr);
}

bool LunarCalendar::LunarDate::isValid() const
{
    return year >= LUNAR_BASE_YEAR && month >= 1 && month <= 12 && day >= 1 && day <= 30;
}

QString LunarCalendar::getLunarDateString(const QDate &date) const
{
    LunarDate lunar = gregorianToLunar(date);
    
    if (!lunar.isValid()) {
        return "日期错误";
    }
    
    QString holiday = getHoliday(date);
    if (!holiday.isEmpty()) {
        return holiday;
    }
    
    return lunar.toString();
}

QString LunarCalendar::getHoliday(const QDate &date) const
{
    LunarDate lunar = gregorianToLunar(date);
    
    if (!lunar.isValid()) {
        return "";
    }
    
    if (lunar.month == 1 && lunar.day == 1) {
        return "春节";
    }
    if (lunar.month == 1 && lunar.day == 15) {
        return "元宵节";
    }
    if (lunar.month == 5 && lunar.day == 5) {
        return "端午节";
    }
    if (lunar.month == 8 && lunar.day == 15) {
        return "中秋节";
    }
    if (lunar.month == 9 && lunar.day == 9) {
        return "重阳节";
    }
    if (lunar.month == 12 && lunar.day == 30) {
        QDate nextDay = date.addDays(1);
        LunarDate nextLunar = gregorianToLunar(nextDay);
        if (nextLunar.month == 1 && nextLunar.day == 1) {
            return "除夕";
        }
    }
    
    return "";
}

struct SolarTermEntry {
    int year;
    int month;
    int day;
    const char* name;
};

static const SolarTermEntry solarTermsData[] = {
    {2016, 1, 6, "小寒"}, {2016, 1, 20, "大寒"}, {2016, 2, 4, "立春"}, {2016, 2, 19, "雨水"},
    {2016, 3, 5, "惊蛰"}, {2016, 3, 20, "春分"}, {2016, 4, 4, "清明"}, {2016, 4, 19, "谷雨"},
    {2016, 5, 5, "立夏"}, {2016, 5, 20, "小满"}, {2016, 6, 5, "芒种"}, {2016, 6, 21, "夏至"},
    {2016, 7, 7, "小暑"}, {2016, 7, 22, "大暑"}, {2016, 8, 7, "立秋"}, {2016, 8, 23, "处暑"},
    {2016, 9, 7, "白露"}, {2016, 9, 22, "秋分"}, {2016, 10, 8, "寒露"}, {2016, 10, 23, "霜降"},
    {2016, 11, 7, "立冬"}, {2016, 11, 22, "小雪"}, {2016, 12, 7, "大雪"}, {2016, 12, 21, "冬至"},
    {2017, 1, 5, "小寒"}, {2017, 1, 20, "大寒"}, {2017, 2, 3, "立春"}, {2017, 2, 18, "雨水"},
    {2017, 3, 5, "惊蛰"}, {2017, 3, 20, "春分"}, {2017, 4, 4, "清明"}, {2017, 4, 20, "谷雨"},
    {2017, 5, 5, "立夏"}, {2017, 5, 21, "小满"}, {2017, 6, 5, "芒种"}, {2017, 6, 21, "夏至"},
    {2017, 7, 7, "小暑"}, {2017, 7, 22, "大暑"}, {2017, 8, 7, "立秋"}, {2017, 8, 23, "处暑"},
    {2017, 9, 7, "白露"}, {2017, 9, 23, "秋分"}, {2017, 10, 8, "寒露"}, {2017, 10, 23, "霜降"},
    {2017, 11, 7, "立冬"}, {2017, 11, 22, "小雪"}, {2017, 12, 7, "大雪"}, {2017, 12, 22, "冬至"},
    {2018, 1, 5, "小寒"}, {2018, 1, 20, "大寒"}, {2018, 2, 4, "立春"}, {2018, 2, 19, "雨水"},
    {2018, 3, 5, "惊蛰"}, {2018, 3, 21, "春分"}, {2018, 4, 5, "清明"}, {2018, 4, 20, "谷雨"},
    {2018, 5, 5, "立夏"}, {2018, 5, 21, "小满"}, {2018, 6, 6, "芒种"}, {2018, 6, 21, "夏至"},
    {2018, 7, 7, "小暑"}, {2018, 7, 23, "大暑"}, {2018, 8, 7, "立秋"}, {2018, 8, 23, "处暑"},
    {2018, 9, 8, "白露"}, {2018, 9, 23, "秋分"}, {2018, 10, 8, "寒露"}, {2018, 10, 23, "霜降"},
    {2018, 11, 7, "立冬"}, {2018, 11, 22, "小雪"}, {2018, 12, 7, "大雪"}, {2018, 12, 22, "冬至"},
    {2019, 1, 5, "小寒"}, {2019, 1, 20, "大寒"}, {2019, 2, 4, "立春"}, {2019, 2, 19, "雨水"},
    {2019, 3, 6, "惊蛰"}, {2019, 3, 21, "春分"}, {2019, 4, 5, "清明"}, {2019, 4, 20, "谷雨"},
    {2019, 5, 6, "立夏"}, {2019, 5, 21, "小满"}, {2019, 6, 6, "芒种"}, {2019, 6, 21, "夏至"},
    {2019, 7, 7, "小暑"}, {2019, 7, 23, "大暑"}, {2019, 8, 8, "立秋"}, {2019, 8, 23, "处暑"},
    {2019, 9, 8, "白露"}, {2019, 9, 23, "秋分"}, {2019, 10, 8, "寒露"}, {2019, 10, 24, "霜降"},
    {2019, 11, 8, "立冬"}, {2019, 11, 22, "小雪"}, {2019, 12, 7, "大雪"}, {2019, 12, 22, "冬至"},
    {2020, 1, 6, "小寒"}, {2020, 1, 20, "大寒"}, {2020, 2, 4, "立春"}, {2020, 2, 19, "雨水"},
    {2020, 3, 5, "惊蛰"}, {2020, 3, 20, "春分"}, {2020, 4, 4, "清明"}, {2020, 4, 19, "谷雨"},
    {2020, 5, 5, "立夏"}, {2020, 5, 20, "小满"}, {2020, 6, 5, "芒种"}, {2020, 6, 21, "夏至"},
    {2020, 7, 6, "小暑"}, {2020, 7, 22, "大暑"}, {2020, 8, 7, "立秋"}, {2020, 8, 22, "处暑"},
    {2020, 9, 7, "白露"}, {2020, 9, 22, "秋分"}, {2020, 10, 8, "寒露"}, {2020, 10, 23, "霜降"},
    {2020, 11, 7, "立冬"}, {2020, 11, 22, "小雪"}, {2020, 12, 7, "大雪"}, {2020, 12, 21, "冬至"},
    {2021, 1, 5, "小寒"}, {2021, 1, 20, "大寒"}, {2021, 2, 3, "立春"}, {2021, 2, 18, "雨水"},
    {2021, 3, 5, "惊蛰"}, {2021, 3, 20, "春分"}, {2021, 4, 4, "清明"}, {2021, 4, 20, "谷雨"},
    {2021, 5, 5, "立夏"}, {2021, 5, 21, "小满"}, {2021, 6, 5, "芒种"}, {2021, 6, 21, "夏至"},
    {2021, 7, 7, "小暑"}, {2021, 7, 22, "大暑"}, {2021, 8, 7, "立秋"}, {2021, 8, 23, "处暑"},
    {2021, 9, 7, "白露"}, {2021, 9, 23, "秋分"}, {2021, 10, 8, "寒露"}, {2021, 10, 23, "霜降"},
    {2021, 11, 7, "立冬"}, {2021, 11, 22, "小雪"}, {2021, 12, 7, "大雪"}, {2021, 12, 21, "冬至"},
    {2022, 1, 5, "小寒"}, {2022, 1, 20, "大寒"}, {2022, 2, 4, "立春"}, {2022, 2, 19, "雨水"},
    {2022, 3, 5, "惊蛰"}, {2022, 3, 20, "春分"}, {2022, 4, 5, "清明"}, {2022, 4, 20, "谷雨"},
    {2022, 5, 5, "立夏"}, {2022, 5, 21, "小满"}, {2022, 6, 6, "芒种"}, {2022, 6, 21, "夏至"},
    {2022, 7, 7, "小暑"}, {2022, 7, 23, "大暑"}, {2022, 8, 7, "立秋"}, {2022, 8, 23, "处暑"},
    {2022, 9, 7, "白露"}, {2022, 9, 23, "秋分"}, {2022, 10, 8, "寒露"}, {2022, 10, 23, "霜降"},
    {2022, 11, 7, "立冬"}, {2022, 11, 22, "小雪"}, {2022, 12, 7, "大雪"}, {2022, 12, 22, "冬至"},
    {2023, 1, 5, "小寒"}, {2023, 1, 20, "大寒"}, {2023, 2, 4, "立春"}, {2023, 2, 19, "雨水"},
    {2023, 3, 6, "惊蛰"}, {2023, 3, 21, "春分"}, {2023, 4, 5, "清明"}, {2023, 4, 20, "谷雨"},
    {2023, 5, 6, "立夏"}, {2023, 5, 21, "小满"}, {2023, 6, 6, "芒种"}, {2023, 6, 21, "夏至"},
    {2023, 7, 7, "小暑"}, {2023, 7, 23, "大暑"}, {2023, 8, 8, "立秋"}, {2023, 8, 23, "处暑"},
    {2023, 9, 8, "白露"}, {2023, 9, 23, "秋分"}, {2023, 10, 8, "寒露"}, {2023, 10, 24, "霜降"},
    {2023, 11, 8, "立冬"}, {2023, 11, 22, "小雪"}, {2023, 12, 7, "大雪"}, {2023, 12, 22, "冬至"},
    {2024, 1, 6, "小寒"}, {2024, 1, 20, "大寒"}, {2024, 2, 4, "立春"}, {2024, 2, 19, "雨水"},
    {2024, 3, 5, "惊蛰"}, {2024, 3, 20, "春分"}, {2024, 4, 4, "清明"}, {2024, 4, 19, "谷雨"},
    {2024, 5, 5, "立夏"}, {2024, 5, 20, "小满"}, {2024, 6, 5, "芒种"}, {2024, 6, 21, "夏至"},
    {2024, 7, 6, "小暑"}, {2024, 7, 22, "大暑"}, {2024, 8, 7, "立秋"}, {2024, 8, 22, "处暑"},
    {2024, 9, 7, "白露"}, {2024, 9, 22, "秋分"}, {2024, 10, 8, "寒露"}, {2024, 10, 23, "霜降"},
    {2024, 11, 7, "立冬"}, {2024, 11, 22, "小雪"}, {2024, 12, 7, "大雪"}, {2024, 12, 21, "冬至"},
    {2025, 1, 5, "小寒"}, {2025, 1, 20, "大寒"}, {2025, 2, 3, "立春"}, {2025, 2, 18, "雨水"},
    {2025, 3, 5, "惊蛰"}, {2025, 3, 20, "春分"}, {2025, 4, 4, "清明"}, {2025, 4, 20, "谷雨"},
    {2025, 5, 5, "立夏"}, {2025, 5, 21, "小满"}, {2025, 6, 5, "芒种"}, {2025, 6, 21, "夏至"},
    {2025, 7, 7, "小暑"}, {2025, 7, 22, "大暑"}, {2025, 8, 7, "立秋"}, {2025, 8, 23, "处暑"},
    {2025, 9, 7, "白露"}, {2025, 9, 23, "秋分"}, {2025, 10, 8, "寒露"}, {2025, 10, 23, "霜降"},
    {2025, 11, 7, "立冬"}, {2025, 11, 22, "小雪"}, {2025, 12, 7, "大雪"}, {2025, 12, 21, "冬至"},
    {2026, 1, 5, "小寒"}, {2026, 1, 20, "大寒"}, {2026, 2, 4, "立春"}, {2026, 2, 19, "雨水"},
    {2026, 3, 6, "惊蛰"}, {2026, 3, 21, "春分"}, {2026, 4, 5, "清明"}, {2026, 4, 20, "谷雨"},
    {2026, 5, 6, "立夏"}, {2026, 5, 21, "小满"}, {2026, 6, 6, "芒种"}, {2026, 6, 21, "夏至"},
    {2026, 7, 7, "小暑"}, {2026, 7, 23, "大暑"}, {2026, 8, 8, "立秋"}, {2026, 8, 23, "处暑"},
    {2026, 9, 8, "白露"}, {2026, 9, 24, "秋分"}, {2026, 10, 8, "寒露"}, {2026, 10, 23, "霜降"},
    {2026, 11, 8, "立冬"}, {2026, 11, 23, "小雪"}, {2026, 12, 7, "大雪"}, {2026, 12, 22, "冬至"},
    {2027, 1, 6, "小寒"}, {2027, 1, 20, "大寒"}, {2027, 2, 4, "立春"}, {2027, 2, 19, "雨水"},
    {2027, 3, 6, "惊蛰"}, {2027, 3, 21, "春分"}, {2027, 4, 5, "清明"}, {2027, 4, 20, "谷雨"},
    {2027, 5, 6, "立夏"}, {2027, 5, 21, "小满"}, {2027, 6, 6, "芒种"}, {2027, 6, 22, "夏至"},
    {2027, 7, 7, "小暑"}, {2027, 7, 23, "大暑"}, {2027, 8, 8, "立秋"}, {2027, 8, 24, "处暑"},
    {2027, 9, 8, "白露"}, {2027, 9, 24, "秋分"}, {2027, 10, 8, "寒露"}, {2027, 10, 24, "霜降"},
    {2027, 11, 8, "立冬"}, {2027, 11, 23, "小雪"}, {2027, 12, 7, "大雪"}, {2027, 12, 22, "冬至"},
    {2028, 1, 6, "小寒"}, {2028, 1, 21, "大寒"}, {2028, 2, 4, "立春"}, {2028, 2, 19, "雨水"},
    {2028, 3, 5, "惊蛰"}, {2028, 3, 20, "春分"}, {2028, 4, 4, "清明"}, {2028, 4, 19, "谷雨"},
    {2028, 5, 5, "立夏"}, {2028, 5, 20, "小满"}, {2028, 6, 5, "芒种"}, {2028, 6, 20, "夏至"},
    {2028, 7, 6, "小暑"}, {2028, 7, 22, "大暑"}, {2028, 8, 7, "立秋"}, {2028, 8, 22, "处暑"},
    {2028, 9, 7, "白露"}, {2028, 9, 22, "秋分"}, {2028, 10, 7, "寒露"}, {2028, 10, 23, "霜降"},
    {2028, 11, 7, "立冬"}, {2028, 11, 22, "小雪"}, {2028, 12, 6, "大雪"}, {2028, 12, 21, "冬至"},
    {2029, 1, 6, "小寒"}, {2029, 1, 21, "大寒"}, {2029, 2, 3, "立春"}, {2029, 2, 18, "雨水"},
    {2029, 3, 5, "惊蛰"}, {2029, 3, 20, "春分"}, {2029, 4, 4, "清明"}, {2029, 4, 20, "谷雨"},
    {2029, 5, 5, "立夏"}, {2029, 5, 21, "小满"}, {2029, 6, 5, "芒种"}, {2029, 6, 21, "夏至"},
    {2029, 7, 7, "小暑"}, {2029, 7, 22, "大暑"}, {2029, 8, 7, "立秋"}, {2029, 8, 23, "处暑"},
    {2029, 9, 7, "白露"}, {2029, 9, 23, "秋分"}, {2029, 10, 8, "寒露"}, {2029, 10, 23, "霜降"},
    {2029, 11, 7, "立冬"}, {2029, 11, 22, "小雪"}, {2029, 12, 7, "大雪"}, {2029, 12, 21, "冬至"},
    {2030, 1, 5, "小寒"}, {2030, 1, 20, "大寒"}, {2030, 2, 3, "立春"}, {2030, 2, 18, "雨水"},
    {2030, 3, 5, "惊蛰"}, {2030, 3, 20, "春分"}, {2030, 4, 5, "清明"}, {2030, 4, 20, "谷雨"},
    {2030, 5, 5, "立夏"}, {2030, 5, 21, "小满"}, {2030, 6, 5, "芒种"}, {2030, 6, 21, "夏至"},
    {2030, 7, 7, "小暑"}, {2030, 7, 22, "大暑"}, {2030, 8, 7, "立秋"}, {2030, 8, 23, "处暑"},
    {2030, 9, 7, "白露"}, {2030, 9, 23, "秋分"}, {2030, 10, 8, "寒露"}, {2030, 10, 23, "霜降"},
    {2030, 11, 7, "立冬"}, {2030, 11, 22, "小雪"}, {2030, 12, 7, "大雪"}, {2030, 12, 21, "冬至"},
};

static const int SOLAR_TERMS_COUNT = sizeof(solarTermsData) / sizeof(solarTermsData[0]);

QString LunarCalendar::getSolarTerm(const QDate &date) const
{
    int targetYear = date.year();
    int targetMonth = date.month();
    int targetDay = date.day();
    
    for (int i = 0; i < SOLAR_TERMS_COUNT; ++i) {
        if (solarTermsData[i].year == targetYear &&
            solarTermsData[i].month == targetMonth &&
            solarTermsData[i].day == targetDay) {
            return QString::fromUtf8(solarTermsData[i].name);
        }
    }
    
    return "";
}
