#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHostAddress>
#include <QJsonValue>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    m_chatClient = new ChatClient(this);

    connect(m_chatClient,&ChatClient::connected,this,&MainWindow::connectToServer);
    // connect(m_chatClient,&ChatClient::messageReceived,this,&MainWindow::messageReceived);
    connect(m_chatClient,&ChatClient::jsonReceived,this,&MainWindow::jsonReceived);
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
}

void MainWindow::connectToServer()
{
    ui->stackedWidget->setCurrentWidget(ui->chatPage);
    m_chatClient->sendMessage(ui->userNameEdit->text(),"login");
}

void MainWindow::messageReceived(const QString &sender,const QString &text)
{
    ui->roomTextEdit->append(QString("%1 : %2").arg(sender).arg(text));
}

void MainWindow::jsonReceived(const QJsonObject &docObj)
{
    const QJsonValue typeVal = docObj.value("type");
    if(typeVal.isNull() || !typeVal.isString())
        return;
    if(typeVal.toString().compare("message",Qt::CaseInsensitive) == 0){
        const QJsonValue textVal = docObj.value("text");
        const QJsonValue senderVal = docObj.value("sender");
        if(textVal.isNull() || !textVal.isString())
            return;

        if(senderVal.isNull() || !senderVal.isString())
            return;

        messageReceived(senderVal.toString(),textVal.toString());

    }else if(typeVal.toString().compare("newuser",Qt::CaseInsensitive) == 0){
        const QJsonValue usernameVal = docObj.value("username");
        if(usernameVal.isNull() || !usernameVal.isString())
            return;

        userJoined(usernameVal.toString());
    }
}

void MainWindow::userJoined(const QString &user)
{
    ui->userListWidget->addItem(user);
}

