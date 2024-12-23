#include "chatclient.h"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

ChatClient::ChatClient(QObject *parent)
    : QObject{parent}
{
    m_clientSocket = new QTcpSocket(this);
    connect(m_clientSocket,&QTcpSocket::connected,this, &ChatClient::connected);
    connect(m_clientSocket,&QTcpSocket::readyRead,this, &ChatClient::onReadyRead);
}

void ChatClient::setUserName(const QString &name)
{
    m_userName = name;
}

QString ChatClient::userName()
{
    return m_userName;
}

void ChatClient::onReadyRead()
{
    QByteArray jsonData;
    QDataStream socketStream(m_clientSocket);
    socketStream.setVersion(QDataStream::Qt_6_7);

    for (;;) {
        socketStream.startTransaction();
        socketStream >> jsonData;
        if (socketStream.commitTransaction()) {
            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isObject()) {
                    const QJsonObject jsonObj = jsonDoc.object();
                    const QString messageType = jsonObj.value("type").toString();
                    qDebug() << "私聊发送消息类型：！！！！！" << messageType;

                    if (messageType.compare("recipient", Qt::CaseInsensitive) == 0) {
                        const QString sender = jsonObj.value("ReLinkUserName").toString(); // 获取发送者
                        const QString reciver = jsonObj.value("LinkedUserName").toString(); // 获取接收者
                        const QString message = jsonObj.value("message").toString(); // 获取消息内容
                        qDebug() << "私聊消息接收者是：" << reciver;
                        emit privateMessageReceived(sender, reciver, message); // 发出私聊消息信号
                    } else {
                        emit jsonReceived(jsonDoc.object()); // 处理其他类型的消息
                    }
                }
            }
        } else {
            break;
        }
    }
}

void ChatClient::sendMessage(const QString &text, const QString &type)
{
    if(m_clientSocket->state() != QAbstractSocket::ConnectedState)
        return;

    if(!text.isEmpty()){
        QDataStream serverStream(m_clientSocket);
        serverStream.setVersion(QDataStream::Qt_6_7);

        QJsonObject message;
        message["type"] = type;
        message["text"] = text;
        serverStream << QJsonDocument(message).toJson();
    }
}

//发送私聊消息
void ChatClient::sendPrivateMessage(const QString &text, const QString &text2, const QString &type)
{
    if(m_clientSocket->state() != QAbstractSocket::ConnectedState)
        return;

    if(!text.isEmpty() && !text2.isEmpty()){
        QDataStream serverStream(m_clientSocket);
        serverStream.setVersion(QDataStream::Qt_6_7);

        QJsonObject message;
        message["type"] = type;
        message["ReLink"] = text;
        message["Linked"] = text2;
        serverStream << QJsonDocument(message).toJson();
    }
}

void ChatClient::sendBothPrivateMessage(const QString &text, const QString &text2, const QString &message, const QString &type)
{
    if(m_clientSocket->state() != QAbstractSocket::ConnectedState)
        return;

    if(!text.isEmpty() && !text2.isEmpty()){
        QDataStream serverStream(m_clientSocket);
        serverStream.setVersion(QDataStream::Qt_6_7);

        QJsonObject sendMessage;
        sendMessage["type"] = type;
        sendMessage["ReLink"] = text;
        sendMessage["Linked"] = text2;
        sendMessage["message"] = message;

        //测试这里的消息有没有
        qDebug() << "测试这里的消息有没有" << message;
        serverStream << QJsonDocument(sendMessage).toJson();
    }
}

void ChatClient::connectToServer(const QHostAddress &address, quint16 port)
{
    m_clientSocket->connectToHost(address,port);
}

void ChatClient::disconnectFromHost()
{
    m_clientSocket->disconnectFromHost();
}
