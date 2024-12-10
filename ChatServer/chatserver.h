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

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    QVector<ServerWorker *>m_clients;

    void boradcast(const QJsonObject &message,ServerWorker *exclude);

signals:
    void logMessage(const QString &msg);

public slots:
    void StopServer();
    void jsonReceived(ServerWorker *sender,const QJsonObject &docObj);

};

#endif // CHATSERVER_H