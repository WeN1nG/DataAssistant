/********************************************************************************
** Form generated from reading UI file 'SettingsDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSDIALOG_H
#define UI_SETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SettingsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *weatherGroupBox;
    QVBoxLayout *verticalLayout_2;
    QCheckBox *weatherEnabledCheckBox;
    QFormLayout *formLayout;
    QLabel *apiKeyLabel;
    QLineEdit *apiKeyLineEdit;
    QLabel *cityLabel;
    QLineEdit *cityLineEdit;
    QGroupBox *backupGroupBox;
    QVBoxLayout *verticalLayout_3;
    QPushButton *backupButton;
    QPushButton *restoreButton;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *saveButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *SettingsDialog)
    {
        if (SettingsDialog->objectName().isEmpty())
            SettingsDialog->setObjectName("SettingsDialog");
        SettingsDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(SettingsDialog);
        verticalLayout->setObjectName("verticalLayout");
        weatherGroupBox = new QGroupBox(SettingsDialog);
        weatherGroupBox->setObjectName("weatherGroupBox");
        verticalLayout_2 = new QVBoxLayout(weatherGroupBox);
        verticalLayout_2->setObjectName("verticalLayout_2");
        weatherEnabledCheckBox = new QCheckBox(weatherGroupBox);
        weatherEnabledCheckBox->setObjectName("weatherEnabledCheckBox");

        verticalLayout_2->addWidget(weatherEnabledCheckBox);

        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        apiKeyLabel = new QLabel(weatherGroupBox);
        apiKeyLabel->setObjectName("apiKeyLabel");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, apiKeyLabel);

        apiKeyLineEdit = new QLineEdit(weatherGroupBox);
        apiKeyLineEdit->setObjectName("apiKeyLineEdit");
        apiKeyLineEdit->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, apiKeyLineEdit);

        cityLabel = new QLabel(weatherGroupBox);
        cityLabel->setObjectName("cityLabel");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, cityLabel);

        cityLineEdit = new QLineEdit(weatherGroupBox);
        cityLineEdit->setObjectName("cityLineEdit");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, cityLineEdit);


        verticalLayout_2->addLayout(formLayout);


        verticalLayout->addWidget(weatherGroupBox);

        backupGroupBox = new QGroupBox(SettingsDialog);
        backupGroupBox->setObjectName("backupGroupBox");
        verticalLayout_3 = new QVBoxLayout(backupGroupBox);
        verticalLayout_3->setObjectName("verticalLayout_3");
        backupButton = new QPushButton(backupGroupBox);
        backupButton->setObjectName("backupButton");

        verticalLayout_3->addWidget(backupButton);

        restoreButton = new QPushButton(backupGroupBox);
        restoreButton->setObjectName("restoreButton");

        verticalLayout_3->addWidget(restoreButton);


        verticalLayout->addWidget(backupGroupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        saveButton = new QPushButton(SettingsDialog);
        saveButton->setObjectName("saveButton");

        horizontalLayout->addWidget(saveButton);

        cancelButton = new QPushButton(SettingsDialog);
        cancelButton->setObjectName("cancelButton");

        horizontalLayout->addWidget(cancelButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(SettingsDialog);

        QMetaObject::connectSlotsByName(SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingsDialog)
    {
        SettingsDialog->setWindowTitle(QCoreApplication::translate("SettingsDialog", "\350\256\276\347\275\256", nullptr));
        weatherGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "\345\244\251\346\260\224\350\256\276\347\275\256", nullptr));
        weatherEnabledCheckBox->setText(QCoreApplication::translate("SettingsDialog", "\345\220\257\347\224\250\345\244\251\346\260\224\346\217\220\351\206\222", nullptr));
        apiKeyLabel->setText(QCoreApplication::translate("SettingsDialog", "\351\253\230\345\276\267 API Key:", nullptr));
        apiKeyLineEdit->setPlaceholderText(QCoreApplication::translate("SettingsDialog", "\350\257\267\350\276\223\345\205\245\351\253\230\345\276\267 API Key", nullptr));
        cityLabel->setText(QCoreApplication::translate("SettingsDialog", "\345\237\216\345\270\202:", nullptr));
        cityLineEdit->setPlaceholderText(QCoreApplication::translate("SettingsDialog", "\350\257\267\350\276\223\345\205\245\345\237\216\345\270\202\345\220\215\347\247\260", nullptr));
        backupGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "\346\225\260\346\215\256\345\244\207\344\273\275", nullptr));
        backupButton->setText(QCoreApplication::translate("SettingsDialog", "\345\244\207\344\273\275\346\225\260\346\215\256", nullptr));
        restoreButton->setText(QCoreApplication::translate("SettingsDialog", "\346\201\242\345\244\215\346\225\260\346\215\256", nullptr));
        saveButton->setText(QCoreApplication::translate("SettingsDialog", "\344\277\235\345\255\230", nullptr));
        cancelButton->setText(QCoreApplication::translate("SettingsDialog", "\345\217\226\346\266\210", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSDIALOG_H
