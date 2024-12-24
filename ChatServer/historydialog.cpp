#include "historydialog.h"
#include "qjsonobject.h"
#include "ui_historydialog.h"

HistoryDialog::HistoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HistoryDialog)
{
    ui->setupUi(this);
}

void HistoryDialog::setHistory(const QList<QJsonObject> &history) {
    for (const QJsonObject &msg : history) {

        // 检查键的存在
        if (!msg.contains("type") || !msg.contains("sender") ||
            !msg.contains("reciver") || !msg.contains("text") ||
            !msg.contains("time")) {
            continue;  // 如果任何键不存在，跳过该消息
        }

        const QJsonValue typeVal = msg.value("type");
        const QJsonValue senderVal = msg.value("sender");
        const QJsonValue reciverVal = msg.value("reciver");
        const QJsonValue textVal = msg.value("text");
        const QJsonValue timeVal = msg.value("time");

        QString type = typeVal.toString(); // 获取消息类型
        QString sender = senderVal.toString(); // 获取发送者
        QString reciver = reciverVal.toString(); // 获取接收者
        QString text = textVal.toString(); // 获取消息
        QString time = timeVal.toString(); // 获取时间

        // 格式化显示字符串
        QString displayString = QString("[%1] From: %2 To: %3 Type: %4 Message: %5")
                                    .arg(time, sender, reciver, type, text);

        // 添加到历史消息 QListWidget
        ui->listWidget->addItem(displayString);

    }
}

//如果历史记录框是打开的，更新历史记录
void HistoryDialog::updateHistory(const QJsonObject &history)
{
    const QJsonValue typeVal = history.value("type");
    const QJsonValue senderVal = history.value("sender");
    const QJsonValue reciverVal = history.value("reciver");
    const QJsonValue textVal = history.value("text");
    const QJsonValue timeVal = history.value("time");

    QString type = typeVal.toString(); // 获取消息类型
    QString sender = senderVal.toString(); // 获取发送者
    QString reciver = reciverVal.toString(); // 获取接收者
    QString text = textVal.toString(); // 获取消息
    QString time = timeVal.toString(); // 获取时间

    // 格式化显示字符串
    QString displayString = QString("[%1] From: %2 To: %3 Type: %4 Message: %5")
                                .arg(time, sender, reciver, type, text);

    // 添加到历史消息 QListWidget
    ui->listWidget->addItem(displayString);

}

void HistoryDialog::on_back_clicked()
{
    accept();  // 关闭对话框并发出 accepted() 信号
}

