#include "historydialog.h"
#include "qjsonobject.h"
#include "ui_historydialog.h"

HistoryDialog::HistoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HistoryDialog)
{
    ui->setupUi(this);
    ui->dateTime->clear();
}

//提取出将历史记录显示在ListWidget中的操作
void HistoryDialog::setHistoryText(const QList<QJsonObject> &history)
{
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

void HistoryDialog::setHistory(const QList<QJsonObject> &history) {
    this->historyList = history;  // 存储原始历史记录
    setHistoryText(history);
}

//如果历史记录框是打开的，更新历史记录
void HistoryDialog::updateHistory(const QJsonObject &history)
{
    //更新历史记录
    this->historyList.append(history);

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

//点击搜索按钮进行历史记录搜索
void HistoryDialog::on_searchButton_clicked()
{
    QString username = ui->userName->text().trimmed();  // 去除首尾空格
    QString timeString;
    QString keyword = ui->keywords->text().trimmed();

    // 获取 QDateTimeEdit 的值并格式化为字符串
    if(ui->dateTime->dateTime().isValid()) {
        QDateTime dateTime = ui->dateTime->dateTime();
        timeString = dateTime.toString("yyyy-MM-dd");
    }

    qDebug() << "搜索名字：" << username << "时间：" << timeString << "关键字：" << keyword;

    //三个参数为空时，返回全部历史记录
    if(username.isEmpty() && timeString == "2000-01-01" && keyword.isEmpty()){
        showSearchHistory(historyList);
        return ;
    }

    QList<QJsonObject> filteredHistory; // 用于存储过滤后的历史记录

    // 遍历历史记录，对其进行筛选
    for (const QJsonObject &msg : historyList) {
        bool matches = true;

        // 根据用户名进行模糊匹配
        if (!username.isEmpty()) {
            QString sender = msg.value("sender").toString();
            QString receiver = msg.value("reciver").toString();
            if (!sender.contains(username, Qt::CaseInsensitive) && !receiver.contains(username, Qt::CaseInsensitive)) {
                qDebug() << "是到用户名这里吗——————————————————————————";
                matches = false;
            }
        }

        qDebug() << "时间模糊匹配上了吗：" << msg.value("time").toString().contains(timeString, Qt::CaseInsensitive);

        // 根据时间进行模糊匹配
        if (timeString != "2000-01-01" && !msg.value("time").toString().contains(timeString, Qt::CaseInsensitive)) {
            qDebug() << "是到时间这里吗——————————————————————————";
            matches = false;
        }

        // 根据关键字进行模糊匹配
        if (!keyword.isEmpty() && !msg.value("text").toString().contains(keyword, Qt::CaseInsensitive)) {
            qDebug() << "是到关键字这里吗——————————————————————————";
            matches = false;
        }

        if (matches) {
            filteredHistory.append(msg); // 添加到过滤后的列表
        }
    }

    // 更新列表显示
    showSearchHistory(filteredHistory);
}

void HistoryDialog::showSearchHistory(const QList<QJsonObject> &history)
{
    //清空记录
    ui->listWidget->clear();

    qDebug() << "是不是为空" << history;

    //显示历史记录
    setHistoryText(history);
}

//重置搜索栏和显示全部历史记录
void HistoryDialog::on_reset_clicked()
{
    ui->userName->clear();
    ui->dateTime->setDate(QDate(2000,1,1));
    ui->keywords->clear();
    showSearchHistory(historyList);
}

