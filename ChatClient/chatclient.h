#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QTcpSocket>

class ChatClient : public QObject
{
    Q_OBJECT
public:
    explicit ChatClient(QObject *parent = nullptr);
    void setUserName(const QString &name);
    QString userName();

signals:
    void connected();
    void messageReceived(const QString &text);
    void jsonReceived(const QJsonObject &docObj);
    void privateMessageReceived(const QString &sender, const QString &reciver, const QString &message);  // 添加信号

private:
    QTcpSocket *m_clientSocket;
    QString m_userName;

public slots:
    void onReadyRead();
    void sendMessage(const QString &text,const QString &type = "message");
    void sendPrivateMessage(const QString &text,const QString &text2,const QString &type = "message");
    void sendBothPrivateMessage(const QString &text, const QString &text2, const QString &message, const QString &type = "message");
    void sendCloseMessage(const QString &text, const QString &text2, const QString &type = "message");
    void connectToServer(const QHostAddress &address,quint16 port);
    void disconnectFromHost();
};

#endif // CHATCLIENT_H
