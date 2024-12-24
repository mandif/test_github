#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QObject>
#include <QTcpServer>
#include "serverworker.h"

class ChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr);
    const QList<QString>& getChatHistory() const; // 获取聊天记录
    void logChatMessage(const QString &msg); // 用于记录聊天消息

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    QVector<ServerWorker *>m_clients;
    QSet<QString> m_mutedUsers; // 存储禁言用户

    void boradcast(const QJsonObject &message,ServerWorker *exclude);

signals:
    void logMessage(const QString &msg);

public slots:
    void StopServer();
    void jsonReceived(ServerWorker *sender,const QJsonObject &docObj);
    void userDisconnected(ServerWorker *sender);
    void handleMuteUser(const QString mutedUserName);

private:
    int adminCnt = 1;
    QList<QString> m_chatHistory; // 聊天记录
};

#endif // CHATSERVER_H
