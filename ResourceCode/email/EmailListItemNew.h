#ifndef EMAILLISTITEMNEW_H
#define EMAILLISTITEMNEW_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include "EmailModels.h"

class EmailListItemNew : public QFrame
{
    Q_OBJECT

public:
    explicit EmailListItemNew(QWidget* parent = nullptr);
    ~EmailListItemNew();

    void setEmail(const Email& email);
    Email getEmail() const { return m_email; }

signals:
    void viewClicked(const Email& email);
    void deleteClicked(const Email& email);

private slots:
    void onViewClicked();
    void onDeleteClicked();

private:
    void updateDisplay();
    QString formatDisplayDate(const QDateTime& dateTime);

private:
    Email m_email;

    QLabel* m_senderLabel;
    QLabel* m_recipientLabel;
    QLabel* m_timeLabel;
    QLabel* m_statusLabel;
    QLabel* m_subjectLabel;
    QPushButton* m_viewBtn;
    QPushButton* m_deleteBtn;

    QVBoxLayout* m_mainLayout;
};

#endif
