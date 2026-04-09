# 个人日程助手 (PersonalDateAssisant) - 项目逻辑文档

## 一、项目概述

### 1.1 项目简介

**项目名称**: 个人日程助手 (PersonalDateAssisant)  
**项目类型**: Qt桌面应用程序  
**主要功能**: 个人日程管理、日历展示、农历转换、天气预报、提醒通知、批量日程管理  
**目标用户**: 需要日程管理、查看农历信息和天气预报的个人用户  

### 1.2 技术栈

- **框架**: Qt5/Qt6 (支持跨平台)
- **构建系统**: CMake/Qt Creator
- **数据库**: SQLite
- **网络**: Qt Network模块 (高德天气API集成)
- **UI框架**: Qt Widgets + Qt Designer UI文件
- **图标库**: Font Awesome (通过QtAwesome库集成)

### 1.3 项目目录结构

```
DataAssistant/
├── logic/                    # 本文档所在目录
│   └── PROJECT_LOGIC_DOCUMENTATION.md
├── lib/
│   └── QtAwesome/            # Font Awesome图标库
│       ├── QtAwesome.h/cpp   # 核心图标类
│       ├── QtAwesomeAnim.h/cpp # 动画支持
│       ├── fonts/            # 字体文件
│       └── CMakeLists.txt
├── scripts/                  # 构建和部署脚本
│   ├── build.bat            # 编译脚本
│   ├── deploy.bat           # 部署脚本
│   ├── package.bat          # 打包脚本
│   └── installer.iss         # InnoSetup安装包配置
├── doc/                     # 项目文档
│   ├── 批量添加日程功能实现文档.md
│   ├── 批量日程合并显示功能说明.md
│   ├── 批量删除日程功能说明.md
│   └── ... (其他功能文档)
├── .trae/                   # AI助手配置
├── ReleaseTemp/             # 发布临时文件
│   └── deploy/              # 部署目录
│       └── PersonalDateAssisant.exe
├── Widget.h/cpp             # 主窗口
├── Widget.ui                # 主窗口UI定义
├── DatabaseManager.h/cpp     # 数据库管理（支持批量日程）
├── ScheduleDialog.h/cpp/ui  # 日程对话框（支持批量添加）
├── ScheduleDetailDialog.h/cpp # 日程详情对话框
├── ScheduleListDialog.h/cpp  # 日程列表对话框（支持批量选择和删除）
├── SettingsDialog.h/cpp/ui  # 设置对话框
├── ReminderManager.h/cpp    # 提醒管理
├── WeatherManager.h/cpp     # 天气管理（高德API）
├── CalendarDelegate.h/cpp   # 日历代理
├── LunarCalendar.h/cpp      # 农历转换
├── main.cpp                 # 应用程序入口
└── CMakeLists.txt           # CMake构建配置
```

---

## 二、系统架构

### 2.1 架构分层图

```
┌─────────────────────────────────────────────────────────────┐
│                      UI Layer (展示层)                        │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────┐ │
│  │  Widget      │ │ ScheduleDialog│ │ ScheduleListDialog   │ │
│  │ (主窗口)     │ │ (日程对话框)  │ │ (日程列表对话框)     │ │
│  │              │ │ - 批量添加    │ │ - 批量选择           │ │
│  │              │ │ - 日期范围    │ │ - 批量删除           │ │
│  └──────────────┘ └──────────────┘ └──────────────────────┘ │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────┐ │
│  │SettingsDialog│ │ScheduleDetail │ │ CalendarDelegate      │ │
│  │ (设置对话框) │ │ Dialog        │ │ (日历代理)            │ │
│  └──────────────┘ └──────────────┘ └──────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   Business Logic Layer (业务逻辑层)           │
│  ┌──────────────┐ ┌──────────────┐ ┌────────────────────┐  │
│  │ReminderManager│ │WeatherManager │ │LunarCalendar        │  │
│  │ (提醒管理)    │ │ (天气管理)    │ │ (农历转换)          │  │
│  └──────────────┘ └──────────────┘ └────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Data Access Layer (数据访问层)            │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              DatabaseManager (数据库管理)              │   │
│  │  - SQLite数据库操作                                    │   │
│  │  - 批量日程CRUD操作                                   │   │
│  │  - 数据备份/恢复                                      │   │
│  │  - 单一共享数据库连接                                  │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Data Storage (数据存储层)                 │
│  ┌──────────────┐ ┌──────────────┐ ┌────────────────────┐  │
│  │ schedules.db │ │weather_setting│ │ (AppDataLocation)   │  │
│  │ (日程数据库) │ │ s.ini        │ │                     │  │
│  │              │ │ (天气设置)   │ │                     │  │
│  └──────────────┘ └──────────────┘ └────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 核心组件交互关系

```
┌──────────────────────────────────────────────────────────────────┐
│                         main.cpp                                  │
│                     (应用程序入口)                                │
└──────────────────────────┬───────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────────┐
│                         Widget                                   │
│                     (主窗口类)                                    │
│  ┌────────────────┬────────────────┬────────────────┬───────────┐ │
│  │ QCalendarWidget │ WeatherManager │ ReminderManager│ Database  │ │
│  │ (日历组件)      │ (天气管理)     │ (提醒管理)      │ Manager   │ │
│  └───────┬────────┴───────┬────────┴───────┬────────┴─────┬─────┘ │
│          │                │                 │              │       │
│          ▼                ▼                 ▼              ▼       │
│  ┌────────────┐   ┌────────────┐   ┌────────────┐  ┌──────────┐ │
│  │CalendarDel │   │ 高德天气API│   │ QMessageBox│  │  SQLite   │ │
│  │ egate      │   │ (网络请求) │   │ (提醒弹窗)  │  │  Database │ │
│  └────────────┘   └────────────┘   └────────────┘  └──────────┘ │
└──────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────┐
│                    ScheduleListDialog                             │
│              (日程列表对话框 - 批量管理)                           │
│  ┌────────────────┬────────────────┬────────────────┬───────────┐ │
│  │ 批量选择复选框  │ 批量删除按钮   │ 批量日程合并   │ Database  │ │
│  │ (QCheckBox)     │ (QPushButton) │ 显示           │ Manager   │ │
│  └────────────────┴────────────────┴────────────────┴───────────┘ │
└──────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────┐
│                     ScheduleDialog                                │
│                (日程对话框 - 批量添加)                            │
│  ┌────────────────┬────────────────┬────────────────┬───────────┐ │
│  │ 批量添加复选框 │ 日期范围选择   │ 开始/结束日期  │ Database  │ │
│  │ (QCheckBox)    │ (QDateTimeEdit)│ (开始-结束)    │ Manager   │ │
│  └────────────────┴────────────────┴────────────────┴───────────┘ │
└──────────────────────────────────────────────────────────────────┘
```

---

## 三、数据结构

### 3.1 Schedule 结构体

```cpp
struct Schedule {
    int id;                      // 日程唯一标识符
    QString title;               // 日程标题
    QString description;         // 日程描述
    QDateTime datetime;          // 日程开始日期时间
    QDateTime endDatetime;       // 批量添加的结束日期时间（单日程时与datetime相同）
    bool isBatch;                // 是否为批量添加的日程（0:否, 1:是）
    int priority;                // 优先级: 0=一般, 1=重要, 2=紧急
    int reminderMinutes;         // 提前提醒分钟数
    bool completed;              // 是否已完成
    QColor color;                // 日程颜色，根据priority自动设置
};
```

**优先级颜色映射**:
- 0 (一般): 蓝色 `#0000FF`
- 1 (重要): 橙色 `rgb(255, 165, 0)`
- 2 (紧急): 红色 `#FF0000`

### 3.2 WeatherInfo 结构体

```cpp
struct WeatherInfo {
    QString city;               // 城市名称
    QString temperature;        // 当前温度 (如 "25°C")
    QString condition;          // 天气状况 (带emoji)
    QString wind;               // 风力
    QString temperatureRange;  // 温度区间 (如 "18°C~25°C")
    QString forecast[3];       // 未来3天预报
};
```

### 3.3 LunarDate 结构体

```cpp
struct LunarDate {
    int year;                   // 农历年
    int month;                  // 农历月 (1-12)
    int day;                    // 农历日 (1-30)
    bool isLeap;               // 是否闰月
};
```

### 3.4 DeleteResult 结构体（批量删除专用）

```cpp
struct DeleteResult {
    int totalSchedules;         // 将要/实际删除的日程总数
    int batchCount;            // 涉及的批量日程批次数
    int scheduleCount;         // 涉及的普通日程数
    int actualDeleted;         // 实际删除的日程数
    QStringList batchDetails;   // 每个批量日程的详细信息
};
```

### 3.5 数据库表结构

**表名**: `schedules`

| 字段名 | 类型 | 说明 |
|--------|------|------|
| id | INTEGER | 主键，自增 |
| title | TEXT | 日程标题，必填 |
| description | TEXT | 日程描述 |
| datetime | TEXT | 开始日期时间 (ISO格式) |
| endDatetime | TEXT | 结束日期时间 (ISO格式，批量日程使用) |
| isBatch | INTEGER | 是否批量日程 (0:否, 1:是) |
| priority | INTEGER | 优先级 (0-2) |
| reminderMinutes | INTEGER | 提前提醒分钟数 |
| completed | INTEGER | 是否完成 (0/1) |
| color | TEXT | 颜色值 (十六进制) |

**数据库路径**: `QStandardPaths::AppDataLocation/schedules.db`

**数据库连接**: 使用单一共享连接，避免多连接冲突

---

## 四、文件详细文档

### 4.1 main.cpp

**文件路径**: `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\main.cpp`  
**功能**: 应用程序入口点  
**核心逻辑**:

```cpp
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);    // 创建Qt应用程序对象
    Widget w;                      // 创建主窗口实例
    w.show();                      // 显示主窗口
    return a.exec();              // 进入事件循环
}
```

**依赖关系**: 无外部依赖，仅包含Widget.h

---

### 4.2 Widget.h / Widget.cpp

**文件路径**: 
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\Widget.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\Widget.cpp`

**功能**: 主窗口类，管理整个应用程序的主要UI和业务逻辑

**类定义**:
```cpp
class Widget : public QWidget {
    Q_OBJECT
    
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    DatabaseManager *dbManager;
    ReminderManager *reminderManager;
    WeatherManager *weatherManager;
    QtAwesome *m_awesome;
    QCalendarWidget *m_calendarWidget;
    CalendarDelegate *m_calendarDelegate;
    QTableView* m_calendarTableView;
    LunarCalendar *m_lunarCalendar;
};
```

**核心功能**:

1. **setupCalendar()** - 配置日历样式和行为
   - 设置日历显示网格
   - 隐藏垂直表头
   - 应用自定义样式表
   - 设置CalendarDelegate

2. **updateCalendarMarks()** - 更新日历标记
   - 获取当前显示月份
   - 从数据库加载日程数据
   - 更新delegate的日程和今天标记
   - 刷新日历视图

3. **updateWeatherDisplay()** - 更新天气显示
   - 从WeatherManager获取天气信息
   - 更新UI标签显示

4. **updateDateInfo()** - 更新日期信息
   - 显示当前日期
   - 显示农历日期
   - 显示节气信息

5. **handleCalendarClick(const QDate& date)** - 处理日历点击
   - 获取指定日期的日程
   - 打开ScheduleDetailDialog显示详情

**信号槽连接**:
- `weatherManager::weatherUpdated` → `Widget::updateWeatherDisplay`
- `weatherManager::errorOccurred` → 错误提示
- `calendarDelegate::dateClicked` → `Widget::onDateClickedFromDelegate`
- `m_calendarWidget::currentPageChanged` → `updateCalendarMarks`

---

### 4.3 CalendarDelegate.h / CalendarDelegate.cpp

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\CalendarDelegate.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\CalendarDelegate.cpp`

**功能**: 自定义日历单元格代理，控制日历单元格的渲染和交互

**类定义**:
```cpp
class CalendarDelegate : public QStyledItemDelegate {
    Q_OBJECT
    
public:
    CalendarDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
               const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, 
                     const QStyleOptionViewItem& option, 
                     const QModelIndex& index) override;
    
signals:
    void dateClicked(const QDate& date);
};
```

**渲染算法 (paint方法)**:

```
输入: 日期(date), 优先级(maxPriority), 是否今天(isToday)
输出: 绘制背景色和日期文字

1. 判断日期是否属于当前月份
   if date.month != m_currentMonth:
       背景色 = #EEEEEE (灰色)
       文字颜色 = 半透明黑色
   else:
       进入步骤2

2. 根据优先级设置背景色:
   if isToday:
       边框 = 2px实线黑色
       字体加粗
       if priority == 2: 背景色 = #FFB4B4 (淡红色)
       else if priority == 1: 背景色 = #FFFFB4 (淡黄色)
       else if priority == 0: 背景色 = #B4C8FF (淡蓝色)
       else: 背景色 = #F5F5F5 (浅灰)
   else if priority == 2: 背景色 = #FFB4B4
   else if priority == 1: 背景色 = #FFFFB4
   else if priority == 0: 背景色 = #B4C8FF
   else: 背景色 = #FAFAFA

3. 周六周日设置红色文字
   if dayOfWeek in [6, 7]:
       文字颜色 = #C83232

4. 绘制日期数字
```

**日期索引计算**:
```cpp
QDate CalendarDelegate::dateFromIndex(const QModelIndex& index) const {
    // 计算方法:
    // 1. 获取当月第一天的星期几
    int firstDayOffset = (QDate(year, month, 1).dayOfWeek() - 1 + 7) % 7;
    
    // 2. 计算单元格对应的天数偏移
    int dayOffset = row * 7 + column;
    
    // 3. 计算实际日期
    QDate date = QDate(year, month, 1).addDays(dayOffset - firstDayOffset);
    
    return date;
}
```

---

### 4.4 DatabaseManager.h / DatabaseManager.cpp

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\DatabaseManager.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\DatabaseManager.cpp`

**功能**: SQLite数据库管理器，负责所有日程数据的CRUD操作，支持批量日程管理

**数据库路径**: `QStandardPaths::AppDataLocation/schedules.db`

**数据库连接管理**:
```cpp
// 使用单一共享连接，避免多连接冲突
static QSqlDatabase sharedDb;
static bool dbInitialized = false;

static QSqlDatabase getDatabaseConnection(const QString& connectionName = QString()) {
    if (!dbInitialized) {
        sharedDb = QSqlDatabase::addDatabase("QSQLITE");
        sharedDb.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/schedules.db");
        sharedDb.open();
        dbInitialized = true;
    }
    return sharedDb;
}
```

**核心方法**:

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `addSchedule` | const Schedule& | bool | 添加单个日程 |
| `addBatchSchedules` | const QVector<Schedule>& | bool | 添加批量日程 |
| `updateSchedule` | const Schedule& | bool | 更新日程 |
| `deleteSchedule` | int id | bool | 删除单个日程 |
| `deleteBatchSchedules` | const QString& batchGroupId | bool | 删除批量日程组 |
| `deleteScheduleWithBatch` | int id | bool | 删除日程及其同批次日程 |
| `deleteAllSchedules` | void | bool | 删除所有日程 |
| `deleteSchedulesByIds` | const QVector<int>& ids | int | 批量删除日程 |
| `getDeletePreview` | const QVector<int>& ids | DeleteResult | 获取删除预览信息 |
| `deleteSchedulesWithDetails` | const QVector<int>& ids | DeleteResult | 批量删除并返回详情 |
| `getSchedules` | void | QVector<Schedule> | 获取所有日程 |
| `getSchedulesByDate` | const QDate& | QVector<Schedule> | 按日期获取日程 |
| `getScheduleById` | int id | Schedule | 按ID获取日程 |
| `getBatchSchedulesByGroupId` | const QString& groupId | QVector<Schedule> | 按组ID获取批量日程 |
| `backupDatabase` | const QString& | bool | 备份数据库 |
| `restoreDatabase` | const QString& | bool | 恢复数据库 |

**SQL语句模板**:

```sql
-- 创建表（支持批量日程）
CREATE TABLE IF NOT EXISTS schedules (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    description TEXT,
    datetime TEXT NOT NULL,
    endDatetime TEXT DEFAULT '',
    isBatch INTEGER DEFAULT 0,
    priority INTEGER DEFAULT 0,
    reminderMinutes INTEGER DEFAULT 0,
    completed INTEGER DEFAULT 0,
    color TEXT DEFAULT ''
);

-- 添加单个日程
INSERT INTO schedules (title, description, datetime, endDatetime, isBatch, priority, reminderMinutes, completed, color) 
VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);

-- 添加批量日程（多条记录，isBatch=1）
INSERT INTO schedules (title, description, datetime, endDatetime, isBatch, priority, reminderMinutes, completed, color) 
VALUES (?, ?, ?, ?, 1, ?, ?, ?, ?);

-- 更新日程
UPDATE schedules SET title=?, description=?, datetime=?, endDatetime=?, 
                     isBatch=?, priority=?, reminderMinutes=?, completed=?, color=? 
WHERE id=?;

-- 删除单个日程
DELETE FROM schedules WHERE id=?;

-- 删除批量日程（整个批次）
DELETE FROM schedules WHERE isBatch=1 AND title=? AND endDatetime=? AND priority=?;

-- 查询所有日程
SELECT * FROM schedules ORDER BY datetime ASC;

-- 查询日期范围内的日程
SELECT * FROM schedules WHERE (datetime >= ? AND datetime <= ?) 
   OR (endDatetime >= ? AND endDatetime <= ?) 
   OR (datetime <= ? AND endDatetime >= ?) 
ORDER BY datetime ASC;

-- 查询批量日程
SELECT * FROM schedules WHERE isBatch=1 ORDER BY datetime ASC;
```

**批量日程删除逻辑**:

```cpp
DeleteResult DatabaseManager::deleteSchedulesWithDetails(const QVector<int>& ids) {
    // 1. 对每个选中的ID
    for (int id : ids) {
        // 2. 查询该日程的详细信息
        Schedule schedule = getScheduleById(id);
        
        // 3. 判断是否为批量日程
        if (schedule.isBatch || (schedule.endDatetime > schedule.datetime)) {
            // 4. 批量日程：删除整个批次
            // 关键：不使用datetime条件，因为每天的datetime不同
            DELETE FROM schedules 
            WHERE isBatch = 1 
              AND title = ? 
              AND endDatetime = ? 
              AND priority = ?;
        } else {
            // 5. 普通日程：只删除当前记录
            DELETE FROM schedules WHERE id = ?;
        }
    }
}
```

---

### 4.5 ScheduleDialog.h / ScheduleDialog.cpp / ScheduleDialog.ui

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleDialog.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleDialog.cpp`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleDialog.ui`

**功能**: 日程创建/编辑对话框，支持批量添加模式

**UI组件**:
- `titleLineEdit` - 标题输入框
- `descriptionTextEdit` - 描述文本框
- `batchAddCheckBox` - 批量添加复选框
- `startDateTimeEdit` - 开始日期时间选择器（批量模式）
- `endDateTimeEdit` - 结束日期时间选择器（批量模式）
- `dateTimeEdit` - 日期时间选择器（单日期模式）
- `priorityComboBox` - 优先级下拉框 (0: 一般, 1: 重要, 2: 紧急)
- `reminderComboBox` - 提醒时间下拉框
  - 0: 不提醒
  - 1: 提前5分钟
  - 2: 提前15分钟
  - 3: 提前30分钟
  - 4: 提前1小时
  - 5: 提前1天
- `saveButton` - 保存按钮
- `cancelButton` - 取消按钮

**业务逻辑**:

1. **批量添加模式**
   ```cpp
   if (ui->batchAddCheckBox->isChecked()) {
       // 1. 验证日期范围
       if (!validateBatchDates()) return;
       
       // 2. 获取开始和结束日期
       QDateTime startDate = ui->startDateTimeEdit->dateTime();
       QDateTime endDate = ui->endDateTimeEdit->dateTime();
       
       // 3. 创建每天的日程
       QVector<Schedule> schedules;
       QDateTime currentDate = startDate;
       
       while (currentDate <= endDate) {
           Schedule schedule;
           schedule.datetime = currentDate;
           schedule.endDatetime = endDate;  // 所有记录的endDatetime相同
           schedule.isBatch = true;          // 标记为批量日程
           schedule.title = title;
           // ... 其他字段
           
           schedules.append(schedule);
           currentDate = currentDate.addDays(1);
       }
       
       // 4. 批量插入数据库
       dbManager.addBatchSchedules(schedules);
   }
   ```

2. **编辑模式下填充现有数据**
3. **新建模式默认时间为当天8:00**
4. **保存时验证标题非空**
5. **根据优先级自动设置颜色**

**日期范围验证**:
```cpp
bool ScheduleDialog::validateBatchDates() {
    QDateTime startDate = ui->startDateTimeEdit->dateTime();
    QDateTime endDate = ui->endDateTimeEdit->dateTime();
    
    if (endDate < startDate) {
        QMessageBox::warning(this, "错误", "结束日期不能早于开始日期");
        return false;
    }
    
    if (startDate.daysTo(endDate) > 365) {
        QMessageBox::warning(this, "错误", "日期范围不能超过365天");
        return false;
    }
    
    return true;
}
```

---

### 4.6 ScheduleDetailDialog.h / ScheduleDetailDialog.cpp

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleDetailDialog.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleDetailDialog.cpp`

**功能**: 显示指定日期的日程详情

**特性**:
1. **点击外部自动关闭**: 通过eventFilter实现
2. **淡出动画**: 使用QPropertyAnimation实现关闭动画
3. **卡片式布局**: 每个日程显示为独立的卡片
4. **操作按钮**: 每个日程卡片包含编辑和删除按钮
5. **批量日程识别**: 识别并特殊显示批量日程

**关键实现**:

```cpp
bool ScheduleDetailDialog::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        // 检查点击位置是否在对话框外部
        if (!rect().contains(mapFromGlobal(mouseEvent->globalPosition().toPoint()))) {
            animateClose();
            return true;
        }
    }
    return false;
}

void ScheduleDetailDialog::animateClose() {
    m_closeAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_closeAnimation->setDuration(200);
    m_closeAnimation->setStartValue(1.0);
    m_closeAnimation->setEndValue(0.0);
    m_closeAnimation->start();
}
```

---

### 4.7 ScheduleListDialog.h / ScheduleListDialog.cpp

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp`

**功能**: 显示所有日程的列表视图，支持批量选择和批量删除

**特性**:
1. **全局日程视图**: 显示所有日程，无日期分组
2. **批量日程合并显示**: 同一批次的多个日程合并为1个条目显示
3. **批量选择**: 使用复选框选择多个日程
4. **批量删除**: 一键删除选中的所有日程
5. **最小化/展开**: 每个日程项可单独最小化
6. **平滑动画**: 最小化/展开时有动画效果
7. **排序**: 按日期时间升序排列
8. **删除预览**: 显示将要删除的日程统计信息

**核心数据结构**:
```cpp
class ScheduleListDialog : public QDialog {
    // ... 其他成员
    
private:
    QVBoxLayout* m_mainLayout;
    QWidget* m_contentWidget;
    QWidget* m_batchActionBar;        // 批量操作工具栏
    QPushButton* m_batchDeleteButton; // 批量删除按钮
    QPushButton* m_selectAllButton;   // 全选按钮
    QPushButton* m_deselectAllButton;  // 取消全选按钮
    QLabel* m_selectionCountLabel;     // 选择计数标签
    
    DatabaseManager* dbManager;
    QMap<int, bool> m_minimizedState;         // 存储每个日程项的最小化状态
    QMap<int, QPropertyAnimation*> m_animations;  // 存储动画对象
    QSet<int> m_selectedScheduleIds;           // 存储选中的日程ID
    QMap<int, QWidget*> m_scheduleItemWidgets; // 日程ID到UI组件的映射
};
```

**批量日程合并显示逻辑**:

```cpp
QWidget* ScheduleListDialog::createScheduleItem(const Schedule& schedule) {
    if (schedule.isBatch) {
        // 批量日程：显示为单个条目
        // 标题显示：schedule.title
        // 详情显示：schedule.datetime 至 schedule.endDatetime
        
        // 查询该批次实际包含多少天
        QVector<Schedule> batchSchedules = dbManager->getBatchSchedulesByGroupId(
            schedule.datetime.toString(Qt::ISODate)
        );
        
        // 显示："项目周 (5天: 04/01 至 04/05)"
        QString detail = QString("%1 (%2天: %3 至 %4)")
            .arg(schedule.title)
            .arg(batchSchedules.size())
            .arg(schedule.datetime.toString("MM/dd"))
            .arg(schedule.endDatetime.toString("MM/dd"));
        
        // ... 创建UI组件
    } else {
        // 普通日程：正常显示
        // ...
    }
}
```

**批量选择和删除流程**:

```cpp
void ScheduleListDialog::onBatchDelete() {
    if (m_selectedScheduleIds.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的日程");
        return;
    }
    
    // 1. 获取删除预览
    QVector<int> ids;
    for (int id : m_selectedScheduleIds) {
        ids.append(id);
    }
    
    DatabaseManager::DeleteResult preview = dbManager->getDeletePreview(ids);
    
    // 2. 显示确认对话框
    QString message = QString("确定要删除以下日程吗？\n\n"
                             "📊 共计: %1 个日程\n"
                             "📅 批量日程: %2 批（共 %3 天）\n"
                             "📝 普通日程: %4 个\n\n")
        .arg(preview.totalSchedules)
        .arg(preview.batchCount)
        .arg(preview.scheduleCount)
        .arg(preview.totalSchedules - preview.scheduleCount);
    
    // ... 显示批次详情
    
    // 3. 用户确认后执行删除
    if (reply == QMessageBox::Yes) {
        DatabaseManager::DeleteResult result = dbManager->deleteSchedulesWithDetails(ids);
        
        // 4. 显示结果
        if (result.actualDeleted > 0) {
            QMessageBox::information(this, "删除成功", 
                QString("✅ 已成功删除 %1 个日程").arg(result.actualDeleted));
        }
        
        // 5. 刷新列表
        loadSchedules();
    }
}
```

---

### 4.8 SettingsDialog.h / SettingsDialog.cpp

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\SettingsDialog.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\SettingsDialog.cpp`

**功能**: 应用程序设置管理

**设置项**:
1. **天气设置**
   - 城市选择
   - API Key配置
   - 天气显示开关

2. **提醒设置**
   - 默认提醒时间
   - 提醒声音

3. **数据管理**
   - 备份数据库
   - 恢复数据库
   - 清空所有数据

**数据持久化**: 使用QSettings，存储在`weather_settings.ini`

---

### 4.9 ReminderManager.h / ReminderManager.cpp

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ReminderManager.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ReminderManager.cpp`

**功能**: 日程提醒管理

**提醒时间选项**:
- 不提醒
- 提前5分钟
- 提前15分钟
- 提前30分钟
- 提前1小时
- 提前1天

**实现方式**: 使用QTimer定时检查，或系统通知

---

### 4.10 WeatherManager.h / WeatherManager.cpp

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\WeatherManager.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\WeatherManager.cpp`

**功能**: 天气信息获取和显示

**数据源**: 高德天气API

**API格式**:
```
https://restapi.amap.com/v3/weather/weatherInfo?key=YOUR_API_KEY&city=CITY_CODE&extensions=all
```

**天气预报数据**:
- 当前天气
- 未来3-4天预报
- 温度范围
- 风力信息
- 天气状况

**信号**:
- `weatherUpdated(WeatherInfo)` - 天气数据更新
- `errorOccurred(QString)` - 错误发生

---

### 4.11 LunarCalendar.h / LunarCalendar.cpp

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\LunarCalendar.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\LunarCalendar.cpp`

**功能**: 农历日期转换

**功能**:
1. 阳历转农历
2. 显示农历年、月、日
3. 显示节气信息
4. 显示传统节日

**算法**: 基于查表法的农历转换算法

---

## 五、批量日程功能详解

### 5.1 批量日程的概念

**定义**: 用户在创建日程时，选择一个日期范围（如04/01-04/05），系统自动创建5天的日程，这5天日程视为一个"批次"。

**数据库存储**:
- 创建5条独立的数据库记录
- 每条记录的`datetime`不同（分别是5天的日期）
- 每条记录的`endDatetime`相同（都是04/05）
- 每条记录的`isBatch=1`
- 每条记录的`title`、`priority`等字段相同

**示例数据**:
| id | title | datetime | endDatetime | isBatch | priority |
|----|----|----------|-------------|---------|----------|
| 1 | 项目周 | 2026-04-01 09:00 | 2026-04-05 18:00 | 1 | 1 |
| 2 | 项目周 | 2026-04-02 09:00 | 2026-04-05 18:00 | 1 | 1 |
| 3 | 项目周 | 2026-04-03 09:00 | 2026-04-05 18:00 | 1 | 1 |
| 4 | 项目周 | 2026-04-04 09:00 | 2026-04-05 18:00 | 1 | 1 |
| 5 | 项目周 | 2026-04-05 09:00 | 2026-04-05 18:00 | 1 | 1 |

### 5.2 批量日程的显示

**列表视图**: 在`ScheduleListDialog`中，同一批次的日程合并为1个条目显示

**显示格式**: `项目周 (5天: 04/01 至 04/05)`

**日历视图**: 在`Widget`的日历中，每天单独显示该天的日程标记

### 5.3 批量日程的删除

**关键挑战**: 
- 批量日程在数据库中是5条独立的记录
- 用户在列表中只看到1个合并的条目
- 删除时需要删除所有5条记录，而不是只删除1条

**解决方案**:

```cpp
// 错误的删除方式（只删除1条）❌
DELETE FROM schedules 
WHERE isBatch = 1 
  AND title = ? 
  AND datetime = ?        // ❌ 问题：datetime每天都不同！
  AND endDatetime = ? 
  AND priority = ?;

// 正确的删除方式（删除整个批次）✅
DELETE FROM schedules 
WHERE isBatch = 1 
  AND title = ? 
  AND endDatetime = ?     // ✅ 正确：所有记录的endDatetime相同
  AND priority = ?;      // ✅ 不使用datetime条件
```

### 5.4 批量日程删除的用户体验

**1. 选择日程**
- 用户在列表中看到1个合并的批量日程条目
- 选中该条目（复选框打勾）
- 工具栏显示"已选择 1 项"

**2. 点击批量删除**
- 系统调用`getDeletePreview()`获取预览信息
- 显示确认对话框：
  ```
  确定要删除以下日程吗？
  
  📊 共计: 5 个日程
  📅 批量日程: 1 批（共 5 天）
  📝 普通日程: 0 个
  
  批量日程详情：
    • 项目周 (5天: 04/01 至 04/05)
  
  ⚠️ 此操作不可撤销！
  ```

**3. 确认删除**
- 用户点击"确定"
- 系统调用`deleteSchedulesWithDetails()`执行删除
- 删除所有5条记录
- 显示成功提示：
  ```
  ✅ 已成功删除 5 个日程
  
  批量日程: 1 批
  普通日程: 0 个
  ```

**4. 刷新列表**
- 日程列表刷新
- 批量日程的所有记录都已删除

---

## 六、数据库连接管理

### 6.1 问题背景

Qt的SQLite驱动在使用多个数据库连接时会出现问题：
- "duplicate connection name" 警告
- "database not open" 错误
- "connection still in use" 错误

### 6.2 解决方案：单一共享连接

```cpp
// 静态变量
static QSqlDatabase sharedDb;
static bool dbInitialized = false;

// 统一的连接获取方法
static QSqlDatabase getDatabaseConnection(const QString& connectionName = QString()) {
    Q_UNUSED(connectionName);
    
    if (!dbInitialized) {
        sharedDb = QSqlDatabase::addDatabase("QSQLITE");
        sharedDb.setDatabaseName(
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) 
            + "/schedules.db"
        );
        
        if (!sharedDb.open()) {
            qWarning() << "Failed to open database:" << sharedDb.lastError().text();
        }
        
        dbInitialized = true;
    }
    
    return sharedDb;
}
```

### 6.3 使用方式

```cpp
// 所有数据库操作都使用统一方法
bool DatabaseManager::addSchedule(const Schedule& schedule) {
    QSqlDatabase db = getDatabaseConnection();  // ✅ 使用共享连接
    
    QSqlQuery query;
    query.prepare("INSERT INTO schedules ...");
    // ...
    
    db.close();  // 注意：不要调用 removeDatabase()
    return true;
}
```

### 6.4 注意事项

1. **不要调用 `removeDatabase()`**: 共享连接不能移除
2. **每个方法都要调用 `db.close()`**: 确保连接状态正确
3. **避免并发访问**: Qt SQLite不支持多线程并发写入

---

## 七、构建和部署

### 7.1 构建系统

**Qt Creator**: 推荐使用Qt Creator进行开发和构建

**命令行构建**:
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 7.2 依赖项

- **Qt 5.x 或 Qt 6.x**
- **Qt SQL模块** (SQLite驱动)
- **Qt Network模块** (天气API)
- **Qt Widgets模块** (UI组件)
- **QtAwesome库** (图标)

### 7.3 部署清单

**Windows部署**:
- PersonalDateAssisant.exe
- Qt6Core.dll
- Qt6Gui.dll
- Qt6Sql.dll
- Qt6Widgets.dll
- platforms/qwindows.dll
- sqldrivers/qsqlite.dll
- Font Awesome字体文件

### 7.4 数据文件位置

**Windows**: `%APPDATA%/PersonalDateAssisant/`
- `schedules.db` - 日程数据库
- `weather_settings.ini` - 天气设置

---

## 八、扩展和优化建议

### 8.1 功能扩展

1. **日程分类标签**: 添加标签系统，支持多标签
2. **重复日程**: 支持按周、月、年重复
3. **日程导出**: 导出为CSV、JSON、ICS格式
4. **日程导入**: 从外部文件导入日程
5. **数据同步**: 支持云端同步
6. **多语言支持**: i18n国际化

### 8.2 性能优化

1. **数据库索引**: 在`datetime`和`isBatch`字段上添加索引
2. **懒加载**: 日历视图使用懒加载，只加载可见月份
3. **缓存**: 缓存常用查询结果
4. **异步操作**: 将数据库操作移到后台线程

### 8.3 UI/UX优化

1. **深色模式**: 支持深色主题
2. **键盘快捷键**: 添加常用操作的快捷键
3. **拖拽排序**: 日程列表支持拖拽排序
4. **手势操作**: 支持触摸手势
5. **通知中心**: 使用系统通知中心

---

## 九、相关文档

更多详细信息请参考以下文档：

- [批量添加日程功能实现文档](../doc/批量添加日程功能实现文档.md)
- [批量日程合并显示功能说明](../doc/批量日程合并显示功能说明.md)
- [批量删除日程功能说明](../doc/批量删除日程功能说明.md)
- [批量删除核心问题修复说明](../doc/批量删除核心问题修复说明.md)
- [日程删除功能优化说明](../doc/日程删除功能优化说明.md)

---

## 十、总结

### 10.1 核心特性

1. ✅ **日程管理**: 创建、编辑、删除日程
2. ✅ **日历视图**: 可视化查看日程
3. ✅ **农历转换**: 显示农历日期和节气
4. ✅ **天气预报**: 集成高德天气API
5. ✅ **提醒通知**: 支持多种提醒方式
6. ✅ **批量日程**: 支持批量添加多天日程
7. ✅ **批量删除**: 一键删除选中日程
8. ✅ **数据持久化**: SQLite本地存储

### 10.2 技术亮点

1. **批量日程合并显示**: 同批次日程在列表中合并为1个条目
2. **智能批量删除**: 自动识别并删除整个批次
3. **单一数据库连接**: 避免多连接冲突
4. **异步天气加载**: 不阻塞UI
5. **动画效果**: 平滑的最小化/展开动画

### 10.3 项目状态

**当前版本**: 1.0+  
**开发状态**: 活跃维护中  
**用户反馈**: 批量删除功能已验证可用

---

**文档更新日期**: 2026-04-08  
**最后维护者**: AI Assistant (Trae IDE)
