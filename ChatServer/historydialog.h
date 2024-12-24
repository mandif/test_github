#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QJsonObject>
#include <QList>
#include "chatserver.h"

namespace Ui {
class HistoryDialog;
}

class HistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget *parent = nullptr);
    void setHistory(const QList<QJsonObject> &history);
    void updateHistory(const QJsonObject &history);

private slots:
    void on_back_clicked();

private:
    Ui::HistoryDialog *ui;
};

#endif // HISTORYDIALOG_H
