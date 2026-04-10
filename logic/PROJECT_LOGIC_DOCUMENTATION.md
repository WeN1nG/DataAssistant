# 个人日程助手 (PersonalDateAssisant) - 项目逻辑文档

> **当前版本**: v0.1.1  
> **下一版本预告**: v0.2 - 多邮箱管理系统（基于SMTP/IMAP协议）

---

## 一、项目概述

### 1.1 项目简介

**项目名称**: 个人日程助手 (PersonalDateAssisant)  
**项目类型**: Qt桌面应用程序  
**当前版本**: v0.1.1  
**下一版本**: v0.2（预告）

**v0.1.1 当前主要功能**:
- 个人日程管理（CRUD操作）
- 日历展示（支持批量日程）
- 农历转换与节气显示
- 天气预报（高德API集成）
- 提醒通知功能
- 批量日程管理（添加、删除）
- 系统托盘通知
- 数据备份与恢复

**v0.2 预告功能** - 多邮箱管理系统:
- 基于SMTP协议的邮件发送功能
- 基于IMAP协议的邮件接收与同步
- 多邮箱账户统一管理
- 邮件与日程关联提醒
- POP3/SMTP/IMAP全协议支持

**目标用户**: 需要日程管理、邮件处理、查看农历信息和天气预报的个人用户

### 1.2 技术栈

**当前版本 (v0.1.1)**:
- **框架**: Qt5/Qt6 (支持跨平台)
- **构建系统**: CMake/Qt Creator
- **数据库**: SQLite
- **网络**: Qt Network模块 (高德天气API集成)
- **UI框架**: Qt Widgets + Qt Designer UI文件
- **图标库**: Font Awesome (通过QtAwesome库集成)

**v0.2 新增技术栈**:
- **邮件协议**: QSslSocket (SMTP/IMAP/POP3)
- **加密**: OpenSSL (SSL/TLS支持)
- **邮件解析**: QMimeMessage (Qt邮件解析)

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
├── SystemTrayNotification.h/cpp # 系统托盘通知
├── main.cpp                 # 应用程序入口
└── CMakeLists.txt           # CMake构建配置

# v0.2 预告新增模块
├── EmailManager.h/cpp        # 邮件管理器（SMTP/IMAP核心）
├── EmailAccountDialog.h/cpp/ui # 邮箱账户配置对话框
├── EmailListDialog.h/cpp/ui   # 邮件列表对话框
├── EmailDetailDialog.h/cpp/ui # 邮件详情对话框
└── EmailScheduleLinker.h/cpp # 邮件日程关联管理
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
│  ┌──────────────────────┐ ┌────────────────────┐            │
│  │ EmailAccountDialog   │ │ EmailListDialog    │ # v0.2     │
│  │ (邮箱账户配置)       │ │ (邮件列表)          │ # v0.2     │
│  └──────────────────────┘ └────────────────────┘            │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   Business Logic Layer (业务逻辑层)           │
│  ┌──────────────┐ ┌──────────────┐ ┌────────────────────┐  │
│  │ReminderManager│ │WeatherManager │ │LunarCalendar        │  │
│  │ (提醒管理)    │ │ (天气管理)    │ │ (农历转换)          │  │
│  └──────────────┘ └──────────────┘ └────────────────────┘  │
│  ┌──────────────────────┐ ┌────────────────────────────┐  │
│  │ EmailManager          │ │ EmailScheduleLinker        │  │ # v0.2
│  │ (邮件管理器)          │ │ (邮件日程关联)            │  │ # v0.2
│  │ - SMTP发送            │ └────────────────────────────┘  │
│  │ - IMAP接收            │                                  │
│  │ - POP3支持            │                                  │
│  └──────────────────────┘                                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Data Access Layer (数据访问层)            │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              DatabaseManager (数据库管理)              │   │
│  │  - SQLite数据库操作                                    │   │
│  │  - 批量日程CRUD操作                                   │   │
│  │  - 邮件账户数据存储                                    │   │
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
│  ┌──────────────────────────────────────────────────────┐   │
│  │ email_accounts.db / accounts表                      │   │ # v0.2
│  │ (邮箱账户数据库)                                     │   │
│  └──────────────────────────────────────────────────────┘   │
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

# v0.2 新增组件
┌──────────────────────────────────────────────────────────────────┐
│                       EmailManager                                │
│              (邮件管理器 - SMTP/IMAP/POP3)                        │
│  ┌────────────────┬────────────────┬────────────────┬───────────┐ │
│  │ SMTP Client    │ IMAP Client    │ POP3 Client    │ SSL/TLS  │ │
│  │ (邮件发送)     │ (邮件接收)     │ (邮件下载)     │ 加密支持  │ │
│  └───────┬────────┴───────┬────────┴───────┬────────┴─────┬─────┘ │
│          │                │                 │              │       │
│          ▼                ▼                 ▼              ▼       │
│  ┌────────────┐   ┌────────────┐   ┌────────────┐  ┌──────────┐ │
│  │ 邮件服务器 │   │ 邮件服务器 │   │ 邮件服务器 │  │ 证书验证 │ │
│  │ SMTP:587  │   │ IMAP:993   │   │ POP3:995   │  │ 加密连接 │ │
│  └────────────┘   └────────────┘   └────────────┘  └──────────┘ │
└──────────────────────────────────────────────────────────────────┘
```

---

## 三、数据结构

### 3.1 Schedule 结构体 (v0.1.1)

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

### 3.2 WeatherInfo 结构体 (v0.1.1)

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

### 3.3 LunarDate 结构体 (v0.1.1)

```cpp
struct LunarDate {
    int year;                   // 农历年
    int month;                  // 农历月 (1-12)
    int day;                    // 农历日 (1-30)
    bool isLeap;               // 是否闰月
};
```

### 3.4 DeleteResult 结构体（批量删除专用 - v0.1.1）

```cpp
struct DeleteResult {
    int totalSchedules;         // 将要/实际删除的日程总数
    int batchCount;            // 涉及的批量日程批次数
    int scheduleCount;         // 涉及的普通日程数
    int actualDeleted;         // 实际删除的日程数
    QStringList batchDetails;   // 每个批量日程的详细信息
};
```

### 3.5 EmailAccount 结构体 (v0.2 预告)

```cpp
struct EmailAccount {
    int id;                     // 账户唯一标识符
    QString email;              // 邮箱地址
    QString displayName;        // 显示名称
    QString provider;          // 邮件服务商 (gmail, outlook, qq, 163等)
    
    // SMTP配置
    QString smtpHost;          // SMTP服务器地址
    int smtpPort;              // SMTP端口 (25, 465, 587)
    QString smtpUsername;      // SMTP用户名
    QString smtpPassword;      // SMTP密码/授权码
    bool smtpUseSsl;           // 是否使用SSL
    bool smtpUseTls;           // 是否使用TLS
    
    // IMAP配置
    QString imapHost;          // IMAP服务器地址
    int imapPort;              // IMAP端口 (993)
    QString imapUsername;      // IMAP用户名
    QString imapPassword;      // IMAP密码/授权码
    bool imapUseSsl;           // 是否使用SSL
    
    // POP3配置 (可选)
    QString pop3Host;          // POP3服务器地址
    int pop3Port;              // POP3端口 (995)
    bool pop3Enabled;          // 是否启用POP3
    
    bool isDefault;           // 是否为默认发送账户
    bool syncEnabled;         // 是否启用同步
    QDateTime lastSyncTime;    // 最后同步时间
};
```

### 3.6 EmailMessage 结构体 (v0.2 预告)

```cpp
struct EmailMessage {
    QString messageId;        // 邮件唯一标识符 (Message-ID)
    QString subject;           // 邮件主题
    QString from;              // 发件人
    QStringList to;            // 收件人列表
    QStringList cc;            // 抄送列表
    QStringList bcc;           // 密送列表
    QDateTime date;            // 发送日期时间
    QString body;             // 邮件正文 (纯文本)
    QString htmlBody;          // 邮件正文 (HTML)
    QStringList attachments;  // 附件路径列表
    bool isRead;               // 是否已读
    bool isStarred;            // 是否标记星标
    QString folder;           // 所在文件夹 (INBOX, Sent, Drafts等)
    int size;                  // 邮件大小 (字节)
    
    // 与日程关联
    int linkedScheduleId;     // 关联的日程ID (可选)
    bool hasScheduleReminder; // 是否设置了日程提醒
};
```

### 3.7 数据库表结构

**表名**: `schedules` (v0.1.1)

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

**表名**: `email_accounts` (v0.2预告)

| 字段名 | 类型 | 说明 |
|--------|------|------|
| id | INTEGER | 主键，自增 |
| email | TEXT | 邮箱地址，唯一 |
| display_name | TEXT | 显示名称 |
| provider | TEXT | 邮件服务商 |
| smtp_host | TEXT | SMTP服务器 |
| smtp_port | INTEGER | SMTP端口 |
| smtp_username | TEXT | SMTP用户名 |
| smtp_password_encrypted | TEXT | 加密的SMTP密码 |
| smtp_use_ssl | INTEGER | SMTP是否使用SSL |
| smtp_use_tls | INTEGER | SMTP是否使用TLS |
| imap_host | TEXT | IMAP服务器 |
| imap_port | INTEGER | IMAP端口 |
| imap_username | TEXT | IMAP用户名 |
| imap_password_encrypted | TEXT | 加密的IMAP密码 |
| imap_use_ssl | INTEGER | IMAP是否使用SSL |
| pop3_host | TEXT | POP3服务器 (可选) |
| pop3_port | INTEGER | POP3端口 |
| pop3_enabled | INTEGER | 是否启用POP3 |
| is_default | INTEGER | 是否默认账户 |
| sync_enabled | INTEGER | 是否启用同步 |
| last_sync_time | TEXT | 最后同步时间 |
| created_at | TEXT | 创建时间 |
| updated_at | TEXT | 更新时间 |

**表名**: `email_messages` (v0.2预告)

| 字段名 | 类型 | 说明 |
|--------|------|------|
| id | INTEGER | 主键，自增 |
| message_id | TEXT | 邮件唯一标识符 |
| account_id | INTEGER | 所属账户ID |
| subject | TEXT | 邮件主题 |
| sender | TEXT | 发件人 |
| recipients | TEXT | 收件人 (JSON数组) |
| cc | TEXT | 抄送 (JSON数组) |
| date | TEXT | 发送日期 |
| body_text | TEXT | 纯文本正文 |
| body_html | TEXT | HTML正文 |
| is_read | INTEGER | 是否已读 |
| is_starred | INTEGER | 是否星标 |
| folder | TEXT | 所在文件夹 |
| size | INTEGER | 邮件大小 |
| linked_schedule_id | INTEGER | 关联日程ID |
| created_at | TEXT | 同步时间 |

**数据库路径**: `QStandardPaths::AppDataLocation/schedules.db`

**数据库连接**: 使用单一共享连接，避免多连接冲突

---

## 四、文件详细文档

### 4.1 main.cpp (v0.1.1)

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

### 4.2 Widget.h / Widget.cpp (v0.1.1)

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

### 4.3 CalendarDelegate.h / CalendarDelegate.cpp (v0.1.1)

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

### 4.4 DatabaseManager.h / DatabaseManager.cpp (v0.1.1)

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\DatabaseManager.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\DatabaseManager.cpp`

**功能**: SQLite数据库管理器，负责所有日程数据的CRUD操作，支持批量日程管理

**数据库路径**: `QStandardPaths::AppDataLocation/schedules.db`

**数据库连接管理**:
```cpp
class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();
    
    static QSqlDatabase getDatabaseConnection(const QString& connectionName = QString());

    // 日程CRUD操作
    bool addSchedule(const Schedule& schedule);
    bool addBatchSchedules(const QVector<Schedule>& schedules);
    bool updateSchedule(const Schedule& schedule);
    
    // 删除操作（多层次设计）
    bool deleteSchedule(int id);
    bool deleteBatchSchedules(const QString& batchGroupId);
    bool deleteScheduleWithBatch(int id);
    bool deleteAllSchedules();
    int deleteSchedulesByIds(const QVector<int>& ids);
    DeleteResult deleteSchedulesWithDetails(const QVector<int>& ids);
    
    // 查询操作
    QVector<Schedule> getSchedules();
    QVector<Schedule> getSchedulesByDate(const QDate& date);
    Schedule getScheduleById(int id);
    QVector<Schedule> getBatchSchedulesByGroupId(const QString& groupId);
    
    // 备份恢复
    bool backupDatabase(const QString& backupPath);
    bool restoreDatabase(const QString& backupPath);
    
    // 工具方法
    static QColor getColorByPriority(int priority);
};
```

**关键实现细节**:

1. **单一数据库连接**: 使用静态方法 `getDatabaseConnection()` 确保全局唯一连接
2. **批量操作事务**: 批量添加/删除使用事务保证数据一致性
3. **删除预览机制**: `getDeletePreview()` 提供删除前预览，包含批量日程详细信息
4. **向后兼容**: 保留传统删除函数，同时提供新的删除函数体系

---

### 4.5 ReminderManager.h / ReminderManager.cpp (v0.1.1)

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ReminderManager.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ReminderManager.cpp`

**功能**: 日程提醒管理，使用QTimer定时检查并触发提醒通知

**核心机制**:
- 使用 `QTimer::singleShot()` 实现延迟提醒
- 支持自定义提前提醒时间（分钟数）
- 提醒触发时弹出系统通知或消息框
- 支持系统托盘通知（通过SystemTrayNotification）

---

### 4.6 WeatherManager.h / WeatherManager.cpp (v0.1.1)

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\WeatherManager.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\WeatherManager.cpp`

**功能**: 天气信息管理，通过高德天气API获取实时天气和预报数据

**API集成**:
```cpp
class WeatherManager : public QObject {
    Q_OBJECT
    
signals:
    void weatherUpdated(const WeatherInfo& weather);
    void errorOccurred(const QString& error);
    
public:
    WeatherManager(QObject* parent = nullptr);
    void fetchWeather(const QString& city);
    WeatherInfo getCurrentWeather() const;
    
private:
    void parseJsonResponse(const QByteArray& data);
    void fetchWeatherByIP();
    
    WeatherInfo m_weatherInfo;
    QString m_apiKey;  // 高德API Key
};
```

**数据结构** (WeatherInfo):
- 城市名称
- 当前温度
- 天气状况（含emoji图标）
- 风力信息
- 温度区间
- 未来3天预报

---

### 4.7 LunarCalendar.h / LunarCalendar.cpp (v0.1.1)

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\LunarCalendar.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\LunarCalendar.cpp`

**功能**: 农历日期转换，支持公历农历互转和节气计算

**主要功能**:
- 公历转农历
- 农历转公历
- 获取节气信息
- 显示农历年月日

---

### 4.8 EmailManager.h / EmailManager.cpp (v0.2预告)

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\EmailManager.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\EmailManager.cpp`

**功能**: 邮件管理器，支持SMTP发送、IMAP接收、POP3下载

**SMTP发送流程**:
```
1. 连接到SMTP服务器 (smtp.example.com:587)
2. 发送EHLO/HELO命令
3. STARTTLS加密（如果启用）
4. AUTH LOGIN认证
5. MAIL FROM设置发件人
6. RCPT TO设置收件人
7. DATA发送邮件内容
8. QUIT断开连接
```

**IMAP接收流程**:
```
1. 连接到IMAP服务器 (imap.example.com:993)
2. STARTTLS加密（如果启用）
3. LOGIN认证
4. SELECT INBOX选择收件箱
5. SEARCH/EXAMINE查询邮件
6. FETCH获取邮件内容
7. LOGOUT断开连接
```

**核心API设计**:
```cpp
class EmailManager : public QObject {
    Q_OBJECT
    
signals:
    void sendCompleted(bool success, const QString& error);
    void receiveCompleted(const QList<EmailMessage>& emails);
    void syncProgress(int current, int total);
    void errorOccurred(const QString& error);
    
public:
    // SMTP操作
    bool sendEmail(const EmailMessage& email, const EmailAccount& account);
    bool sendEmailAsync(const EmailMessage& email, const EmailAccount& account);
    
    // IMAP操作
    QList<EmailMessage> fetchEmails(const EmailAccount& account, 
                                   const QString& folder = "INBOX",
                                   int limit = 50);
    bool syncEmails(const EmailAccount& account);
    
    // POP3操作
    QList<EmailMessage> downloadEmails(const EmailAccount& account, 
                                       bool deleteAfterDownload = false);
    
    // 账户管理
    bool validateAccount(const EmailAccount& account);
    EmailAccount detectAccountSettings(const QString& email);
    
private:
    QSslSocket* createSmtpConnection(const EmailAccount& account);
    QSslSocket* createImapConnection(const EmailAccount& account);
    bool authenticateSmtp(QSslSocket* socket, const EmailAccount& account);
    bool authenticateImap(QSslSocket* socket, const EmailAccount& account);
    EmailMessage parseEmailResponse(const QByteArray& data);
    QString encodeBase64(const QString& input);
    QString decodeBase64(const QString& input);
};
```

---

### 4.9 EmailAccountDialog.h / EmailAccountDialog.cpp (v0.2预告)

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\EmailAccountDialog.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\EmailAccountDialog.ui`

**功能**: 邮箱账户配置对话框

**主要功能**:
- 添加新邮箱账户
- 编辑现有账户
- 自动检测邮件服务商设置
- 支持SMTP/IMAP/POP3配置
- SSL/TLS加密选项
- 密码/授权码安全存储

**UI组件**:
- 邮箱地址输入框
- 显示名称输入框
- 邮件服务商选择（Gmail, Outlook, QQ, 163等）
- SMTP服务器配置（地址、端口、加密方式）
- IMAP服务器配置（地址、端口、加密方式）
- POP3服务器配置（可选）
- 用户名/密码输入
- 测试连接按钮
- 保存/取消按钮

---

### 4.10 EmailListDialog.h / EmailListDialog.cpp (v0.2预告)

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\EmailListDialog.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\EmailListDialog.ui`

**功能**: 邮件列表对话框，显示收件箱/发件箱等文件夹中的邮件

**主要功能**:
- 显示邮件列表（发件人、主题、日期、已读状态）
- 支持多选操作（删除、移动、星标）
- 文件夹切换（INBOX, Sent, Drafts, Trash）
- 搜索邮件功能
- 分页/滚动加载
- 邮件排序（日期、发件人、主题）

**数据结构**:
```cpp
class EmailListModel : public QAbstractTableModel {
    Q_OBJECT
    
public:
    enum EmailRoles {
        SubjectRole = Qt::UserRole + 1,
        SenderRole,
        DateRole,
        IsReadRole,
        IsStarredRole,
        MessageIdRole
    };
    
    QList<EmailMessage> emails;
    
    // QAbstractTableModel接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, 
                       int role = Qt::DisplayRole) const override;
};
```

---

### 4.11 EmailScheduleLinker.h / EmailScheduleLinker.cpp (v0.2预告)

**文件路径**:
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\EmailScheduleLinker.h`
- `c:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\EmailScheduleLinker.cpp`

**功能**: 邮件与日程关联管理

**主要功能**:
- 自动识别邮件中的日程信息（时间、地点、参与者）
- 从邮件内容提取日程并创建提醒
- 邮件回复自动关联原日程
- 日程变更通知通过邮件发送
- 日程邀请邮件解析和响应

**智能识别算法**:
```
输入: 邮件正文 (body)
输出: 日程信息 (Schedule) 或空

1. 时间识别
   - 正则匹配日期时间模式
   - 识别自然语言时间描述 ("明天上午", "下周一"等)
   - 时区处理

2. 地点识别
   - 识别常见地点关键词 ("地点", "地点是", "at", "where")
   - 提取具体地址信息

3. 事件识别
   - 关键词匹配 ("会议", "meeting", "约会", "date")
   - 日程邀请格式解析 (iCalendar)

4. 参与者识别
   - 提取邮箱地址
   - 识别人员姓名

5. 置信度评估
   - 综合以上信息评估识别准确性
   - 提供用户确认界面
```

---

## 五、SMTP/IMAP协议集成方案 (v0.2)

### 5.1 协议概述

**SMTP (Simple Mail Transfer Protocol)**:
- 用途: 发送电子邮件
- 默认端口: 25 (明文), 465 (SSL), 587 (TLS/STARTTLS)
- 特性: 可靠传输、支持认证、支持附件

**IMAP (Internet Message Access Protocol)**:
- 用途: 在服务器上访问和管理邮件
- 默认端口: 143 (明文), 993 (SSL)
- 特性: 多设备同步、服务器端文件夹管理、按需获取邮件部分

**POP3 (Post Office Protocol v3)**:
- 用途: 下载邮件到本地
- 默认端口: 110 (明文), 995 (SSL)
- 特性: 离线访问、简单实现、可选择下载后删除

### 5.2 SSL/TLS加密方案

**加密连接流程**:
```
1. 创建QSslSocket
2. 设置SSL配置
   - 设置证书模式 (QueryPeer)
   - 设置协议版本 (TLSv1.2, TLSv1.3)
3. 连接到服务器
   - 直接SSL连接 (端口465/993/995)
   - STARTTLS升级连接 (端口25/587/143)
4. 验证服务器证书
5. 进行认证
```

**证书验证**:
```cpp
QSslSocket* socket = new QSslSocket(this);
socket->setProtocol(QSsl::TlsV1_2OrLater);
socket->setPeerVerifyMode(QSslSocket::QueryPeer);

connect(socket, &QSslSocket::sslErrors, this, [](const QList<QSslError>& errors) {
    // 处理SSL错误，可选择忽略自签名证书
    for (const QSslError& error : errors) {
        qDebug() << "SSL Error:" << error.errorString();
    }
});
```

### 5.3 认证机制

**基本认证 (Base64编码)**:
```
用户名密码经过Base64编码后传输
AUTH LOGIN
Username: dXNlcm5hbWU=
Password: cGFzc3dvcmQ=
```

**OAuth 2.0认证 (v0.2高级功能)**:
```
用于Gmail、Outlook等现代邮件服务
1. 获取授权码 (用户授权)
2. 使用授权码换取访问令牌
3. 使用访问令牌发送/接收邮件
4. 令牌过期时刷新
```

### 5.4 常用邮件服务商配置

| 服务商 | SMTP服务器 | SMTP端口 | IMAP服务器 | IMAP端口 |
|--------|-----------|---------|-----------|---------|
| Gmail | smtp.gmail.com | 587 (TLS) | imap.gmail.com | 993 (SSL) |
| Outlook | smtp-mail.outlook.com | 587 (TLS) | outlook.office365.com | 993 (SSL) |
| QQ邮箱 | smtp.qq.com | 587 (TLS) | imap.qq.com | 993 (SSL) |
| 163邮箱 | smtp.163.com | 465 (SSL) | imap.163.com | 993 (SSL) |
| 企业邮箱 | smtp.company.com | 465/587 | imap.company.com | 993 |

**自动检测实现**:
```cpp
EmailAccount EmailManager::detectAccountSettings(const QString& email) {
    EmailAccount account;
    QString domain = email.split('@').last().toLower();
    
    // 常见邮箱域名映射
    QMap<QString, EmailProviderConfig> providers = {
        {"gmail.com", {"smtp.gmail.com", 587, "imap.gmail.com", 993}},
        {"qq.com", {"smtp.qq.com", 587, "imap.qq.com", 993}},
        {"163.com", {"smtp.163.com", 465, "imap.163.com", 993}},
        // ... 更多服务商
    };
    
    if (providers.contains(domain)) {
        const auto& config = providers[domain];
        account.smtpHost = config.smtpHost;
        account.smtpPort = config.smtpPort;
        account.imapHost = config.imapHost;
        account.imapPort = config.imapPort;
        account.provider = domain;
    } else {
        // MX记录查询获取邮件服务器
        // 或者让用户手动配置
    }
    
    return account;
}
```

---

## 六、数据流程

### 6.1 日程管理流程 (v0.1.1)

```
┌─────────────────────────────────────────────────────────────────┐
│                     日程创建流程                                  │
├─────────────────────────────────────────────────────────────────┤
│ 用户点击"添加日程" → ScheduleDialog打开                         │
│         ↓                                                        │
│ 用户填写日程信息 (标题、描述、时间、优先级)                       │
│         ↓                                                        │
│ 用户选择"批量添加"选项（可选）                                    │
│         ↓                                                        │
│ 用户点击"保存"                                                   │
│         ↓                                                        │
│ ScheduleDialog调用DatabaseManager::addSchedule()                │
│         ↓                                                        │
│ DatabaseManager验证数据                                          │
│         ↓                                                        │
│ 如果是批量添加: DatabaseManager::addBatchSchedules()            │
│ 生成每日程，共享batchGroupId                                     │
│         ↓                                                        │
│ 事务提交到SQLite数据库                                           │
│         ↓                                                        │
│ 日程保存成功 → ReminderManager注册提醒                           │
│         ↓                                                        │
│ Widget::updateCalendarMarks()刷新日历显示                         │
│         ↓                                                        │
│ 日程添加完成                                                      │
└─────────────────────────────────────────────────────────────────┘
```

### 6.2 邮件发送流程 (v0.2预告)

```
┌─────────────────────────────────────────────────────────────────┐
│                     SMTP邮件发送流程                              │
├─────────────────────────────────────────────────────────────────┤
│ 用户点击"发送邮件"或"回复邮件"                                   │
│         ↓                                                        │
│ EmailComposeDialog打开（新建或回复模式）                          │
│         ↓                                                        │
│ 用户编辑邮件内容、添加附件、选择发送账户                          │
│         ↓                                                        │
│ 用户点击"发送"                                                   │
│         ↓                                                        │
│ EmailManager::sendEmail()被调用                                  │
│         ↓                                                        │
│ 验证发件人账户配置                                                │
│         ↓                                                        │
│ 创建QSslSocket连接到SMTP服务器                                    │
│         ↓                                                        │
│ 执行SMTP握手: EHLO → STARTTLS → AUTH LOGIN                      │
│         ↓                                                        │
│ 使用Base64编码的用户名密码进行认证                                │
│         ↓                                                        │
│ 发送MAIL FROM、RCPT TO命令                                        │
│         ↓                                                        │
│ 发送DATA命令，传输RFC 2822格式邮件内容                            │
│         ↓                                                        │
│ 服务器返回250 OK表示发送成功                                      │
│         ↓                                                        │
│ 断开连接，更新邮件到已发送文件夹                                  │
│         ↓                                                        │
│ 发送成功通知显示给用户                                            │
└─────────────────────────────────────────────────────────────────┘
```

### 6.3 邮件接收流程 (v0.2预告)

```
┌─────────────────────────────────────────────────────────────────┐
│                     IMAP邮件接收流程                              │
├─────────────────────────────────────────────────────────────────┤
│ 用户选择邮箱账户并点击"同步"                                      │
│         ↓                                                        │
│ EmailManager::syncEmails()被调用                                 │
│         ↓                                                        │
│ 创建QSslSocket连接到IMAP服务器                                    │
│         ↓                                                        │
│ 执行IMAP握手: 登录认证                                            │
│         ↓                                                        │
│ SELECT INBOX选择收件箱                                           │
│         ↓                                                        │
│ SEARCH UNSEEN查询未读邮件                                        │
│         ↓                                                        │
│ 遍历邮件列表，发送FETCH命令获取邮件内容                           │
│         ↓                                                        │
│ 解析IMAP响应，提取邮件头部和正文                                  │
│         ↓                                                        │
│ EmailScheduleLinker检查是否包含日程信息                          │
│         ↓                                                        │
│ 保存邮件到本地数据库 (email_messages表)                           │
│         ↓                                                        │
│ 如果有日程关联，创建或更新提醒                                    │
│         ↓                                                        │
│ 更新Widget显示新邮件数量                                          │
│         ↓                                                        │
│ 邮件同步完成，显示同步统计                                        │
└─────────────────────────────────────────────────────────────────┘
```

### 6.4 邮件与日程关联流程 (v0.2预告)

```
┌─────────────────────────────────────────────────────────────────┐
│                     邮件日程关联流程                              │
├─────────────────────────────────────────────────────────────────┤
│ 新邮件同步到本地数据库                                            │
│         ↓                                                        │
│ EmailScheduleLinker::analyzeEmail()分析邮件内容                  │
│         ↓                                                        │
│ 提取邮件中的时间信息、地点信息、参与者                            │
│         ↓                                                        │
│ 如果检测到有效日程信息:                                           │
│         ↓                                                        │
│ 显示"检测到日程信息"提示                                          │
│         ↓                                                        │
│ 用户确认或编辑日程详情                                            │
│         ↓                                                        │
│ 创建Schedule对象并保存                                           │
│         ↓                                                        │
│ 在email_messages表中设置linked_schedule_id                       │
│         ↓                                                        │
│ ReminderManager注册日程提醒                                       │
│         ↓                                                        │
│ 日程提醒触发时，自动回复邮件通知参与者                             │
│         ↓                                                        │
│ 关联完成                                                          │
└─────────────────────────────────────────────────────────────────┘
```

---

## 七、API接口设计 (v0.2预告)

### 7.1 EmailManager 公共接口

```cpp
class EmailManager : public QObject {
    Q_OBJECT
    
public:
    // 单例模式获取实例
    static EmailManager* instance();
    
    // SMTP操作
    bool sendEmail(const EmailMessage& email, const EmailAccount& account);
    bool sendEmailWithAttachments(const EmailMessage& email, 
                                 const QStringList& attachmentPaths,
                                 const EmailAccount& account);
    
    // IMAP操作
    QList<EmailMessage> fetchInbox(const EmailAccount& account, int limit = 50);
    QList<EmailMessage> fetchFolder(const EmailAccount& account, 
                                    const QString& folderName, 
                                    int limit = 50);
    bool markAsRead(const QString& messageId);
    bool markAsUnread(const QString& messageId);
    bool moveToFolder(const QString& messageId, const QString& folder);
    bool deleteEmail(const QString& messageId);
    
    // POP3操作
    QList<EmailMessage> downloadAll(const EmailAccount& account);
    QList<EmailMessage> downloadNew(const EmailAccount& account);
    
    // 账户管理
    bool addAccount(const EmailAccount& account);
    bool updateAccount(const EmailAccount& account);
    bool deleteAccount(int accountId);
    QList<EmailAccount> getAllAccounts();
    EmailAccount getDefaultAccount();
    bool setDefaultAccount(int accountId);
    
    // 设置
    void setSyncInterval(int minutes);
    void setAutoSync(bool enabled);
    
signals:
    void emailSent(bool success, const QString& error);
    void emailsReceived(const QList<EmailMessage>& emails);
    void syncStarted();
    void syncFinished(int totalEmails);
    void syncProgress(int current, int total);
    void errorOccurred(const QString& error);
    
private:
    // 内部实现
    bool connectToSmtp(const EmailAccount& account);
    bool connectToImap(const EmailAccount& account);
    QString buildSmtpCommand(const QString& command);
    EmailMessage parseImapFetch(const QByteArray& data);
    QByteArray encodeBase64(const QString& input);
    QString decodeBase64(const QByteArray& input);
};
```

### 7.2 EmailScheduleLinker 接口

```cpp
class EmailScheduleLinker : public QObject {
    Q_OBJECT
    
public:
    struct ExtractedSchedule {
        QString title;
        QDateTime startTime;
        QDateTime endTime;
        QString location;
        QStringList participants;
        double confidence;
    };
    
    // 日程提取
    ExtractedSchedule analyzeEmail(const EmailMessage& email);
    QList<ExtractedSchedule> analyzeMultipleEmails(const QList<EmailMessage>& emails);
    
    // 日程创建
    bool createScheduleFromEmail(const EmailMessage& email);
    bool createScheduleFromExtracted(const ExtractedSchedule& schedule, 
                                    const EmailMessage& email);
    
    // 日程通知
    void scheduleEmailReminder(const Schedule& schedule, const EmailMessage& email);
    bool sendScheduleNotification(const Schedule& schedule, 
                                 const QStringList& recipients);
    
    // 日程邀请解析
    bool parseCalendarInvite(const QString& icsContent);
    Schedule createScheduleFromInvite(const QString& icsContent);
    
signals:
    void scheduleDetected(const ExtractedSchedule& schedule);
    void scheduleCreated(const Schedule& schedule);
    void reminderScheduled(const Schedule& schedule);
};
```

### 7.3 数据库操作接口

```cpp
class DatabaseManager {
public:
    // v0.1.1 现有接口
    bool addSchedule(const Schedule& schedule);
    bool addBatchSchedules(const QVector<Schedule>& schedules);
    bool updateSchedule(const Schedule& schedule);
    bool deleteSchedule(int id);
    QVector<Schedule> getSchedulesByDate(const QDate& date);
    
    // v0.2 新增邮箱相关接口
    bool addEmailAccount(const EmailAccount& account);
    bool updateEmailAccount(const EmailAccount& account);
    bool deleteEmailAccount(int accountId);
    QList<EmailAccount> getAllEmailAccounts();
    EmailAccount getEmailAccountById(int accountId);
    EmailAccount getDefaultEmailAccount();
    
    bool addEmailMessage(const EmailMessage& email);
    bool updateEmailMessage(const EmailMessage& email);
    bool deleteEmailMessage(const QString& messageId);
    QList<EmailMessage> getEmailsByFolder(int accountId, const QString& folder);
    QList<EmailMessage> searchEmails(int accountId, const QString& query);
    
    bool linkEmailToSchedule(const QString& messageId, int scheduleId);
    bool unlinkEmailFromSchedule(const QString& messageId);
    QList<EmailMessage> getEmailsBySchedule(int scheduleId);
};
```

---

## 八、版本规划

### 8.1 v0.1.1 (当前版本)

**发布时间**: 2024年
**主要功能**:
- ✅ 日程管理（创建、编辑、删除、查看）
- ✅ 日历展示（周视图、月视图切换）
- ✅ 农历转换和节气显示
- ✅ 天气预报（高德API集成）
- ✅ 提醒通知功能
- ✅ 批量日程管理
- ✅ 系统托盘
- ✅ 数据备份与恢复

**技术亮点**:
- Qt Widgets现代UI设计
- 自定义日历代理实现
- SQLite数据库事务管理
- 多层次删除函数设计

### 8.2 v0.2 (下一版本 - 预告)

**计划发布时间**: 待定
**主要目标**: 多邮箱管理系统

**核心功能**:
- 🔄 SMTP邮件发送
- 🔄 IMAP邮件同步
- 🔄 POP3邮件下载
- 🔄 多邮箱账户管理
- 🔄 SSL/TLS加密连接
- 🔄 邮件与日程智能关联
- 🔄 日程邀请自动解析

**技术挑战**:
- Qt Network邮件协议实现
- SSL/TLS证书验证
- 大型邮件高效处理
- 邮件编码处理（Base64、Quoted-printable）
- iCalendar格式解析

### 8.3 未来版本展望

**v0.3 规划**:
- 日程共享与协作
- 云同步服务集成
- 移动端应用（iOS/Android）

**v1.0 目标**:
- 完整的邮件+日程一体化管理平台
- 跨平台支持（Windows/macOS/Linux）
- 企业版功能（团队协作、权限管理）

---

## 九、部署与安装

### 9.1 Windows部署

**依赖项**:
- Qt 5.15+ 或 Qt 6.2+ Runtime
- Microsoft Visual C++ Redistributable
- OpenSSL 1.1.1+ (用于SSL/TLS)
- SQLite运行时库

**安装方式**:
1. InnoSetup打包安装程序
2. 便携版ZIP压缩包
3. MSIX/Microsoft Store分发

**v0.2 新增部署需求**:
- OpenSSL DLL文件 (libssl-1_1-x64.dll, libcrypto-1_1-x64.dll)
- 网络防火墙配置（允许SMTP/IMAP/POP3出站流量）

### 9.2 构建说明

**前置条件**:
```bash
# Windows
Qt 5.15+ 或 Qt 6.2+
CMake 3.16+
Visual Studio 2019+ 或 MinGW 8.1+

# Linux
Qt 5.15+
CMake 3.16+
gcc/g++ 9+
```

**编译步骤**:
```bash
cd DataAssistant
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

**CMakeLists.txt v0.2更新**:
```cmake
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Widgets 
    Sql 
    Network
    NetworkAuth    # OAuth支持
)

target_link_libraries(PersonalDateAssisant PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Sql 
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::NetworkAuth
    QtAwesome 
    winmm.lib
    # v0.2 新增SSL支持
    OpenSSL::SSL
    OpenSSL::Crypto
)
```

---

## 十、常见问题与解决方案

### 10.1 v0.1.1 已知问题

**Q: 日历点击无响应**
- 检查CalendarDelegate是否正确安装
- 确认QCalendarWidget::setDelegate()被调用
- 查看日志中的错误信息

**Q: 天气信息获取失败**
- 确认网络连接正常
- 检查高德API Key是否有效
- 查看API调用配额是否超限

**Q: 批量日程显示重叠**
- CalendarDelegate的paint()方法中检查绘制顺序
- 确认schedule叠加绘制逻辑

### 10.2 v0.2 预期问题与解决

**Q: SMTP连接超时**
- 检查网络防火墙设置
- 确认SMTP端口未被阻止（25, 465, 587）
- 尝试更换端口（587 → 465）

**Q: SSL证书验证失败**
- 忽略自签名证书（仅用于测试）
- 更新OpenSSL到最新版本
- 确保证书链完整

**Q: 邮件发送被拒**
- 检查用户名密码是否正确
- Gmail需要应用专用密码
- 确认两步验证设置

**Q: IMAP同步慢**
- 使用IDLE命令保持连接
- 分批获取邮件（每次50封）
- 启用增量同步

---

## 十一、附录

### 11.1 相关文档

- [批量添加日程功能实现文档](../doc/批量添加日程功能实现文档.md)
- [批量日程合并显示功能说明](../doc/批量日程合并显示功能说明.md)
- [批量删除日程功能说明](../doc/批量删除日程功能说明.md)
- [农历日期计算修复报告](../doc/农历日期计算修复报告.md)
- [高德API调用须知](../doc/高德API调用须知.md)

### 11.2 外部资源

- Qt官方文档: https://doc.qt.io/
- SMTP协议规范: RFC 5321
- IMAP协议规范: RFC 3501
- POP3协议规范: RFC 1939
- iCalendar规范: RFC 5545

### 11.3 版本历史

| 版本 | 日期 | 主要更新 |
|------|------|---------|
| v0.1.0 | 2024年初 | 基础日程管理功能 |
| v0.1.1 | 2024年 | 批量日程管理优化、UI改进 |

---

**文档维护**: 本文档随项目迭代持续更新，如有疑问请联系开发团队。

**最后更新**: 2026年4月10日
