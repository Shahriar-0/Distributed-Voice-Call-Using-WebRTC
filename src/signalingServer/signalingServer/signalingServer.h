#ifndef SIGNALINGSERVER_H
#define SIGNALINGSERVER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QTcpServer>
#include <QTcpSocket>

class SignalingServer : public QTcpServer {
    Q_OBJECT

public:
    explicit SignalingServer(QObject* parent = nullptr);
    bool startServer(const QHostAddress& address, quint16 port);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpSocket* getClientSocketById(const QString& clientId);
    bool hasThisClient(const QString& clientId);
    QMap<QString, QTcpSocket*> clients; // Maps sockets to client IDs
};

#endif // SIGNALINGSERVER_H
