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
    connect(&webrtc, &WebRTC::localDescriptionGenerated, this, &Client::sdpReady);
}

void Client::sdpReady(const QString& peerId, const QString& sdp) {
    qDebug() << "sdp Ready";
    this->sendSdpToServer(sdp, this->otherClientId);
}

void Client::offerCall(QString answererId) {
    this->otherClientId = answererId;

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

void Client::sendSdpToServer(const QString &sdp, const QString &targetId) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject message;
        message["sourceId"] = clientId;
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

    }

void Client::onReadyRead() {
    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    QString sourceId = obj["sourceId"].toString();
    QString receivedSdp = obj["sdp"].toString();

    qDebug() << clientId << "received:" << receivedSdp << "from" << sourceId;

    if (!receivedSdp.isEmpty()) {
        emit sdpReceived(receivedSdp); 
    }
}

void Client::onDisconnected() {
    qDebug() << "Disconnected from server.";
}


