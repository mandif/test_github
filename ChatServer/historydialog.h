#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QListWidget>

namespace Ui {
class HistoryDialog;
}

class HistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget *parent = nullptr);
    void setHistory(const QList<QString> &history);

private:
    Ui::HistoryDialog *ui;
};

#endif // HISTORYDIALOG_H
