#include "serverworker.h"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

ServerWorker::ServerWorker(QObject *parent)
    : QObject{parent}
{
    m_serverSocket = new QTcpSocket(this);
    connect(m_serverSocket,&QTcpSocket::readyRead,this, &ServerWorker::onReadyRead);
    connect(m_serverSocket,&QTcpSocket::disconnected,this, &ServerWorker::disconnectedFromClient);
}

bool ServerWorker::setSocketDescriptor(qintptr socketDescriptor)
{
    return m_serverSocket->setSocketDescriptor(socketDescriptor);
}

QString ServerWorker::userName()
{
    return m_userName;
}

void ServerWorker::setUserName(QString user)
{
    m_userName = user;
}

void ServerWorker::onReadyRead()
{
    QByteArray jsonData;  // 存储接收到的原始 JSON 数据
    QDataStream socketStream(m_serverSocket);  // 使用 QDataStream 从套接字读取数据
    socketStream.setVersion(QDataStream::Qt_6_7);  // 设置数据流版本（确保数据格式兼容性）

    for(;;) {
        socketStream.startTransaction();  // 开始新的数据传输事务
        socketStream >> jsonData;  // 从套接字读取数据到 jsonData
        if(socketStream.commitTransaction()) {  // 尝试提交事务
            // 数据成功读取
            QJsonParseError parseError;  // 用于捕获解析错误
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);  // 尝试将字节数组解析为 JSON 文档

            if (parseError.error == QJsonParseError::NoError) {  // 检查解析是否成功
                if (jsonDoc.isObject()) {  // 确认 JSON 文档是一个对象
                    qDebug() << QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact);
                    emit logMessage(QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact));  // 记录收到的 JSON 消息
                    qDebug() << jsonDoc.object();
                    emit jsonReceived(this, jsonDoc.object());  // 发射信号，传递 JSON 对象以供进一步处理
                }
            }

        } else {
            break;  // 如果事务提交失败，退出循环
        }
    }
}

void ServerWorker::sendMessage(const QString &text, const QString &type)
{
    if(m_serverSocket->state() != QAbstractSocket::ConnectedState)
        return;

    if(!text.isEmpty()){
        QDataStream serverStream(m_serverSocket);
        serverStream.setVersion(QDataStream::Qt_6_7);

        QJsonObject message;
        message["type"] = type;
        message["text"] = text;

        serverStream << QJsonDocument(message).toJson();
    }
}

void ServerWorker::sendJson(const QJsonObject &json)
{
    // 将传入的 QJsonObject 转换为紧凑格式的字节数组（QByteArray）
    const QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);

    // 记录发送的日志，包括发送者的用户名和要发送的 JSON 数据
    emit logMessage(QLatin1String("Sending to ") + userName() +
                    QLatin1String(" - ") + QString::fromUtf8(jsonData));

    // 创建一个 QDataStream，以便将数据写入到 m_serverSocket
    QDataStream socketStream(m_serverSocket);

    // 设置数据流版本为 Qt 6.7，确保数据格式的兼容性
    socketStream.setVersion(QDataStream::Qt_6_7);

    // 将 jsonData 写入到数据流中，以通过套接字发送数据
    socketStream << jsonData;
}

