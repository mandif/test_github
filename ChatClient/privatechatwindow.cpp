#include "privatechatwindow.h"
#include <QVBoxLayout>
#include <QDebug>

PrivateChatWindow::PrivateChatWindow(const QString &username, QWidget *parent)
    : QDialog(parent), m_username(username) {
    ui.setupUi(this); // 正确初始化 ui 对象
    setWindowTitle(QString("Chat with %1").arg(username));  // 设置窗口标题

    // 连接信号和槽
    connect(ui.sendButton, &QPushButton::clicked, this, &PrivateChatWindow::sendMessage);
}

void PrivateChatWindow::sendMessage() {
    QString message = ui.messageLineEdit->text();  // 使用 QLineEdit 的对象
    qDebug() << "发送了什么消息：" << message;
    if (!message.isEmpty()) {
        ui.chatTextEdit->append(QString("Me: %1").arg(message));  // 显示发送的消息
        emit sendMessageToUser(m_username, message);  // 发出信号
        ui.messageLineEdit->clear();  // 清空输入框
    }
}

void PrivateChatWindow::receiveMessage(const QString &sender, const QString &message) {
    ui.chatTextEdit->append(QString("%1: %2").arg(sender).arg(message));  // 显示接收到的消息
}
