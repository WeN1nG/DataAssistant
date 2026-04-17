#include "Widget.h"
#include "./ui_Widget.h"
#include "ScheduleDialog.h"
#include "ReminderManager.h"
#include "WeatherManager.h"
#include "SettingsDialog.h"
#include "QtAwesome.h"
#include "EmailDataManager.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QMenu>
#include <QTextCharFormat>
#include <QMap>
#include <QColor>
#include <QTableView>
#include <QHeaderView>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>

using namespace fa;

void Widget::setupToolBarButtons() {
    // 设置按钮样式
    QString buttonStyle = R"(
        QPushButton {
            border: 1px solid #cccccc;
            background-color: #f5f5f5;
            text-align: center;
            padding: 4px;
            border-radius: 4px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
            border-color: #999999;
        }
        QPushButton:pressed {
            background-color: #d0d0d0;
            border-color: #666666;
        }
    )";
    
    // 设置添加日程按钮（纯图标）
    ui->addScheduleButton->setStyleSheet(buttonStyle);
    ui->addScheduleButton->setToolTip("添加日程");

    // 设置查看日程按钮（纯图标）
    ui->viewAllSchedulesButton->setStyleSheet(buttonStyle);
    ui->viewAllSchedulesButton->setToolTip("查看所有日程");

    // 设置设置按钮（纯图标）
    ui->settingsButton->setStyleSheet(buttonStyle);
    ui->settingsButton->setToolTip("设置");

    // 设置邮箱按钮（纯图标）
    ui->emailButton->setStyleSheet(buttonStyle);
    ui->emailButton->setToolTip("邮箱");

    // 设置日历按钮（纯图标）
    ui->calendarButton->setStyleSheet(buttonStyle);
    ui->calendarButton->setToolTip("日历");

    // 设置发邮件按钮（纯图标）
    ui->sendEmailButton->setStyleSheet(buttonStyle);
    ui->sendEmailButton->setToolTip("发邮件");

    // 设置邮箱账号管理按钮（纯图标）
    ui->userManagementButton->setStyleSheet(buttonStyle);
    ui->userManagementButton->setToolTip("邮箱账号管理");
}

void Widget::setupAwesomeIcons() {
    // 设置图标样式属性
    m_awesome->setDefaultOption("color", QColor(85, 85, 85));
    m_awesome->setDefaultOption("color-selected", QColor(255, 255, 255));
    m_awesome->setDefaultOption("scale-factor", 0.8);

    // 日历按钮图标 (fa_calendar)
    QIcon calendarIcon = m_awesome->icon(fa_solid, fa_calendar);
    ui->calendarButton->setIcon(calendarIcon);
    ui->calendarButton->setIconSize(QSize(24, 24));

    // 邮箱按钮图标 (fa_mail_bulk - 信箱图标)
    QIcon emailIcon = m_awesome->icon(fa_solid, fa_mail_bulk);
    ui->emailButton->setIcon(emailIcon);
    ui->emailButton->setIconSize(QSize(24, 24));

    // 添加日程按钮图标 (fa_plus - 加号)
    QIcon addIcon = m_awesome->icon(fa_solid, fa_plus);
    ui->addScheduleButton->setIcon(addIcon);
    ui->addScheduleButton->setIconSize(QSize(24, 24));

    // 日程表按钮图标 (fa_list - 列表)
    QIcon scheduleListIcon = m_awesome->icon(fa_solid, fa_list);
    ui->viewAllSchedulesButton->setIcon(scheduleListIcon);
    ui->viewAllSchedulesButton->setIconSize(QSize(24, 24));

    // 齿轮/设置按钮图标 (fa_cog)
    QIcon settingsIcon = m_awesome->icon(fa_solid, fa_cog);
    ui->settingsButton->setIcon(settingsIcon);
    ui->settingsButton->setIconSize(QSize(24, 24));

    // 发邮件按钮图标 (fa_paper_plane)
    QIcon sendEmailIcon = m_awesome->icon(fa_solid, fa_paper_plane);
    ui->sendEmailButton->setIcon(sendEmailIcon);
    ui->sendEmailButton->setIconSize(QSize(24, 24));

    // 邮箱账号管理按钮图标 (fa_user)
    QIcon userManagementIcon = m_awesome->icon(fa_solid, fa_user);
    ui->userManagementButton->setIcon(userManagementIcon);
    ui->userManagementButton->setIconSize(QSize(24, 24));

    // 更新按钮样式，支持图标和文字并排显示
    QString iconButtonStyle = R"(
        QPushButton {
            border: 1px solid #cccccc;
            background-color: #f5f5f5;
            text-align: center;
            padding: 4px;
            border-radius: 4px;
            font-size: 13px;
            icon-size: 24px;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
            border-color: #999999;
        }
        QPushButton:pressed {
            background-color: #d0d0d0;
            border-color: #666666;
        }
    )";

    ui->calendarButton->setStyleSheet(iconButtonStyle);
    ui->emailButton->setStyleSheet(iconButtonStyle);
    ui->addScheduleButton->setStyleSheet(iconButtonStyle);
    ui->viewAllSchedulesButton->setStyleSheet(iconButtonStyle);
    ui->sendEmailButton->setStyleSheet(iconButtonStyle);
    ui->userManagementButton->setStyleSheet(iconButtonStyle);
    ui->settingsButton->setStyleSheet(iconButtonStyle);
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , dbManager(new DatabaseManager())
    , reminderManager(new ReminderManager(this))
    , weatherManager(new WeatherManager(this))
    , m_awesome(new QtAwesome(this))
    , m_calendarWidget(new QCalendarWidget(this))
    , m_calendarDelegate(new CalendarDelegate(this))
    , m_calendarTableView(nullptr)
    , m_lunarCalendar(new LunarCalendar())
    , m_emailWindow(nullptr)
    , m_emailCountWindow(nullptr)
    , m_emailSendWindow(nullptr)
    , m_inboxWidget(nullptr)
    , m_sendEmailWidget(nullptr)
    , m_userManagementWidget(nullptr)
    , m_addScheduleWidget(nullptr)
    , m_scheduleListWidget(nullptr)
    , m_settingsWidget(nullptr)
    , m_windowStack(nullptr)
    , m_currentWindow(Window_Calendar)
{
    // 初始化刷新状态数组
    for (int i = 0; i < 7; ++i) {
        m_needsRefresh[i] = false;
    }
    
    ui->setupUi(this);
    
    // 设置窗口栈
    setupWindowStack();
    
    setupCalendar();
    updateCalendarMarks();
    updateDateInfo();  // 更新今日日期、节气、农历信息
    reminderManager->startMonitoring();
    m_awesome->initFontAwesome();
    
    connect(weatherManager, &WeatherManager::weatherUpdated, this, &Widget::updateWeatherDisplay);
    connect(weatherManager, &WeatherManager::errorOccurred, [=](const QString &error) {
        QMessageBox::warning(this, "天气更新失败", error);
    });
    
    // 不再使用QCalendarWidget::clicked信号，使用事件过滤器替代
    // connect(m_calendarWidget, &QCalendarWidget::clicked, this, &Widget::onCalendarViewClicked);
    
    // 连接日历月份变更信号 - 确保切换月份时更新今天的标记
    connect(m_calendarWidget, &QCalendarWidget::currentPageChanged, this, [this](int year, int month) {
        Q_UNUSED(year);
        Q_UNUSED(month);
        updateCalendarMarks();  // 更新今天的标记
    });
    
    // 设置工具栏按钮（使用黄金分割比例）
    setupToolBarButtons();
    setupAwesomeIcons();
    
    // 连接日历Delegate的日期点击信号
    connect(m_calendarDelegate, &CalendarDelegate::dateClicked, this, &Widget::onDateClickedFromDelegate);
    
    updateWeatherDisplay();
    
    if (!weatherManager->isEnabled()) {
        ui->weatherWidget->hide();
    }

    EmailDataManager emailDataManager;
    bool del = emailDataManager.IniAllEmailDatabaseFiles();
    qDebug() << "Delete Local Database : " << del;
}

Widget::~Widget()
{
    delete weatherManager;
    delete reminderManager;
    delete dbManager;
    delete m_awesome;
    delete m_calendarDelegate;
    delete m_lunarCalendar;
    delete m_emailWindow;
    delete m_inboxWidget;
    delete m_sendEmailWidget;
    delete m_userManagementWidget;
    delete m_addScheduleWidget;
    delete m_scheduleListWidget;
    delete m_settingsWidget;
    delete m_windowStack;
    delete ui;
}

void Widget::setupWindowStack() {
    // 隐藏原来的日历
    ui->calendarView->hide();
    
    // 创建窗口栈
    m_windowStack = new QStackedWidget(this);
    m_windowStack->setObjectName("windowStack");
    m_windowStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 设置日历窗口
    m_calendarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_windowStack->addWidget(m_calendarWidget);
    
    // 创建并设置邮箱窗口
    setupInboxWidget();
    if (m_emailWindow) {
        m_windowStack->addWidget(m_emailWindow);
    }

    // 创建并设置发送邮件窗口
    setupSendEmailWidget();
    if (m_sendEmailWidget) {
        m_windowStack->addWidget(m_sendEmailWidget);
    }
    
    // 创建并设置用户管理窗口
    setupUserManagementWidget();
    if (m_userManagementWidget) {
        m_windowStack->addWidget(m_userManagementWidget);
    }
    
    // 创建并设置添加日程窗口
    setupAddScheduleWidget();
    if (m_addScheduleWidget) {
        m_windowStack->addWidget(m_addScheduleWidget);
    }
    
    // 创建并设置日程表窗口
    setupScheduleListWidget();
    if (m_scheduleListWidget) {
        m_windowStack->addWidget(m_scheduleListWidget);
    }
    
    // 创建并设置设置窗口
    setupSettingsWidget();
    if (m_settingsWidget) {
        m_windowStack->addWidget(m_settingsWidget);
    }
    
    // 将窗口栈添加到布局
    QVBoxLayout* calendarLayout = qobject_cast<QVBoxLayout*>(ui->calendarView->parentWidget()->layout());
    if (calendarLayout) {
        calendarLayout->replaceWidget(ui->calendarView, m_windowStack);
    }
    
    // 设置默认显示日历窗口
    m_windowStack->setCurrentIndex(Window_Calendar);
}

void Widget::switchToWindow(WindowType windowType) {
    if (windowType == m_currentWindow && !m_needsRefresh[windowType]) {
        return;
    }
    
    // 标记当前窗口需要刷新
    markWindowAsRefreshed(m_currentWindow);
    
    // 切换到新窗口
    m_currentWindow = windowType;
    m_windowStack->setCurrentIndex(windowType);
    
    // 清除刷新标记
    m_needsRefresh[windowType] = false;
}

void Widget::markWindowAsRefreshed(WindowType windowType) {
    m_needsRefresh[windowType] = true;
}

void Widget::setupCalendar() {
    // 设置日历的一些属性
    m_calendarWidget->setGridVisible(true);
    m_calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    
    // 获取日历的内部TableView并设置delegate
    m_calendarTableView = m_calendarWidget->findChild<QTableView*>("qt_calendar_calendarview");
    if (m_calendarTableView) {
        m_calendarTableView->setItemDelegate(m_calendarDelegate);
        // 安装事件过滤器来捕获鼠标点击
        m_calendarTableView->viewport()->installEventFilter(this);
    }
    
    // 简化样式表 - 只保留必要的基础样式
    QString calendarStyle = R"(
        QCalendarWidget {
            background-color: #F5F5F5;
            border: 1px solid #333333;
            color: #333333;
        }
        
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background-color: #333333;
            color: #F5F5F5;
            padding: 5px;
        }
        
        QCalendarWidget QPushButton {
            background-color: transparent;
            color: #F5F5F5;
            border: none;
            padding: 5px;
            font-weight: bold;
        }
        
        QCalendarWidget QPushButton:hover {
            background-color: rgba(245, 245, 245, 20);
        }
        
        QCalendarWidget QPushButton:pressed {
            background-color: rgba(245, 245, 245, 40);
        }
        
        QCalendarWidget QWidget#qt_calendar_monthbutton {
            color: #F5F5F5;
            font-weight: bold;
            font-size: 14px;
        }
        
        QCalendarWidget QTableView {
            background-color: transparent;
            border: none;
            color: #333333;
            font-size: 14px;
            gridline-color: rgba(51, 51, 51, 30);
            selection-background-color: #333333;
        }
        
        QCalendarWidget QTableView::item {
            background-color: transparent;
            color: #333333;
            padding: 2px;
            border: none;
        }
        
        QCalendarWidget QHeaderView {
            background-color: #F5F5F5;
            color: #333333;
            font-weight: bold;
        }
        
        QCalendarWidget QHeaderView::section {
            background-color: #F5F5F5;
            color: #333333;
            border: none;
            padding: 5px;
            font-weight: bold;
        }
        
        QCalendarWidget QTableCornerButton::section {
            background-color: #F5F5F5;
            border: none;
        }
    )";
    m_calendarWidget->setStyleSheet(calendarStyle);
}

void Widget::updateCalendarMarks() {
    // 获取当前视图的月份
    int year = m_calendarWidget->yearShown();
    int month = m_calendarWidget->monthShown();
    QDate today = QDate::currentDate();
    
    // 获取日程数据
    QVector<Schedule> schedules = dbManager->getSchedules();
    
    // 更新delegate的数据
    m_calendarDelegate->setCurrentMonth(year, month);
    m_calendarDelegate->setToday(today);
    m_calendarDelegate->setSchedules(schedules);
    
    // 刷新日历视图
    QTableView* tableView = m_calendarWidget->findChild<QTableView*>();
    if (tableView) {
        tableView->viewport()->update();
    }
}

void Widget::on_addScheduleButton_clicked() {
    switchToWindow(Window_AddSchedule);
}

void Widget::on_viewAllSchedulesButton_clicked() {
    switchToWindow(Window_ScheduleList);
}

void Widget::on_settingsButton_clicked() {
    switchToWindow(Window_Settings);
}

void Widget::on_emailButton_clicked() {
    switchToWindow(Window_Inbox);
}

void Widget::on_calendarButton_clicked() {
    switchToWindow(Window_Calendar);
}

void Widget::on_sendEmailButton_clicked() {
    switchToWindow(Window_SendEmail);
}

void Widget::setupSendEmailWidget() {
    if (m_sendEmailWidget) {
        return;
    }
    
    m_sendEmailWidget = new QWidget(this);
    m_sendEmailWidget->setObjectName("sendEmailWidget");
    m_sendEmailWidget->setStyleSheet(R"(
        #sendEmailWidget {
            background-color: #ffffff;
            border: 1px solid #333333;
        }
    )");
    
    m_sendEmailWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QVBoxLayout* layout = new QVBoxLayout(m_sendEmailWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_emailSendWindow = new EmailSendWindow(m_sendEmailWidget);
    m_emailSendWindow->setObjectName("emailSendWindow");
    layout->addWidget(m_emailSendWindow);
}

void Widget::on_userManagementButton_clicked() {
    switchToWindow(Window_UserManagement);
}

void Widget::setupUserManagementWidget() {
    if (m_userManagementWidget) {
        return;
    }
    
    m_userManagementWidget = new QWidget(this);
    m_userManagementWidget->setObjectName("userManagementWidget");
    m_userManagementWidget->setStyleSheet(R"(
        #userManagementWidget {
            background-color: #ffffff;
            border: 1px solid #333333;
        }
    )");
    
    m_userManagementWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QVBoxLayout* layout = new QVBoxLayout(m_userManagementWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_emailCountWindow = new EmailCountWindow(m_userManagementWidget);
    m_emailCountWindow->setObjectName("emailCountWindow");
    layout->addWidget(m_emailCountWindow);
}

void Widget::setupAddScheduleWidget() {
    if (m_addScheduleWidget) {
        return;
    }
    
    m_addScheduleWidget = new QWidget(this);
    m_addScheduleWidget->setObjectName("addScheduleWidget");
    m_addScheduleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    ScheduleDialog* scheduleDialog = new ScheduleDialog(m_addScheduleWidget);
    QVBoxLayout* layout = new QVBoxLayout(m_addScheduleWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scheduleDialog);
}

void Widget::setupScheduleListWidget() {
    if (m_scheduleListWidget) {
        return;
    }
    
    m_scheduleListWidget = new QWidget(this);
    m_scheduleListWidget->setObjectName("scheduleListWidget");
    m_scheduleListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    ScheduleListDialog* scheduleListDialog = new ScheduleListDialog(m_scheduleListWidget);
    QVBoxLayout* layout = new QVBoxLayout(m_scheduleListWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scheduleListDialog);
}

void Widget::setupSettingsWidget() {
    if (m_settingsWidget) {
        return;
    }
    
    m_settingsWidget = new QWidget(this);
    m_settingsWidget->setObjectName("settingsWidget");
    m_settingsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    SettingsDialog* settingsDialog = new SettingsDialog(m_settingsWidget, weatherManager);
    QVBoxLayout* layout = new QVBoxLayout(m_settingsWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(settingsDialog);
}

void Widget::setupInboxWidget() {
    if (m_emailWindow) {
        return;
    }

    m_emailWindow = new EmailBoxWindow(this);
    m_emailWindow->setObjectName("emailWindow");
    m_emailWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void Widget::on_calendarView_selectionChanged() {
    // 该方法已被 onCalendarViewClicked 替代，不再使用
    // 避免在选择日期时弹出提示框
}

void Widget::onCalendarViewClicked(const QDate &date) {
    // 处理日历日期点击事件
    QVector<Schedule> schedules = dbManager->getSchedulesByDate(date);
    
    // 创建并显示日程详情对话框
    ScheduleDetailDialog dialog(date, schedules, this);
    dialog.exec();
    
    // 关闭对话框后刷新日历标记，确保状态正确
    updateCalendarMarks();
}

bool Widget::eventFilter(QObject* watched, QEvent* event)
{
    // 拦截日历TableView的鼠标点击事件
    if (watched == m_calendarTableView->viewport()) {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint pos = mouseEvent->pos();
            
            // 获取点击位置的索引
            QModelIndex index = m_calendarTableView->indexAt(pos);
            
            if (index.isValid()) {
                // 使用CalendarDelegate的方法计算日期
                QDate date = m_calendarDelegate->dateFromIndex(index);
                
                if (date.isValid()) {
                    qDebug() << "EventFilter: Clicked date:" << date 
                             << "Row:" << index.row() << "Col:" << index.column();
                    handleCalendarClick(date);
                    return true; // 事件已处理
                }
            }
        }
    }
    
    return QWidget::eventFilter(watched, event);
}

void Widget::handleCalendarClick(const QDate& date)
{
    qDebug() << "handleCalendarClick:" << date;
    
    QVector<Schedule> schedules = dbManager->getSchedulesByDate(date);
    
    // 创建并显示日程详情对话框
    ScheduleDetailDialog dialog(date, schedules, this);
    dialog.exec();
    
    // 关闭对话框后刷新日历标记，确保状态正确
    updateCalendarMarks();
}

void Widget::onDateClickedFromDelegate(const QDate &date) {
    // 处理从delegate发送的日期点击事件
    // 使用selectedDate检查是否真的点击了新日期
    if (date != m_calendarWidget->selectedDate()) {
        return; // 如果不是选择新日期，不处理
    }
    
    QVector<Schedule> schedules = dbManager->getSchedulesByDate(date);
    
    // 创建并显示日程详情对话框
    ScheduleDetailDialog dialog(date, schedules, this);
    dialog.exec();
    
    // 关闭对话框后刷新日历标记，确保状态正确
    updateCalendarMarks();
}

void Widget::updateWeatherDisplay() {
    if (weatherManager->isEnabled()) {
        // 确保天气组件可见
        ui->weatherWidget->show();
        
        WeatherInfo info = weatherManager->getWeatherInfo();
        if (!info.condition.isEmpty() && !info.temperatureRange.isEmpty()) {
            // 天气数据已加载，显示白天和晚上天气
            // 格式："🌤 晴 - 🌧️ 小雨"
            QString weatherDisplay = QString("%1 - %2")
                .arg(info.condition)
                .arg(info.nightCondition.isEmpty() ? info.condition : info.nightCondition);
            ui->weatherConditionLabel->setText(weatherDisplay);
            ui->temperatureRangeLabel->setText(info.temperatureRange);
            
            // 显示未来三日天气预报（已包含白天和晚上天气）
            QStringList forecasts;
            for (int i = 0; i < 3; ++i) {
                if (!info.forecast[i].isEmpty()) {
                    forecasts << info.forecast[i];
                }
            }
            if (!forecasts.isEmpty()) {
                ui->forecastContentLabel->setText(forecasts.join("\n"));
            }
        } else {
            // 天气数据未加载或正在加载
            ui->weatherConditionLabel->setText("天气加载中...");
            ui->temperatureRangeLabel->setText("--");
            ui->forecastContentLabel->setText("预报加载中...");
        }
    } else {
        // 天气功能未启用
        ui->weatherWidget->hide();
    }
}

void Widget::updateDateInfo() {
    QDate today = QDate::currentDate();
    
    QString dateStr = QString("%1 月 %2 日").arg(today.month()).arg(today.day());
    ui->dateLabel->setText(dateStr);
    
    QString lunarDate = m_lunarCalendar->getLunarDateString(today);
    ui->lunarLabel->setText(lunarDate);
    
    QString solarTerm = m_lunarCalendar->getSolarTerm(today);
    if (solarTerm.isEmpty()) {
        ui->solarTermLabel->clear();
    } else {
        ui->solarTermLabel->setText(solarTerm);
    }
}

void Widget::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_T) {
        qDebug() << "Ctrl+T pressed - showing test notification";
        reminderManager->testNotification();
        event->accept();
    } else {
        QWidget::keyPressEvent(event);
    }
}
