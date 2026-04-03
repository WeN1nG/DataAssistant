#include "Widget.h"
#include "./ui_Widget.h"
#include "ScheduleDialog.h"
#include "ReminderManager.h"
#include "WeatherManager.h"
#include "SettingsDialog.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QMenu>
#include <QTextCharFormat>
#include <QMap>
#include <QColor>

void Widget::setupToolBarButtons() {
    // 设置按钮样式
    QString buttonStyle = R"(
        QPushButton {
            border: 1px solid #cccccc;
            background-color: #f5f5f5;
            text-align: center;
            padding: 8px 4px;
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
    
    // 设置添加日程按钮（纯文字）
    ui->addScheduleButton->setText("添加日程");
    ui->addScheduleButton->setStyleSheet(buttonStyle);
    ui->addScheduleButton->setToolTip("添加日程");
    
    // 设置查看日程按钮（纯文字）
    ui->viewAllSchedulesButton->setText("查看日程");
    ui->viewAllSchedulesButton->setStyleSheet(buttonStyle);
    ui->viewAllSchedulesButton->setToolTip("查看所有日程");
    
    // 设置设置按钮（纯文字）
    ui->settingsButton->setText("设置");
    ui->settingsButton->setStyleSheet(buttonStyle);
    ui->settingsButton->setToolTip("设置");
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , dbManager(new DatabaseManager())
    , reminderManager(new ReminderManager(this))
    , weatherManager(new WeatherManager(this))
{
    ui->setupUi(this);
    setupCalendar();
    updateCalendarMarks();
    reminderManager->startMonitoring();
    
    connect(weatherManager, &WeatherManager::weatherUpdated, this, &Widget::updateWeatherDisplay);
    connect(weatherManager, &WeatherManager::errorOccurred, [=](const QString &error) {
        QMessageBox::warning(this, "天气更新失败", error);
    });
    
    // 连接日历点击事件
    connect(ui->calendarView, &QCalendarWidget::clicked, this, &Widget::onCalendarViewClicked);
    
    // 设置工具栏按钮（使用黄金分割比例）
    setupToolBarButtons();
    
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
    delete ui;
}

void Widget::setupCalendar() {
    // 设置日历的一些属性
    ui->calendarView->setGridVisible(true);
    ui->calendarView->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
}

void Widget::updateCalendarMarks() {
    // 获取当前月份的所有日程
    QDate currentDate = QDate::currentDate();
    QDate firstDay = QDate(currentDate.year(), currentDate.month(), 1);
    QDate lastDay = firstDay.addMonths(1).addDays(-1);
    
    QVector<Schedule> schedules = dbManager->getSchedules();
    
    // 统计每天的优先级数量
    QMap<QDate, QMap<int, int>> datePriorityCount;
    
    for (const Schedule& schedule : schedules) {
        if (schedule.datetime.date() >= firstDay && schedule.datetime.date() <= lastDay) {
            QDate date = schedule.datetime.date();
            datePriorityCount[date][schedule.priority]++;
        }
    }
    
    // 为有日程的日期设置标记
    for (auto it = datePriorityCount.begin(); it != datePriorityCount.end(); ++it) {
        QDate date = it.key();
        QMap<int, int> priorityCounts = it.value();
        
        // 获取最高优先级
        int maxPriority = 0;
        if (priorityCounts.contains(2)) maxPriority = 2;
        else if (priorityCounts.contains(1)) maxPriority = 1;
        
        // 计算总数量
        int totalCount = 0;
        for (int count : priorityCounts.values()) {
            totalCount += count;
        }
        
        // 根据最高优先级设置颜色
        QTextCharFormat format;
        if (maxPriority == 2) {
            // 紧急：红色小点
            format.setForeground(Qt::red);
        } else if (maxPriority == 1) {
            // 重要：橙色小点
            format.setForeground(QColor(255, 165, 0));
        } else {
            // 一般：蓝色小点
            format.setForeground(Qt::blue);
        }
        
        // 设置字体大小为 8 用于显示小点
        format.setFontPointSize(8);
        
        // 使用特殊字符作为标记
        QString markText = QString("● %1").arg(totalCount);
        format.setFontUnderline(false);
        
        ui->calendarView->setDateTextFormat(date, format);
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

void Widget::updateWeatherDisplay() {
    if (weatherManager->isEnabled()) {
        WeatherInfo info = weatherManager->getWeatherInfo();
        ui->temperatureLabel->setText(info.temperature);
        ui->weatherConditionLabel->setText(info.condition);
        
        // 显示完整的三天预报，每行一个
        QString forecastText = QString("未来三天天气：%1\n%2\n%3")
            .arg(info.forecast[0])
            .arg(info.forecast[1])
            .arg(info.forecast[2]);
        ui->forecastLabel->setText(forecastText);
        
        ui->dateLabel->setText(QDate::currentDate().toString("yyyy 年 MM 月 dd 日"));
        
        // 根据天气情况设置图标
        if (info.condition.contains("晴")) {
            ui->weatherIconLabel->setText("☀️");
        } else if (info.condition.contains("云")) {
            ui->weatherIconLabel->setText("☁️");
        } else if (info.condition.contains("雨")) {
            ui->weatherIconLabel->setText("🌧️");
        } else {
            ui->weatherIconLabel->setText("🌤️");
        }
    } else {
        ui->weatherWidget->hide();
    }
}
