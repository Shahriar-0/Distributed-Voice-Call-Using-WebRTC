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
    explicit Client(QObject* parent = nullptr);
private:
    WebRTC webrtc;
    QUdpSocket udpSocket;
    QString peerId;
    QString id;
    bool is_offerer;
signals:
};

#endif // CLIENT_H
