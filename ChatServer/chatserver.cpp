#include "chatserver.h"
#include "serverworker.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QThreadPool>
#include <QRunnable>

ChatServer::ChatServer(QObject *parent):
    QTcpServer(parent)
{}

const QList<QJsonObject> &ChatServer::getChatHistory() const
{
    return m_chatHistory; // 返回聊天记录
}

const QJsonObject &ChatServer::theNewChatHistory() const
{
    return m_chatHistory.last();
}

void ChatServer::logChatMessage(const QJsonObject &msg)
{

    //获取消息类型，如果是message，就将接收者设置为all，如果是recipient，就按照原来的接收者
    const QJsonValue msgTypeVal = msg.value("type");

    const QString msgType = msgTypeVal.toString();

    //获取系统当前时间
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 将当前时间转换为字符串格式
    QString timeString = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");

    QJsonObject chatMessage;

    if(msgType == "message"){
        const QJsonValue senderVal = msg.value("sender");
        const QJsonValue textVal = msg.value("text");
        const QString sender = senderVal.toString();
        const QString text = textVal.toString();

        chatMessage["type"] = "message";
        chatMessage["sender"] = sender;
        chatMessage["reciver"] = "all";
        chatMessage["text"] = text;
        chatMessage["time"] = timeString;
    }else if(msgType == "recipient"){
        const QJsonValue ReLinkUserVal = msg.value("ReLinkUserName");
        const QJsonValue LinkedUserVal = msg.value("LinkedUserName");
        const QJsonValue messageVal = msg.value("message");

        const QString sender = ReLinkUserVal.toString();
        const QString reciver = LinkedUserVal.toString();
        const QString text = messageVal.toString();

        chatMessage["type"] = "recipient";
        chatMessage["sender"] = sender;
        chatMessage["reciver"] = reciver;
        chatMessage["text"] = text;
        chatMessage["time"] = timeString;
    }

    qDebug() << "记录历史消息：" << chatMessage;

    //保存消息到历史记录
    m_chatHistory.append(chatMessage);

    //如果历史记录框存在，更新
    emit newMessageReceived();
}

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

    // 创建一个新的线程
    QThread *thread = new QThread();
    worker->moveToThread(thread);  // 将 worker 移到这个新线程

    connect(thread, &QThread::started, worker, &ServerWorker::run);  // 当线程启动时，调用 worker 的 run 方法
    connect(worker, &ServerWorker::disconnectedFromClient, thread, &QThread::quit);  // 当客户端断开连接时，退出线程
    connect(worker, &ServerWorker::disconnectedFromClient, worker, &ServerWorker::deleteLater);  // 删除 worker
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);  // 删除线程

    thread->start();  // 启动线程
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

    //测试获取到的是什么类型的数据
    qDebug() << "测试获取到的是什么类型的数据:" << typeVal.toString();

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

        //将消息保存到历史记录中，保持格式一致
        logChatMessage(message);

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
        // for (ServerWorker *worker : m_clients) {
        //     if (worker->userName() == kickedUser) {
        //         worker->disconnect(); // 断开连接
        //         break; // 找到用户后退出循环
        //     }
        // }

        // 广播踢出消息
        QJsonObject kickMessage;
        kickMessage["type"] = "kick";
        kickMessage["kickUserName"] = kickedUser;
        boradcast(kickMessage, nullptr); // 广播踢出事件
        emit logMessage(kickedUser + "disconnected");
    }else if(typeVal.toString().compare("ReLink", Qt::CaseInsensitive) == 0){
        const QJsonValue ReLinkUserVal = docObj.value("ReLink");
        const QJsonValue LinkedUserVal = docObj.value("Linked");
        if (ReLinkUserVal.isNull() || !ReLinkUserVal.isString()) return;
        if (LinkedUserVal.isNull() || !LinkedUserVal.isString()) return;

        QString LinkedUserName = LinkedUserVal.toString();
        QString ReLinkUserName = ReLinkUserVal.toString();
        qDebug() << "用户" << ReLinkUserName << "发起连接";
        qDebug() << "用户" << LinkedUserName << "被请求连接";

        // 通知所有用户某个用户被请求连接
        QJsonObject linkMessage;
        linkMessage["type"] = "ReLink"; // 通知类型为请求连接
        linkMessage["LinkedUserName"] = LinkedUserName; // 被请求连接的用户名
        linkMessage["ReLinkUserName"] = ReLinkUserName; // 请求连接的用户名

        boradcast(linkMessage, nullptr); // 广播请求连接消息

    }else if(typeVal.toString().compare("recipient", Qt::CaseInsensitive) == 0){

        //测试私聊消息有没有到这
        qDebug() << "测试私聊消息有没有到这";

        const QJsonValue ReLinkUserVal = docObj.value("ReLink");
        const QJsonValue LinkedUserVal = docObj.value("Linked");
        const QJsonValue messageVal = docObj.value("message");
        if (ReLinkUserVal.isNull() || !ReLinkUserVal.isString()) return;
        if (LinkedUserVal.isNull() || !LinkedUserVal.isString()) return;


        QString LinkedUserName = LinkedUserVal.toString();
        QString ReLinkUserName = ReLinkUserVal.toString();
        QString message = messageVal.toString();
        qDebug() << "用户" << ReLinkUserName << "发起连接";
        qDebug() << "用户" << LinkedUserName << "被请求连接";
        qDebug() << "发送了私聊消息：" << message;

        // 通知所有用户某个用户被请求连接
        QJsonObject privateMessage;
        privateMessage["type"] = "recipient"; // 通知类型为请求连接
        privateMessage["LinkedUserName"] = LinkedUserName; // 被请求连接的用户名
        privateMessage["ReLinkUserName"] = ReLinkUserName; // 请求连接的用户名
        privateMessage["message"] = message; // 私聊消息

        //将消息保存到历史记录中，保持格式一致
        logChatMessage(privateMessage);



        boradcast(privateMessage, nullptr); // 广播请求连接消息
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

