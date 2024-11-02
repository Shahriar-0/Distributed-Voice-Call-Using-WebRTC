#include "client.h"

Client::Client(QObject* parent)
    : QObject{parent}, webrtc() {
        QHostAddress destinationAddress("192.168.1.100");
        quint16 destinationPort = 1234;

        // init all the fucking motherfuckers

        webrtc.init(this->id, this->is_offerer);
        webrtc.addPeer(this->peerId);

        if(this->is_offerer) {
            webrtc.generateOfferSDP(this->peerId);
        }
        else {
            webrtc.generateAnswerSDP(this->peerId);
        }

        QJsonObject sdp;
        // key-val all the fucking motherfuckers
        sdp["ice"] = NULL;

        QJsonDocument jsonDoc(sdp);
        
        // webrtc.setRemoteDescription(this->peerId, NULL);
        webrtc.setRemoteCandidate(this->peerId, NULL);
        
}
