#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QtWebSockets>
#include <QJsonObject>
#include <QJsonDocument>

#include "webrtc.h"
#include "../audio/audioinput.h"
#include "../audio/audiooutput.h"

class Client : public QObject {
    Q_OBJECT
public:
    explicit Client(QObject* parent = nullptr, const QString &clientId = "");
    void connectToServer(const QString &serverIp, int serverPort);
    void sendSdpToServer(const QString &sdp, const QString &targetId, const QString &clientType);

    Q_INVOKABLE void offerCall(QString otherClinetId);
    void answerCall(QString offererId, QString offererSdp);
    void confirmCall(QString answererSdp);
    Q_INVOKABLE void endCall();

Q_SIGNALS:
    void sdpReceived(const QString &sdp);
    void callReceived();
    void callEnded();

public Q_SLOTS:
    void offererIsReady(const QString& peerId, const QString& sdp);
    void answererIsReady(const QString& peerId, const QString& sdp);
    void packetRecieved(const QString& peerId, const QByteArray& data, qint64 len);
    void packetReady(const QByteArray& buffer);
    void startAudio();
    void endAudio();

private:
    WebRTC webrtc;
    QString peerId;
    QString clientId;
    QString otherClientId;
    QTcpSocket *socket;

    AudioInput audioinput;
    AudioOutput audiooutput;

    void onConnected();
    void onReadyRead();
    void onDisconnected();

    // void connectedToServer();
    

};

#endif
