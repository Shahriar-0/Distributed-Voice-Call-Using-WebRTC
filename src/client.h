#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QtWebSockets>
#include <QJsonObject>
#include <QJsonDocument>

#include "webrtc.h"

class Client : public QObject {
    Q_OBJECT
public:
    explicit Client(QObject* parent = nullptr, const QString &clientId = "", bool is_offerer = false);
    void connectToServer(const QString &serverIp, int serverPort);
    void sendSdpToClient(const QJsonObject &sdp, const QString &targetId);

    void offerCall();
    void answerCall();

Q_SIGNALS:
    void sdpReceived(const QJsonObject &sdp);

// public Q_SLOTS:
//     void x();

private:
    WebRTC webrtc;
    QString peerId;
    QString clientId;
    QTcpSocket *socket;


    void onConnected();
    void onReadyRead();
    void onDisconnected();

    // void connectedToServer();
    

};

#endif
