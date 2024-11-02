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
    explicit Client(QObject* parent = nullptr, const QString &clientId = "");
    void connectToServer(const QString &serverIp, int serverPort);
    void sendSdpToClient(const QString &sdp, const QString &targetId);
private:
    WebRTC webrtc;
    QString peerId;
    bool is_offerer;
    QString clientId;
    QTcpSocket *socket;


    void onConnected();
    void onReadyRead();
    void onDisconnected();

signals:
    void sdpReceived(const QString &sdp);
    // void connectedToServer();
};

#endif
