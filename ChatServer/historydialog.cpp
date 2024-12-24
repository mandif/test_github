#include "historydialog.h"
#include "ui_historydialog.h"

HistoryDialog::HistoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HistoryDialog)
{
    ui->setupUi(this);
}

void HistoryDialog::setHistory(const QList<QString> &history) {
    for (const QString &msg : history) {
        ui->listWidget->addItem(msg); // 添加历史消息到 ListWidget
    }
}
