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

    qDebug() << "Server: New Connection";
}

void SignalingServer::onReadyRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (!clientSocket) return;

    QByteArray data = clientSocket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    QString clientId = obj["clientId"].toString();
    QString sdp = obj["sdp"].toString();
    QString targetId = obj["targetId"].toString();

    if (!clientId.isEmpty()) {
        clients[clientSocket] = clientId;
        qDebug() << clientId;
        return;
    }
    else {
        if (hasThisClient(targetId)) {
            // Forward SDP to the target client
            QTcpSocket *targetSocket = getClientSocketById(targetId);
            if (targetSocket) {
                QJsonObject response;
                response["sdp"] = sdp;
                targetSocket->write(QJsonDocument(response).toJson());
            }
        }
    }

}

void SignalingServer::onDisconnected() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (!clientSocket) return;

    QString clientId = clients[clientSocket];
    clients.remove(clientSocket);
    clientSocket->deleteLater();

    qDebug() << "Server: client disconnected";
}

QTcpSocket* SignalingServer::getClientSocketById(const QString &clientId) {
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it.value() == clientId)
            return it.key();
    }
    return nullptr;
}

bool SignalingServer::hasThisClient(const QString &clientId) {
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it.value() == clientId)
            return true;
    }
    return false;
}
