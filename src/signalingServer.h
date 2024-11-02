#ifndef SIGNALINGSERVER_H
#define SIGNALINGSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>

class SignalingServer : public QTcpServer {
    Q_OBJECT

public:
    explicit SignalingServer(QObject *parent = nullptr);
    bool startServer(const QHostAddress &address, quint16 port);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpSocket* getClientSocketById(const QString &clientId);
    bool hasThisClient(const QString &clientId);
    QMap<QTcpSocket *, QString> clients;  // Maps sockets to client IDs
};

#endif // SIGNALINGSERVER_H
