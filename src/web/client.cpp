#include "client.h"

Client::Client(QObject* parent, const QString &clientId)
    : QObject{parent}, webrtc(), audioinput(), audiooutput() {

    this->clientId = clientId;
    this->peerId = clientId;// TEMP

    // tcp connectio to signalingServer
    this->socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Client::onDisconnected);

    // webrtc
    connect(&webrtc, &WebRTC::offerIsReady, this, &Client::offererIsReady);
    connect(&webrtc, &WebRTC::answerIsReady, this, &Client::answererIsReady);
    connect(&webrtc, &WebRTC::incommingPacket, this, &Client::packetRecieved);
    connect(&audioinput, &AudioInput::newAudioData, this, &Client::packetReady);

    connect(&webrtc, &WebRTC::p2pConnected, this, &Client::startAudio);
    connect(&webrtc, &WebRTC::p2pDisconnected, this, &Client::endAudio);
}

void Client::endCall() {
    webrtc.closeConnection(this->peerId);
}

void Client::startAudio() {
    if (!audioinput.startAudio()) {
        qWarning() << "Failed to start audio input.";
    }
}

void Client::endAudio() {
    if (!audioinput.stopAudio()) {
        qWarning() << "Failed to stop audio input.";
    }
    emit callEnded();
}

void Client::packetRecieved(const QString& peerId, const QByteArray& data, qint64 len) {
    qDebug() << "packet received";
    audiooutput.addData(data);
}
void Client::packetReady(const QByteArray& buffer) {
    qDebug() << "packet ready";
    webrtc.sendTrack(this->peerId, buffer);
}

void Client::offererIsReady(const QString& peerId, const QString& sdp) {
    qDebug() << "offerer sdp Ready";
    this->sendSdpToServer(sdp, this->otherClientId, "offerer");
}

void Client::answererIsReady(const QString& peerId, const QString& sdp) {
    qDebug() << "answerer sdp Ready";
    this->sendSdpToServer(sdp, this->otherClientId, "answerer");
}

void Client::offerCall(QString answererId) {
    webrtc.init(this->clientId, true);
    webrtc.addPeer(this->peerId);

    this->otherClientId = answererId;
    webrtc.generateOfferSDP(this->peerId);
}

void Client::answerCall(QString offererId, QString offererSdp) {
    webrtc.init(this->clientId, false);
    webrtc.addPeer(this->peerId);

    this->otherClientId = offererId;
    
    webrtc.setRemoteDescription(this->peerId, offererSdp, rtc::Description::Type::Offer);
    webrtc.generateAnswerSDP(this->peerId);
    emit callReceived();
}

void Client::confirmCall(QString answererSdp) {
    webrtc.setRemoteDescription(this->peerId, answererSdp, rtc::Description::Type::Answer);
}


void Client::connectToServer(const QString &serverIp, int serverPort) {
    socket->connectToHost(QHostAddress(serverIp), serverPort);
}

void Client::sendSdpToServer(const QString &sdp, const QString &targetId, const QString &clientType) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject message;
        message["sourceId"] = clientId;
        message["clientType"] = clientType;
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
    QString clientType = obj["clientType"].toString();
    QString receivedSdp = obj["sdp"].toString();

    QString sdp = QJsonDocument::fromJson(receivedSdp.toUtf8()).object()["sdp"].toString();;

    if(clientType == "offerer") {
        answerCall(sourceId, sdp);
    } else if(clientType == "answerer") {
        confirmCall(sdp);
    } else {
        qDebug() << "Invalid client type";
    }

    if (!receivedSdp.isEmpty()) {
        emit sdpReceived(sdp); 
    }
}

void Client::onDisconnected() {
    qDebug() << "Disconnected from server.";
}


