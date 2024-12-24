#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include "serverworker.h"

class ChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr);
    const QList<QJsonObject>& getChatHistory() const; // 获取聊天记录
    const QJsonObject& theNewChatHistory() const; // 获取最新聊天记录
    void logChatMessage(const QJsonObject &msg); // 用于记录聊天消息

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    QVector<ServerWorker *>m_clients;
    QSet<QString> m_mutedUsers; // 存储禁言用户

    void boradcast(const QJsonObject &message,ServerWorker *exclude);

signals:
    void logMessage(const QString &msg);
    void newMessageReceived();

public slots:
    void StopServer();
    void jsonReceived(ServerWorker *sender,const QJsonObject &docObj);
    void userDisconnected(ServerWorker *sender);
    void handleMuteUser(const QString mutedUserName);

private:
    int adminCnt = 1;
    QList<QJsonObject> m_chatHistory; // 聊天记录
};

#endif // CHATSERVER_H
