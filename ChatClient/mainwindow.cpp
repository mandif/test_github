#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHostAddress>
#include <QJsonValue>
#include <QJsonObject>
#include <QMenu>
#include <QContextMenuEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    ui->sayButton->setEnabled(true);
    m_chatClient = new ChatClient(this);

    connect(m_chatClient,&ChatClient::connected,this,&MainWindow::connectToServer);
    // connect(m_chatClient,&ChatClient::messageReceived,this,&MainWindow::messageReceived);
    connect(m_chatClient,&ChatClient::jsonReceived,this,&MainWindow::jsonReceived);
    // connect(m_chatClient,&ChatServer::handleMuteUser,this,&MainWindow::jsonReceived);

    ui->userListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->userListWidget, &QListWidget::customContextMenuRequested,
            this, &MainWindow::contextMenuRequested);

    connect(ui->userListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onUserDoubleClicked);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loginButton_clicked()
{
    //ui->stackedWidget->setCurrentWidget(ui->chatPage);
    m_chatClient->connectToServer(QHostAddress(ui->serverEdit->text()),1967);
}

void MainWindow::on_sayButton_clicked()
{
    if(!ui->sayLineEdit->text().isEmpty()){
        m_chatClient->sendMessage(ui->sayLineEdit->text());
        ui->sayLineEdit->clear();
    }
}

void MainWindow::on_logoutButton_clicked()
{
    m_chatClient->disconnectFromHost();
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    ui->sayButton->setEnabled(true);

    for(auto aItem : ui->userListWidget->findItems(ui->userNameEdit->text(),Qt::MatchExactly)){
        qDebug("remove");
        ui->userListWidget->removeItemWidget(aItem);
        delete aItem;
    }
}

void MainWindow::connectToServer() //连接服务器
{
    ui->stackedWidget->setCurrentWidget(ui->chatPage);
    if(ui->userNameEdit->text().compare("管理员") == 0){  //判断是否是管理员登录
        m_chatClient->sendMessage(ui->userNameEdit->text(),"admin");
        isAdmin = true;  // 如果是管理员，设置为true
    }
    else{
        m_chatClient->setUserName(ui->userNameEdit->text());
        m_chatClient->sendMessage(ui->userNameEdit->text(),"login");
        isAdmin = false;  // 如果不是管理员，设置为false
    }

}

void MainWindow::messageReceived(const QString &sender,const QString &text)
{
    ui->roomTextEdit->append(QString("%1 : %2").arg(sender).arg(text));
}

void MainWindow::jsonReceived(const QJsonObject &docObj)
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
        // 从 JSON 对象中获取 "sender" 字段的值
        const QJsonValue senderVal = docObj.value("sender");

        // 检查消息文本是否为空或非字符串类型，如果是，则返回
        if (textVal.isNull() || !textVal.isString())
            return;

        // 检查发送者是否为空或非字符串类型，如果是，则返回
        if (senderVal.isNull() || !senderVal.isString())
            return;

        // 调用 messageReceived 函数处理接收到的消息和发送者
        messageReceived(senderVal.toString(), textVal.toString());

        // 检查 "type" 是否为 "newuser"（不区分大小写）
    } else if (typeVal.toString().compare("newuser", Qt::CaseInsensitive) == 0) {
        // 从 JSON 对象中获取 "username" 字段的值
        const QJsonValue usernameVal = docObj.value("username");

        // 检查用户名是否为空或非字符串类型，如果是，则返回
        if (usernameVal.isNull() || !usernameVal.isString())
            return;

        // 调用 userJoined 函数处理新用户加入
        userJoined(usernameVal.toString());

    }else if(typeVal.toString().compare("admin", Qt::CaseInsensitive) == 0){ // 检查 "type" 是否为 "admin"（不区分大小写）
        // 从 JSON 对象中获取 "adminname" 字段的值
        const QJsonValue adminnameVal = docObj.value("adminname");

        // 检查用户名是否为空或非字符串类型，如果是，则返回
        if (adminnameVal.isNull() || !adminnameVal.isString())
            return;

        // 调用 userJoined 函数处理新用户加入
        // userJoined(adminnameVal.toString());
    }else if (typeVal.toString().compare("userdisconnected", Qt::CaseInsensitive) == 0) { // 检查 "type" 是否为 "userdisconnected"（不区分大小写）
        // 从 JSON 对象中获取 "username" 字段的值
        const QJsonValue usernameVal = docObj.value("username");

        // 检查用户名是否为空或非字符串类型，如果是，则返回
        if (usernameVal.isNull() || !usernameVal.isString())
            return;

        // 调用 userLeft 函数处理用户离开
        userLeft(usernameVal.toString());

        // 检查 "type" 是否为 "userlist"（不区分大小写）
    } else if (typeVal.toString().compare("userlist", Qt::CaseInsensitive) == 0) {
        // 从 JSON 对象中获取 "userlist" 字段的值
        const QJsonValue userlistVal = docObj.value("userlist");

        // 检查用户列表是否为空或非数组类型，如果是，则返回
        if (userlistVal.isNull() || !userlistVal.isArray())
            return;

        // 调试输出用户列表，将 JSON 数组转换为字符串列表并打印
        qDebug() << userlistVal.toVariant().toStringList();

        // 调用 userListReceived 函数处理接收到的用户列表
        userListReceived(userlistVal.toVariant().toStringList());
    }else if (typeVal.toString().compare("mute", Qt::CaseInsensitive) == 0){
        qDebug() << "禁言是不是到这一步了！！！！！！！！";

        // 从 JSON 对象中获取 "mutedUserName" 字段的值
        const QJsonValue muteVal = docObj.value("mutedUserName");
        QString muteUser = muteVal.toString();

        qDebug() << "禁言人的用户名是：" << muteVal.toString();

        // 检查用户列表是否为空或非数组类型，如果是，则返回
        // if (muteVal.isNull() || !muteVal.isArray())
        //     return;

        // 调试输出用户列表，将 JSON 数组转换为字符串列表并打印
        qDebug() << muteVal.toVariant().toStringList();

        qDebug() << "有没有走到这一步！！！！！！！！" << muteVal.toString();

        // 将这个用户禁言
        handleMuteUser(muteUser);
    }else if (typeVal.toString().compare("kick", Qt::CaseInsensitive) == 0){
        // 从 JSON 对象中获取 "username" 字段的值
        const QJsonValue kicknameVal = docObj.value("kickUserName");

        // 检查用户名是否为空或非字符串类型，如果是，则返回
        if (kicknameVal.isNull() || !kicknameVal.isString())
            return;

        qDebug() << "该线程用户是：" << m_chatClient->userName();
        qDebug() << "被踢出的用户是：" << kicknameVal.toString();
        qDebug() << "踢出操作有没有到这一步！！！！！！！！！！！";

        // 调用 userLeft 函数处理用户离开
        userLeft(kicknameVal.toString());

        //进行强制下线操作
        if(isSelf(kicknameVal.toString())){
            qDebug() << "确认是该用户了！！！！！！";
            on_logoutButton_clicked();
        }

        printAllItems();
    }else if (typeVal.toString().compare("ReLink", Qt::CaseInsensitive) == 0){
        // 从 JSON 对象中获取 "username" 字段的值
        const QJsonValue LinkedUserNameVal = docObj.value("LinkedUserName");
        const QJsonValue ReLinkUserNameVal = docObj.value("ReLinkUserName");

        // 检查用户名是否为空或非字符串类型，如果是，则返回
        if (LinkedUserNameVal.isNull() || !LinkedUserNameVal.isString())
            return;
        if (ReLinkUserNameVal.isNull() || !ReLinkUserNameVal.isString())
            return;

        QString LinkedUserName = LinkedUserNameVal.toString();
        QString ReLinkUserName = ReLinkUserNameVal.toString();

        if(isSelf(LinkedUserName)){
            qDebug() << "用户" << LinkedUserName << "接收到连接了";
            showWindow(ReLinkUserName);
        }

    }else if (typeVal.toString().compare("closeWindow", Qt::CaseInsensitive) == 0){
        // 从 JSON 对象中获取 "username" 字段的值
        const QJsonValue LinkedUserNameVal = docObj.value("LinkedUserName");
        const QJsonValue ReLinkUserNameVal = docObj.value("ReLinkUserName");

        // 检查用户名是否为空或非字符串类型，如果是，则返回
        if (LinkedUserNameVal.isNull() || !LinkedUserNameVal.isString())
            return;
        if (ReLinkUserNameVal.isNull() || !ReLinkUserNameVal.isString())
            return;

        QString LinkedUserName = LinkedUserNameVal.toString();
        QString ReLinkUserName = ReLinkUserNameVal.toString();

        if(isSelf(LinkedUserName)){
            qDebug() << "用户" << LinkedUserName << "接收到关闭连接了";
            closeWindow(ReLinkUserName);
        }
    }
}

void MainWindow::handleMuteUser(const QString muteUser){
    QStringList itemList; // 用于存储所有的项
    // 遍历 QListWidget 的所有项目
    for (int i = 0; i < ui->userListWidget->count(); ++i) {
        // 获取 QListWidgetItem
        QListWidgetItem *item = ui->userListWidget->item(i);
        if (item->text() == muteUser + "*") {
            ui->sayButton->setEnabled(false);
            break;
        }
    }
}

void MainWindow::userJoined(const QString &user)
{
    ui->userListWidget->addItem(user);
}

void MainWindow::userLeft(const QString &user)
{
    for(auto aItem : ui->userListWidget->findItems(user,Qt::MatchExactly)){
        qDebug("remove");
        ui->userListWidget->removeItemWidget(aItem);
        delete aItem;
    }
}

void MainWindow::userListReceived(const QStringList &list)
{
    ui->userListWidget->clear();
    if(list.contains("管理员")){
        // 使用范围 for 循环遍历字符串列表
        for (const QString &fruit : list) {
            if(fruit.compare("管理员") != 0)
                ui->userListWidget->addItem(fruit);
        }
    }
    else ui->userListWidget->addItems(list);
}

void MainWindow::contextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *item = ui->userListWidget->itemAt(pos);
    if (item) {
        QString userName = item->text();
        qDebug() << "用户名是：" + userName;

        // 确保只有管理员可以操作
        if (isAdmin) {
            // 不允许管理员对自己或其他管理员进行操作
            if (userName != ui->userNameEdit->text() && !userName.contains("管理员")) {
                QMenu contextMenu(this);
                QAction *muteAction = contextMenu.addAction("禁言");
                QAction *kickAction = contextMenu.addAction("踢除");

                QAction *selectedAction = contextMenu.exec(ui->userListWidget->viewport()->mapToGlobal(pos));

                if (selectedAction == muteAction) {
                    muteUser(userName);
                } else if (selectedAction == kickAction) {
                    kickUser(userName);
                }
            }
        }
    }
}



void MainWindow::muteUser(const QString &userName) {
    // 逻辑以禁言用户
    qDebug() << userName << "has been muted.";

    // 假设您需要通过格式化并发送到服务器
    m_chatClient->sendMessage(userName, "mute");
}

void MainWindow::kickUser(const QString &userName) {
    // 逻辑以踢除用户
    qDebug() << userName << "has been kicked out.";

    // 假设您需要通过格式化并发送到服务器
    m_chatClient->sendMessage(userName, "kick");

    //对被踢出的用户进行下线，并且在用户列表中将该用户删除

    // ui->stackedWidget->setCurrentWidget(ui->loginPage);
    // ui->sayButton->setEnabled(true);
}

bool MainWindow::isSelf(const QString &userName){
    qDebug() << "有没有对用户所在线程进行判断";
    if(m_chatClient->userName() == userName){
        qDebug() << "这不是正确的吗！！！！！！！！";
        return true;
    }
    return false;
}

void MainWindow::printAllItems() {
    for (int i = 0; i < ui->userListWidget->count(); ++i) {
        // 获取 QListWidgetItem
        QListWidgetItem *item = ui->userListWidget->item(i);
        if (item) {
            // 打印每个项目的文本
            qDebug() << item->text();
        }
    }
}

//双击进行私聊
void MainWindow::onUserDoubleClicked(QListWidgetItem *item) {
    if (item) {
        QString username = item->text();

        qDebug() << "被双击的用户是：" << username;

        showWindow(username);

        //请求被双击用户的连接
        m_chatClient->sendPrivateMessage(m_chatClient->userName(),username,"ReLink");
    }
}

void MainWindow::showWindow(const QString &username){
    // 检查是否已经有这个用户的聊天窗口打开
    if (m_chatWindows.contains(username)) {
        // 如果窗口已经打开，则直接激活它
        m_chatWindows[username]->activateWindow();
        return;
    }

    // 创建新聊天窗口
    chatWindow = new PrivateChatWindow(username, this);
    m_chatWindows[username] = chatWindow; // 存储窗口实例

    // 连接发送消息的信号
    connect(chatWindow, &PrivateChatWindow::sendMessageToUser,
            [=](const QString &recipient, const QString &message) {
                qDebug() << "类型为：" << recipient;   //这里的类型recipient是私聊消息接收方的用户名
                qDebug() << "私聊发送的消息是：" << message;
                qDebug() << "有哪些私聊用户：" << m_chatWindows;
                m_chatClient->sendBothPrivateMessage(m_chatClient->userName(), recipient, message, "recipient");
            });

    // 连接接收消息的信号
    connect(m_chatClient, &ChatClient::privateMessageReceived,
            chatWindow, &PrivateChatWindow::receiveMessage);

    // 连接 finished 信号来处理窗口关闭
    connect(chatWindow, &QDialog::finished, [=]() {
        qDebug() << "现在还剩下什么用户：" << m_chatWindows;
        m_chatWindows.remove(username);  // 从 map 中移除

        //调用发送关闭窗口函数
        m_chatClient->sendCloseMessage(m_chatClient->userName(), username, "closeWindow");


    });

    chatWindow->show();  // 显示被双击用户的聊天窗口
}

//关闭私聊窗口
void MainWindow::closeWindow(const QString &username)
{
    if(m_chatWindows.contains(username)){
        m_chatWindows[username]->close();
        m_chatWindows.remove(username);  // 从 map 中移除
    }
}
