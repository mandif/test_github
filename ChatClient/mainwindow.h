#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListWidgetItem>
#include <QMainWindow>
#include "chatclient.h"
#include "privatechatwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sayButton_clicked();

    void on_loginButton_clicked();

    void on_logoutButton_clicked();

    void connectToServer();

    void messageReceived(const QString &sender,const QString &text);
    void jsonReceived(const QJsonObject &docObj);
    void userJoined(const QString &user);
    void userLeft(const QString &user);
    void userListReceived(const QStringList &list);
    // void contextMenuEvent(QContextMenuEvent *event);
    void muteUser(const QString &userName);
    void kickUser(const QString &userName);

    void contextMenuRequested(const QPoint &pos);
    void handleMuteUser(const QString muteUser);
    bool isSelf(const QString &userName);
    void printAllItems();

    void onUserDoubleClicked(QListWidgetItem *item);
    void showWindow(const QString &username);
private:
    Ui::MainWindow *ui;

    ChatClient *m_chatClient;
    QString chatUserName;
    bool isAdmin;  // 新增变量，用于标记当前用户是否为管理员

    QMap<QString, PrivateChatWindow*> m_chatWindows; // 用于存储用户私聊窗口
};
#endif // MAINWINDOW_H
