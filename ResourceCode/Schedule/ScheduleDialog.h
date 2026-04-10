#ifndef SCHEDULEDIALOG_H
#define SCHEDULEDIALOG_H

#include <QDialog>
#include "DatabaseManager.h"

namespace Ui {
class ScheduleDialog;
}

class ScheduleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduleDialog(QWidget *parent = nullptr, Schedule *schedule = nullptr);
    ~ScheduleDialog();

private slots:
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void on_batchAddCheckBox_stateChanged(int state);

private:
    Ui::ScheduleDialog *ui;
    Schedule *currentSchedule;
    bool isEditing;
    bool validateBatchDates();
    int getDaysInRange(const QDateTime& start, const QDateTime& end);
};

#endif // SCHEDULEDIALOG_H