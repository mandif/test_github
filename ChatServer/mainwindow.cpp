#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "historydialog.h" // 引入头文件
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_chatServer = new ChatServer(this);
    dialog = nullptr;

    connect(m_chatServer,&ChatServer::logMessage,this,&MainWindow::logMessage);
    connect(m_chatServer, &ChatServer::newMessageReceived, this, &MainWindow::updateHistory);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startStopButton_clicked()
{
    if(m_chatServer->isListening()){
        m_chatServer->StopServer();
        ui->startStopButton->setText("启动服务器");
        logMessage("服务器已停止");
    }else{
        if(!m_chatServer->listen(QHostAddress::Any,1967)){
            QMessageBox::critical(this,"错误","无法启动服务器");
            return;
        }
        logMessage("服务器已经启动");
        ui->startStopButton->setText("停止服务器");
    }


}

void MainWindow::logMessage(const QString &msg)
{
    ui->logEditor->appendPlainText(msg);
}

void MainWindow::on_historyButton_clicked()
{
    dialog = new HistoryDialog(this);
    dialog->setHistory(m_chatServer->getChatHistory()); // 设置历史消息
    dialog->exec(); // 弹出对话框
    delete dialog; // 关闭后删除对话框
    dialog = nullptr; // 清空指针
}

void MainWindow::updateHistory() {
    qDebug() << dialog << "是不是空指针" ;
    qDebug() << "测试是不是来到这里，然后崩溃了！！！！";
    if (dialog != nullptr) { // dialog 是 HistoryDialog 的指针
        qDebug() << "再测试是否来到这里崩溃";
        dialog->updateHistory(m_chatServer->theNewChatHistory());
    }
}
