#ifndef SERVERWORKER_H
#define SERVERWORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QRunnable>

class ServerWorker : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ServerWorker(QObject *parent = nullptr);
    virtual bool setSocketDescriptor(qintptr socketDescriptor);

    QString userName();
    void setUserName(QString user);

    // 重写 run() 方法，以便在线程池中执行
    void run() override;

signals:
    void logMessage(const QString &msg);
    void jsonReceived(ServerWorker *sender,const QJsonObject &docObj);
    void disconnectedFromClient();

private:
    QTcpSocket *m_serverSocket;
    QString m_userName;

public slots:
    void onReadyRead();
    void sendMessage(const QString &text,const QString &type = "message");
    void sendJson(const QJsonObject &json);
};

#endif // SERVERWORKER_H
