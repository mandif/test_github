#include "chatserver.h"
#include "serverworker.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

ChatServer::ChatServer(QObject *parent):
    QTcpServer(parent)
{}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    ServerWorker *worker = new ServerWorker(this);
    if(!worker->setSocketDescriptor(socketDescriptor)){
        worker->deleteLater();
        return;
    }
    connect(worker,&ServerWorker::logMessage,this,&ChatServer::logMessage);
    connect(worker,&ServerWorker::jsonReceived,this,&ChatServer::jsonReceived);
    connect(worker,&ServerWorker::disconnectedFromClient,this,
            std::bind(&ChatServer::userDisconnected,this,worker));
    m_clients.append(worker);
    emit logMessage("新的用户连接上了");
}

void ChatServer::boradcast(const QJsonObject &message, ServerWorker *exclude)
{
    for(ServerWorker *worker: m_clients){
        worker->sendJson(message);
    }
}

void ChatServer::StopServer()
{
    close();
}

void ChatServer::jsonReceived(ServerWorker *sender, const QJsonObject &docObj)
{
    // 从 JSON 对象中获取 "type" 字段的值
    const QJsonValue typeVal = docObj.value("type");

    // 检查 "type" 值是否为空或非字符串类型，如果是，则返回
    if (typeVal.isNull() || !typeVal.isString())
        return;

    // 检查 "type" 是否为 "message"（不区分大小写）
    if (typeVal.toString().compare("message", Qt::CaseInsensitive) == 0) {

        // 从 JSON 对象中获取 "text" 字段的值
        const QJsonValue textVal = docObj.value("text");

        // 检查 "text" 是否为空或非字符串类型，如果是，则返回
        if (textVal.isNull() || !textVal.isString())
            return;

        // 获取并修剪文本内容（去除首尾空白）
        const QString text = textVal.toString().trimmed();

        // 如果文本为空，返回
        if (text.isEmpty())
            return;

        // 创建一个新的 JSON 对象，用于构建要发送的消息
        QJsonObject message;
        message["type"] = "message";        // 设置消息类型为 "message"
        message["text"] = text;              // 设置消息内容
        message["sender"] = sender->userName(); // 设置消息发送者的用户名

        // 将消息广播给所有连接的客户端，除了发送者
        boradcast(message, sender);
    }
    // 如果消息类型是 "login"（不区分大小写）
    else if (typeVal.toString().compare("login", Qt::CaseInsensitive) == 0) {

        // 从 JSON 对象中获取用户的用户名
        const QJsonValue usernameVal = docObj.value("text");

        // 检查用户名是否为空或非字符串类型，如果是，则返回
        if (usernameVal.isNull() || !usernameVal.isString())
            return;

        // 设置发送者的用户名
        sender->setUserName(usernameVal.toString());

        // 创建一个新的 JSON 对象，通知所有其他用户有新用户连接
        QJsonObject connectedMessage;
        connectedMessage["type"] = "newuser";            // 设置消息类型为 "newuser"
        connectedMessage["username"] = usernameVal.toString(); // 设置新用户的用户名

        // 将用户连接的消息广播给所有连接的客户端
        boradcast(connectedMessage, sender);

        // 发送用户列表给新登录的用户
        QJsonObject userListMessage;
        userListMessage["type"] = "userlist"; // 设置消息类型为 "userlist"
        QJsonArray userlist;                   // 创建一个 JSON 数组用于存储用户列表

        // 遍历所有连接的 ServerWorker 实例
        for (ServerWorker *worker : m_clients) {
            // 如果当前 worker 是新用户，用户名后加上 "*" 标识
            if (worker == sender)
                userlist.append(worker->userName() + "*");
            else
                userlist.append(worker->userName()); // 添加其他用户的用户名
        }

        // 将用户列表添加到 JSON 消息中
        userListMessage["userlist"] = userlist;

        // 将用户列表消息发送给新登录的用户
        sender->sendJson(userListMessage);
    }
    else if(typeVal.toString().compare("admin", Qt::CaseInsensitive) == 0){
        // 从 JSON 对象中获取用户的用户名
        const QJsonValue adminnameVal = docObj.value("text");

        // 检查用户名是否为空或非字符串类型，如果是，则返回
        if (adminnameVal.isNull() || !adminnameVal.isString())
            return;

        // 设置发送者的用户名
        sender->setUserName(adminnameVal.toString());

        // 创建一个新的 JSON 对象，通知所有其他用户有新用户连接
        QJsonObject connectedMessage;
        connectedMessage["type"] = "admin";            // 设置消息类型为 "admin"
        connectedMessage["adminname"] = adminnameVal.toString() + QString::number(adminCnt); // 设置新用户的用户名


        // 将用户连接的消息广播给所有连接的客户端
        boradcast(connectedMessage, sender);

        // 发送用户列表给新登录的用户
        QJsonObject userListMessage;
        userListMessage["type"] = "userlist"; // 设置消息类型为 "userlist"
        QJsonArray userlist;                   // 创建一个 JSON 数组用于存储用户列表

        // 遍历所有连接的 ServerWorker 实例
        for (ServerWorker *worker : m_clients) {
            // 如果当前 worker 是新用户，用户名后加上 "*" 标识
            if (worker == sender)
                userlist.append(worker->userName()  + QString::number(adminCnt) + "*");
            else
                userlist.append(worker->userName()); // 添加其他用户的用户名
        }

        // 将用户列表添加到 JSON 消息中
        userListMessage["userlist"] = userlist;
        adminCnt++;  //管理员数量增加

        // 将用户列表消息发送给新登录的用户
        sender->sendJson(userListMessage);
    }else if(typeVal.toString().compare("mute", Qt::CaseInsensitive) == 0){
        const QJsonValue muteUserVal = docObj.value("text");
        if (muteUserVal.isNull() || !muteUserVal.isString()) return;

        QString mutedUserName = muteUserVal.toString();
        m_mutedUsers.insert(mutedUserName); // 将用户添加到禁言列表

        // 通知所有用户某个用户被禁言
        QJsonObject muteMessage;
        muteMessage["type"] = "mute"; // 通知类型为禁言
        muteMessage["mutedUserName"] = mutedUserName; // 被禁言的用户名

        boradcast(muteMessage, nullptr); // 广播禁言消息
        qDebug() << mutedUserName << "被禁言了";

        // handleMuteUser(mutedUserName);


    }else if(typeVal.toString().compare("kick", Qt::CaseInsensitive) == 0){
        const QJsonValue kickUserVal = docObj.value("text");
        if (kickUserVal.isNull() || !kickUserVal.isString())
            return;

        QString kickedUser = kickUserVal.toString();
        qDebug() << "被踢出的用户是：" << kickedUser;

        // 处理踢出用户的逻辑
        for (ServerWorker *worker : m_clients) {
            if (worker->userName() == kickedUser) {
                worker->disconnect(); // 断开连接
                break; // 找到用户后退出循环
            }
        }

        // 广播踢出消息
        QJsonObject kickMessage;
        kickMessage["type"] = "kick";
        kickMessage["username"] = kickedUser;
        boradcast(kickMessage, nullptr); // 广播踢出事件
    }
}


void ChatServer::userDisconnected(ServerWorker *sender)
{
    m_clients.removeAll(sender);
    const QString userName = sender->userName();
    if(!userName.isEmpty()){
        QJsonObject disconnectedMessage;
        disconnectedMessage["type"] = "userdisconnected";
        disconnectedMessage["username"] = userName;
        boradcast(disconnectedMessage,nullptr);
        emit logMessage(userName + "disconnected");
    }
    sender->deleteLater();
}

void ChatServer::handleMuteUser(const QString mutedUserName){
    handleMuteUser(mutedUserName);
}

