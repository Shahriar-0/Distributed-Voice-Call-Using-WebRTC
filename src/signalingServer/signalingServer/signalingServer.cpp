#include "signalingserver.h"

SignalingServer::SignalingServer(QObject *parent)
    : QTcpServer(parent) {
    connect(this, &QTcpServer::newConnection, this, &SignalingServer::onNewConnection);
}

bool SignalingServer::startServer(const QHostAddress &address, quint16 port) {
    return listen(address, port);
}

void SignalingServer::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *clientSocket = new QTcpSocket(this);
    clientSocket->setSocketDescriptor(socketDescriptor);
    connect(clientSocket, &QTcpSocket::readyRead, this, &SignalingServer::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &SignalingServer::onDisconnected);
}

void SignalingServer::onNewConnection() {
    QTcpSocket *clientSocket = nextPendingConnection();
}

void SignalingServer::onReadyRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (!clientSocket) return;

    QByteArray data = clientSocket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    QString clientId = obj["clientId"].toString();
    QString sourceId = obj["sourceId"].toString();
    QString clientType = obj["clientType"].toString();
    QString sdp = obj["sdp"].toString();
    QString targetId = obj["targetId"].toString();

    if (!clientId.isEmpty()) {
        clients[clientId] = clientSocket;
        qDebug() << "Server: New Connection from" << clientId;

        return;
    }
    else {
        if (clients.contains(targetId)) {
            QTcpSocket *targetSocket = clients[targetId];
            if (targetSocket) {
                QJsonObject response;
                response["sourceId"] = sourceId;
                response["clientType"] = clientType;
                response["sdp"] = sdp;
                targetSocket->write(QJsonDocument(response).toJson());
            }
        }
    }

}

void SignalingServer::onDisconnected() {
    // QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    // if (!clientSocket) return;

    // QString clientId = clients[clientSocket];
    // clients.remove(clientSocket);
    // clientSocket->deleteLater();

    qDebug() << "Server: client disconnected";
}