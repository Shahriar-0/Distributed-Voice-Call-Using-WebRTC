#include "client.h"

Client::Client(QObject* parent, const QString &clientId, bool isOfferer)
    : QObject{parent}, webrtc() {


    // connect(this, &Client::sdpReceived, this, &Client::x);


    this->clientId = clientId;
    this->peerId = clientId;// TEMP

    // tcp connectio to signalingServer
    this->socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Client::onDisconnected);

    // webrtc
}

void Client::offerCall() {
    webrtc.init(this->clientId, true);
    webrtc.addPeer(this->peerId);
    webrtc.generateOfferSDP(this->peerId);
}

// void Client::answerCall(QString &offererSDP) {
//     webrtc.init(this->clientId, false);
//     webrtc.addPeer(this->peerId);
//     webrtc.generateAnswerSDP(this->peerId);
//     webrtc.setRemoteDescription(this->peerId, offererSDP);
// }


void Client::connectToServer(const QString &serverIp, int serverPort) {
    socket->connectToHost(QHostAddress(serverIp), serverPort);
}

void Client::sendSdpToClient(const QJsonObject &sdp, const QString &targetId) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject message;
        message["sdp"] = sdp;
        message["targetId"] = targetId;

        QJsonDocument doc(message);
        socket->write(doc.toJson());
    } else {
        qDebug() << "Not connected to server.";
    }
}

void Client::onConnected() {
        qDebug() << "Connected to signaling server.";

        QJsonObject message;
        message["clientId"] = clientId;
        QJsonDocument doc(message);
        socket->write(doc.toJson());

        // this->sendSdpToClient("sddppp", "client2");
    }

void Client::onReadyRead() {
    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    QJsonObject receivedSdp = obj["sdp"].toObject();

    if (!receivedSdp.isEmpty()) {
        emit sdpReceived(receivedSdp); 
    }
}

void Client::onDisconnected() {
    qDebug() << "Disconnected from server.";
}


