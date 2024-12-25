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
    void showSearchHistory(const QList<QJsonObject> &history);
    void setHistoryText(const QList<QJsonObject> &history);

private slots:
    void on_back_clicked();



    //点击搜索按钮进行历史记录搜索
    void on_searchButton_clicked();

    void on_reset_clicked();

private:
    Ui::HistoryDialog *ui;

    // 存储历史记录的列表
    QList<QJsonObject> historyList;
};

#endif // HISTORYDIALOG_H
