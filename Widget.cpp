#include "Widget.h"
#include "./ui_Widget.h"
#include "ScheduleDialog.h"
#include "ReminderManager.h"
#include "WeatherManager.h"
#include "SettingsDialog.h"
#include "QtAwesome.h"
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
}

void Widget::setupAwesomeIcons() {
    // 设置图标样式属性
    m_awesome->setDefaultOption("color", QColor(85, 85, 85));
    m_awesome->setDefaultOption("color-selected", QColor(255, 255, 255));
    m_awesome->setDefaultOption("scale-factor", 0.8);

    // 添加按钮图标 (fa_plus)
    QIcon addIcon = m_awesome->icon(fa_solid, fa_plus);
    ui->addScheduleButton->setIcon(addIcon);
    ui->addScheduleButton->setIconSize(QSize(24, 24));

    // 日历按钮图标 (fa_calendar)
    QIcon calendarIcon = m_awesome->icon(fa_solid, fa_calendar);
    ui->viewAllSchedulesButton->setIcon(calendarIcon);
    ui->viewAllSchedulesButton->setIconSize(QSize(24, 24));

    // 齿轮/设置按钮图标 (fa_cog)
    QIcon settingsIcon = m_awesome->icon(fa_solid, fa_cog);
    ui->settingsButton->setIcon(settingsIcon);
    ui->settingsButton->setIconSize(QSize(24, 24));

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

    ui->addScheduleButton->setStyleSheet(iconButtonStyle);
    ui->viewAllSchedulesButton->setStyleSheet(iconButtonStyle);
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
{
    ui->setupUi(this);
    
    // 使用标准QCalendarWidget
    QVBoxLayout* calendarLayout = qobject_cast<QVBoxLayout*>(ui->calendarView->parentWidget()->layout());
    if (calendarLayout) {
        // 隐藏原来的日历
        ui->calendarView->hide();
        
        // 设置日历的大小策略
        m_calendarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
        // 将日历添加到布局
        calendarLayout->replaceWidget(ui->calendarView, m_calendarWidget);
    }
    
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
}

Widget::~Widget()
{
    delete weatherManager;
    delete reminderManager;
    delete dbManager;
    delete m_awesome;
    delete m_calendarDelegate;
    delete m_lunarCalendar;
    delete ui;
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
    ScheduleDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        updateCalendarMarks();
    }
}

void Widget::on_viewAllSchedulesButton_clicked() {
    // 使用新的日程列表对话框
    ScheduleListDialog dialog(this);
    dialog.exec();
    
    // 刷新日历标记
    updateCalendarMarks();
}

void Widget::on_settingsButton_clicked() {
    SettingsDialog dialog(this, weatherManager);
    dialog.exec();
    
    if (weatherManager->isEnabled()) {
        ui->weatherWidget->show();
    } else {
        ui->weatherWidget->hide();
    }
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
            // 天气数据已加载
            ui->weatherConditionLabel->setText(info.condition);
            ui->temperatureRangeLabel->setText(info.temperatureRange);
            
            // 显示未来三日天气预报
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
