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
    void sendSdpToServer(const QString &sdp, const QString &targetId, const QString &clientType);

    void offerCall(QString otherClinetId);
    void answerCall(QString offererId, QString offererSdp);
    void confirmCall(QString answererSdp);

Q_SIGNALS:
    void sdpReceived(const QString &sdp);

public Q_SLOTS:
    void offererIsReady(const QString& peerId, const QString& sdp);
    void answererIsReady(const QString& peerId, const QString& sdp);

private:
    WebRTC webrtc;
    QString peerId;
    QString clientId;
    QString otherClientId;
    QTcpSocket *socket;


    void onConnected();
    void onReadyRead();
    void onDisconnected();

    // void connectedToServer();
    

};

#endif
