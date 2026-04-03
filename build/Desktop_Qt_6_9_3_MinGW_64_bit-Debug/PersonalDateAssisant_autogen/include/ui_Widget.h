/********************************************************************************
** Form generated from reading UI file 'Widget.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCalendarWidget>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QHBoxLayout *horizontalLayout;
    QWidget *toolBarWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *addScheduleButton;
    QPushButton *viewAllSchedulesButton;
    QPushButton *settingsButton;
    QSpacerItem *verticalSpacer;
    QWidget *calendarWidget;
    QVBoxLayout *verticalLayout_2;
    QCalendarWidget *calendarView;
    QWidget *weatherWidget;
    QVBoxLayout *verticalLayout_3;
    QWidget *weatherHeaderWidget;
    QHBoxLayout *horizontalLayout_2;
    QLabel *weatherIconLabel;
    QLabel *temperatureLabel;
    QWidget *weatherContentWidget;
    QVBoxLayout *verticalLayout_4;
    QLabel *weatherConditionLabel;
    QLabel *dateLabel;
    QFrame *line;
    QLabel *forecastLabel;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(1000, 600);
        horizontalLayout = new QHBoxLayout(Widget);
        horizontalLayout->setObjectName("horizontalLayout");
        toolBarWidget = new QWidget(Widget);
        toolBarWidget->setObjectName("toolBarWidget");
        toolBarWidget->setMinimumSize(QSize(80, 0));
        toolBarWidget->setMaximumSize(QSize(80, 16777215));
        verticalLayout = new QVBoxLayout(toolBarWidget);
        verticalLayout->setObjectName("verticalLayout");
        addScheduleButton = new QPushButton(toolBarWidget);
        addScheduleButton->setObjectName("addScheduleButton");
        addScheduleButton->setMinimumSize(QSize(70, 60));
        addScheduleButton->setMaximumSize(QSize(70, 60));
        addScheduleButton->setIconSize(QSize(32, 32));

        verticalLayout->addWidget(addScheduleButton);

        viewAllSchedulesButton = new QPushButton(toolBarWidget);
        viewAllSchedulesButton->setObjectName("viewAllSchedulesButton");
        viewAllSchedulesButton->setMinimumSize(QSize(70, 60));
        viewAllSchedulesButton->setMaximumSize(QSize(70, 60));
        viewAllSchedulesButton->setIconSize(QSize(32, 32));

        verticalLayout->addWidget(viewAllSchedulesButton);

        settingsButton = new QPushButton(toolBarWidget);
        settingsButton->setObjectName("settingsButton");
        settingsButton->setMinimumSize(QSize(70, 60));
        settingsButton->setMaximumSize(QSize(70, 60));
        settingsButton->setIconSize(QSize(32, 32));

        verticalLayout->addWidget(settingsButton);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout->addWidget(toolBarWidget);

        calendarWidget = new QWidget(Widget);
        calendarWidget->setObjectName("calendarWidget");
        calendarWidget->setMinimumSize(QSize(700, 0));
        verticalLayout_2 = new QVBoxLayout(calendarWidget);
        verticalLayout_2->setObjectName("verticalLayout_2");
        calendarView = new QCalendarWidget(calendarWidget);
        calendarView->setObjectName("calendarView");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(calendarView->sizePolicy().hasHeightForWidth());
        calendarView->setSizePolicy(sizePolicy);

        verticalLayout_2->addWidget(calendarView);


        horizontalLayout->addWidget(calendarWidget);

        weatherWidget = new QWidget(Widget);
        weatherWidget->setObjectName("weatherWidget");
        weatherWidget->setMinimumSize(QSize(200, 0));
        weatherWidget->setMaximumSize(QSize(200, 16777215));
        weatherWidget->setStyleSheet(QString::fromUtf8("#weatherWidget {\n"
" background-color: #f8f9fa;\n"
" border-radius: 12px;\n"
" padding: 0px;\n"
"}\n"
""));
        verticalLayout_3 = new QVBoxLayout(weatherWidget);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        weatherHeaderWidget = new QWidget(weatherWidget);
        weatherHeaderWidget->setObjectName("weatherHeaderWidget");
        weatherHeaderWidget->setMinimumSize(QSize(0, 60));
        weatherHeaderWidget->setMaximumSize(QSize(16777215, 60));
        weatherHeaderWidget->setStyleSheet(QString::fromUtf8("#weatherHeaderWidget {\n"
" background-color: #ffffff;\n"
" border-top-left-radius: 12px;\n"
" border-top-right-radius: 12px;\n"
" border-bottom: 1px solid #e9ecef;\n"
" padding: 0px 16px;\n"
"}"));
        horizontalLayout_2 = new QHBoxLayout(weatherHeaderWidget);
        horizontalLayout_2->setSpacing(8);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(16, 12, 16, 12);
        weatherIconLabel = new QLabel(weatherHeaderWidget);
        weatherIconLabel->setObjectName("weatherIconLabel");
        weatherIconLabel->setMinimumSize(QSize(36, 36));
        weatherIconLabel->setMaximumSize(QSize(36, 36));
        QFont font;
        font.setPointSize(24);
        weatherIconLabel->setFont(font);
        weatherIconLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_2->addWidget(weatherIconLabel);

        temperatureLabel = new QLabel(weatherHeaderWidget);
        temperatureLabel->setObjectName("temperatureLabel");
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Segoe UI")});
        font1.setPointSize(28);
        font1.setBold(true);
        temperatureLabel->setFont(font1);
        temperatureLabel->setStyleSheet(QString::fromUtf8("color: #2c3e50;"));
        temperatureLabel->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);

        horizontalLayout_2->addWidget(temperatureLabel);


        verticalLayout_3->addWidget(weatherHeaderWidget);

        weatherContentWidget = new QWidget(weatherWidget);
        weatherContentWidget->setObjectName("weatherContentWidget");
        weatherContentWidget->setStyleSheet(QString::fromUtf8("#weatherContentWidget {\n"
" background-color: transparent;\n"
" padding: 16px;\n"
"}"));
        verticalLayout_4 = new QVBoxLayout(weatherContentWidget);
        verticalLayout_4->setSpacing(12);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        weatherConditionLabel = new QLabel(weatherContentWidget);
        weatherConditionLabel->setObjectName("weatherConditionLabel");
        QFont font2;
        font2.setFamilies({QString::fromUtf8("Microsoft YaHei")});
        font2.setPointSize(20);
        font2.setBold(true);
        weatherConditionLabel->setFont(font2);
        weatherConditionLabel->setStyleSheet(QString::fromUtf8("color: #2c3e50;\n"
"padding: 8px 0px;"));
        weatherConditionLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_4->addWidget(weatherConditionLabel);

        dateLabel = new QLabel(weatherContentWidget);
        dateLabel->setObjectName("dateLabel");
        QFont font3;
        font3.setFamilies({QString::fromUtf8("Microsoft YaHei")});
        font3.setPointSize(11);
        dateLabel->setFont(font3);
        dateLabel->setStyleSheet(QString::fromUtf8("color: #868e96;\n"
"padding: 4px 0px;"));
        dateLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_4->addWidget(dateLabel);

        line = new QFrame(weatherContentWidget);
        line->setObjectName("line");
        line->setStyleSheet(QString::fromUtf8("color: #e9ecef;"));
        line->setFrameShape(QFrame::Shape::HLine);
        line->setFrameShadow(QFrame::Shadow::Sunken);

        verticalLayout_4->addWidget(line);

        forecastLabel = new QLabel(weatherContentWidget);
        forecastLabel->setObjectName("forecastLabel");
        QFont font4;
        font4.setFamilies({QString::fromUtf8("Microsoft YaHei")});
        font4.setPointSize(9);
        forecastLabel->setFont(font4);
        forecastLabel->setStyleSheet(QString::fromUtf8("color: #6c757d;\n"
"padding: 4px 0px;\n"
"line-height: 1.5;"));
        forecastLabel->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
        forecastLabel->setWordWrap(true);

        verticalLayout_4->addWidget(forecastLabel);


        verticalLayout_3->addWidget(weatherContentWidget);


        horizontalLayout->addWidget(weatherWidget);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "\344\270\252\344\272\272\346\227\245\347\250\213\345\212\251\346\211\213", nullptr));
#if QT_CONFIG(tooltip)
        addScheduleButton->setToolTip(QCoreApplication::translate("Widget", "\346\267\273\345\212\240\346\227\245\347\250\213", nullptr));
#endif // QT_CONFIG(tooltip)
        addScheduleButton->setText(QString());
#if QT_CONFIG(tooltip)
        viewAllSchedulesButton->setToolTip(QCoreApplication::translate("Widget", "\346\237\245\347\234\213\346\211\200\346\234\211\346\227\245\347\250\213", nullptr));
#endif // QT_CONFIG(tooltip)
        viewAllSchedulesButton->setText(QString());
#if QT_CONFIG(tooltip)
        settingsButton->setToolTip(QCoreApplication::translate("Widget", "\350\256\276\347\275\256", nullptr));
#endif // QT_CONFIG(tooltip)
        settingsButton->setText(QString());
        weatherIconLabel->setText(QCoreApplication::translate("Widget", "\360\237\214\244\357\270\217", nullptr));
        temperatureLabel->setText(QCoreApplication::translate("Widget", "25\302\260C", nullptr));
        weatherConditionLabel->setText(QCoreApplication::translate("Widget", "\346\231\264", nullptr));
        dateLabel->setText(QString());
        forecastLabel->setText(QCoreApplication::translate("Widget", "\346\234\252\346\235\245\344\270\211\345\244\251\345\244\251\346\260\224", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
