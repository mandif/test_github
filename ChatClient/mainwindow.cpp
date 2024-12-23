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

// void MainWindow::contextMenuRequested(const QPoint &pos)
// {
//     QListWidgetItem *item = ui->userListWidget->itemAt(pos);
//     if (item) {
//         if(isAdmin == true){
//             QMenu contextMenu(this);
//             QAction *muteAction = contextMenu.addAction("禁言");
//             QAction *kickAction = contextMenu.addAction("踢除");

//             QAction *selectedAction = contextMenu.exec(ui->userListWidget->viewport()->mapToGlobal(pos));

//             if (selectedAction == muteAction) {
//                 QString userName = item->text();
//                 muteUser(userName);
//             } else if (selectedAction == kickAction) {
//                 QString userName = item->text();
//                 kickUser(userName);
//             }
//         }
//     }
// }

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
}
