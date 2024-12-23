#ifndef PRIVATECHATWINDOW_H
#define PRIVATECHATWINDOW_H

#include <QDialog>
#include "ui_PrivateChatWindow.h" // 确保你的 UI 文件名正确

class PrivateChatWindow : public QDialog {
    Q_OBJECT

public:
    explicit PrivateChatWindow(const QString &username, QWidget *parent = nullptr);

signals:
    void sendMessageToUser(const QString &recipient, const QString &message);

public slots:
    void receiveMessage(const QString &sender, const QString &message);

private slots:
    void sendMessage();

private:
    Ui::PrivateChatWindow ui; // 使用 UI 结构体的声明
    QString m_username;
};

#endif // PRIVATECHATWINDOW_H
