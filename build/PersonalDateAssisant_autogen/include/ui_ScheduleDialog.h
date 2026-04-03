/********************************************************************************
** Form generated from reading UI file 'ScheduleDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCHEDULEDIALOG_H
#define UI_SCHEDULEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ScheduleDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *titleLineEdit;
    QTextEdit *descriptionTextEdit;
    QDateTimeEdit *dateTimeEdit;
    QHBoxLayout *horizontalLayout;
    QLabel *priorityLabel;
    QComboBox *priorityComboBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *reminderLabel;
    QComboBox *reminderComboBox;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer;
    QPushButton *saveButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *ScheduleDialog)
    {
        if (ScheduleDialog->objectName().isEmpty())
            ScheduleDialog->setObjectName("ScheduleDialog");
        ScheduleDialog->resize(400, 350);
        verticalLayout = new QVBoxLayout(ScheduleDialog);
        verticalLayout->setObjectName("verticalLayout");
        groupBox = new QGroupBox(ScheduleDialog);
        groupBox->setObjectName("groupBox");
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName("verticalLayout_2");
        titleLineEdit = new QLineEdit(groupBox);
        titleLineEdit->setObjectName("titleLineEdit");

        verticalLayout_2->addWidget(titleLineEdit);

        descriptionTextEdit = new QTextEdit(groupBox);
        descriptionTextEdit->setObjectName("descriptionTextEdit");

        verticalLayout_2->addWidget(descriptionTextEdit);

        dateTimeEdit = new QDateTimeEdit(groupBox);
        dateTimeEdit->setObjectName("dateTimeEdit");
        dateTimeEdit->setCalendarPopup(true);

        verticalLayout_2->addWidget(dateTimeEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        priorityLabel = new QLabel(groupBox);
        priorityLabel->setObjectName("priorityLabel");

        horizontalLayout->addWidget(priorityLabel);

        priorityComboBox = new QComboBox(groupBox);
        priorityComboBox->addItem(QString());
        priorityComboBox->addItem(QString());
        priorityComboBox->addItem(QString());
        priorityComboBox->setObjectName("priorityComboBox");

        horizontalLayout->addWidget(priorityComboBox);


        verticalLayout_2->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        reminderLabel = new QLabel(groupBox);
        reminderLabel->setObjectName("reminderLabel");

        horizontalLayout_2->addWidget(reminderLabel);

        reminderComboBox = new QComboBox(groupBox);
        reminderComboBox->addItem(QString());
        reminderComboBox->addItem(QString());
        reminderComboBox->addItem(QString());
        reminderComboBox->addItem(QString());
        reminderComboBox->addItem(QString());
        reminderComboBox->addItem(QString());
        reminderComboBox->setObjectName("reminderComboBox");

        horizontalLayout_2->addWidget(reminderComboBox);


        verticalLayout_2->addLayout(horizontalLayout_2);


        verticalLayout->addWidget(groupBox);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        saveButton = new QPushButton(ScheduleDialog);
        saveButton->setObjectName("saveButton");

        horizontalLayout_3->addWidget(saveButton);

        cancelButton = new QPushButton(ScheduleDialog);
        cancelButton->setObjectName("cancelButton");

        horizontalLayout_3->addWidget(cancelButton);


        verticalLayout->addLayout(horizontalLayout_3);


        retranslateUi(ScheduleDialog);

        QMetaObject::connectSlotsByName(ScheduleDialog);
    } // setupUi

    void retranslateUi(QDialog *ScheduleDialog)
    {
        ScheduleDialog->setWindowTitle(QCoreApplication::translate("ScheduleDialog", "\347\274\226\350\276\221\346\227\245\347\250\213", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ScheduleDialog", "\346\227\245\347\250\213\344\277\241\346\201\257", nullptr));
        titleLineEdit->setPlaceholderText(QCoreApplication::translate("ScheduleDialog", "\350\257\267\350\276\223\345\205\245\346\227\245\347\250\213\346\240\207\351\242\230", nullptr));
        descriptionTextEdit->setPlaceholderText(QCoreApplication::translate("ScheduleDialog", "\350\257\267\350\276\223\345\205\245\346\227\245\347\250\213\346\217\217\350\277\260", nullptr));
        priorityLabel->setText(QCoreApplication::translate("ScheduleDialog", "\347\264\247\350\246\201\347\250\213\345\272\246:", nullptr));
        priorityComboBox->setItemText(0, QCoreApplication::translate("ScheduleDialog", "\344\270\200\350\210\254", nullptr));
        priorityComboBox->setItemText(1, QCoreApplication::translate("ScheduleDialog", "\351\207\215\350\246\201", nullptr));
        priorityComboBox->setItemText(2, QCoreApplication::translate("ScheduleDialog", "\347\264\247\346\200\245", nullptr));

        reminderLabel->setText(QCoreApplication::translate("ScheduleDialog", "\346\217\220\351\206\222\346\227\266\351\227\264:", nullptr));
        reminderComboBox->setItemText(0, QCoreApplication::translate("ScheduleDialog", "\344\270\215\346\217\220\351\206\222", nullptr));
        reminderComboBox->setItemText(1, QCoreApplication::translate("ScheduleDialog", "\346\217\220\345\211\2155\345\210\206\351\222\237", nullptr));
        reminderComboBox->setItemText(2, QCoreApplication::translate("ScheduleDialog", "\346\217\220\345\211\21515\345\210\206\351\222\237", nullptr));
        reminderComboBox->setItemText(3, QCoreApplication::translate("ScheduleDialog", "\346\217\220\345\211\21530\345\210\206\351\222\237", nullptr));
        reminderComboBox->setItemText(4, QCoreApplication::translate("ScheduleDialog", "\346\217\220\345\211\2151\345\260\217\346\227\266", nullptr));
        reminderComboBox->setItemText(5, QCoreApplication::translate("ScheduleDialog", "\346\217\220\345\211\2151\345\244\251", nullptr));

        saveButton->setText(QCoreApplication::translate("ScheduleDialog", "\344\277\235\345\255\230", nullptr));
        cancelButton->setText(QCoreApplication::translate("ScheduleDialog", "\345\217\226\346\266\210", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ScheduleDialog: public Ui_ScheduleDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCHEDULEDIALOG_H
