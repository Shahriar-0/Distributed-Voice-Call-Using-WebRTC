#include "webrtc.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>

static_assert(true);

#pragma pack(push, 1)
struct RtpHeader {
    uint8_t first;
    uint8_t marker : 1;
    uint8_t payloadType : 7;
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint32_t ssrc;
};
#pragma pack(pop)

WebRTC::WebRTC(QObject* parent) : QObject{parent} {
    connect(this, &WebRTC::gatheringComplited, [this](const QString& peerId) {
        this->m_localDescription = descriptionToJson(m_peerConnections[peerId]->localDescription().value());

        Q_EMIT localDescriptionGenerated(peerId, m_localDescription);

        if (m_isOfferer)
            Q_EMIT this->offerIsReady(peerId, m_localDescription);
        else
            Q_EMIT this->answerIsReady(peerId, m_localDescription);
    });

}

WebRTC::~WebRTC() {}


void WebRTC::init(const QString& id, bool isOfferer) {
    // Initialize WebRTC using libdatachannel library
    this->m_isOfferer = isOfferer;
    this->m_localId = id;

    // Create an instance of rtc::Configuration to Set up ICE configuration
    rtc::Configuration config;

    // Add a STUN server to help peers find their public IP addresses
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    // Add a TURN server for relaying media if a direct connection can't be established
    // config.iceServers.emplace_back("turn:turnserver.com:3478", "username", "password");

    // Set up the audio stream configuration

    this->m_config = config;
}

void WebRTC::addPeer(const QString& peerId) {
    // Create and add a new peer connection
    auto newPeer = std::make_shared<rtc::PeerConnection>(this->m_config);
    this->m_peerConnections[peerId] = newPeer;


    // Set up a callback for handling local ICE candidates
    newPeer->onLocalCandidate([this, peerId](const rtc::Candidate& candidate) {
        QString candidateStr = QString::fromStdString(candidate.candidate());
        Q_EMIT localCandidateGenerated(peerId, QString::fromStdString(candidate.candidate()), QString());
    });

    // Set up a callback for when the state of the peer connection changes
    newPeer->onStateChange([this, peerId](rtc::PeerConnection::State state) {
        std::cout << "Peer State " << peerId.toStdString() << " Changed to:" << state << std::endl;
        if (state == rtc::PeerConnection::State::Connected) {
            emit p2pConnected();

        } else if (state == rtc::PeerConnection::State::Disconnected) {
            emit p2pDisconnected();
        }
    });

    // Set up a callback for monitoring the gathering state
    newPeer->onGatheringStateChange([this, peerId](rtc::PeerConnection::GatheringState state) {
        if (state == rtc::PeerConnection::GatheringState::Complete) {
            qDebug() << "gathering completed";
            Q_EMIT gatheringComplited(peerId);
        }
    });

    // Set up a callback for handling incoming tracks
    newPeer->onTrack([this, peerId](std::shared_ptr<rtc::Track> track) {
        track->onMessage([this, peerId](rtc::message_variant data) {
            QByteArray buffer = readVariant(data);
            Q_EMIT incommingPacket(peerId, buffer, buffer.size());
        });
    });

    // Add an audio track to the peer connection
    addAudioTrack(peerId, "Audio");
}

// Set the local description for the peer's connection
void WebRTC::generateOfferSDP(const QString& peerId) {
    if (!m_peerConnections.contains(peerId)) {
        qDebug() << "Peer ID not found:" << peerId;
        return;
    }
    auto peer = m_peerConnections[peerId];    
    peer->setLocalDescription(rtc::Description::Type::Offer);
}

// Generate an answer SDP for the peer
void WebRTC::generateAnswerSDP(const QString& peerId) {
    if (!m_peerConnections.contains(peerId)) {
        qDebug() << "Peer ID not found:" << peerId;
        return;
    }
    auto peer = m_peerConnections[peerId];
    peer->localDescription()->generateSdp();
}

// Add an audio track to the peer connection
void WebRTC::addAudioTrack(const QString& peerId, const QString& trackName) {
    auto peer = m_peerConnections[peerId];
    this->m_audio = rtc::Description::Audio(trackName.toStdString(), rtc::Description::Direction::SendRecv);
    auto audioTrack = peer->addTrack(this->m_audio);

    audioTrack->onMessage([this, peerId](rtc::message_variant data) {
        QByteArray buffer = readVariant(data);
        Q_EMIT incommingPacket(peerId, buffer, buffer.size());
    });

    m_peerTracks[peerId] = audioTrack;
}

// Sends audio track data to the peer
void WebRTC::sendTrack(const QString& peerId, const QByteArray& buffer) {
    auto track = this->m_peerTracks[peerId];

    // Create the RTP header and initialize an RtpHeader struct
    RtpHeader header;
    header.first = 0x80;
    header.marker = 0;
    header.payloadType = this->m_payloadType;
    header.sequenceNumber = qToBigEndian(this->m_sequenceNumber++);
    header.timestamp = qToBigEndian(getCurrentTimestamp());
    header.ssrc = qToBigEndian(this->m_ssrc);

    // Create the RTP packet by appending the RTP header and the payload buffer
    QByteArray rtpPacket;
    rtpPacket.append(reinterpret_cast<char*>(&header), sizeof(RtpHeader));
    rtpPacket.append(buffer);

    std::vector<std::byte> packetData(rtpPacket.size());
    std::transform(rtpPacket.begin(), rtpPacket.end(), packetData.begin(), [](char c) {
        return static_cast<std::byte>(c);
    });

    try {
        track->send(packetData);
        qDebug() << "Sent RTP packet to peer" << peerId << "with size:" << rtpPacket.size();    }
    catch (const std::exception& e) {
        qDebug() << "Error sending RTP packet: " << e.what();
    }
}

/**
 * ====================================================
 * ================= public slots =====================
 * ====================================================
 */

// Set the remote SDP description for the peer that contains metadata about the media being
// transmitted
void WebRTC::setRemoteDescription(const QString& peerId, const QString& sdp , rtc::Description::Type remoteType) {
    auto peer = m_peerConnections[peerId];
    peer->setRemoteDescription(rtc::Description(sdp.toStdString(), remoteType));
}

// Add remote ICE candidates to the peer connection
void WebRTC::setRemoteCandidate(const QString& peerId, const QString& candidate, const QString& sdpMid) {
    auto peer = m_peerConnections[peerId];
    peer->addRemoteCandidate(rtc::Candidate(candidate.toStdString(), sdpMid.toStdString()));
}

/*
 * ====================================================
 * ================= private methods ==================
 * ====================================================
 */

// Utility function to read the rtc::message_variant into a QByteArray
QByteArray WebRTC::readVariant(const rtc::message_variant &data)
{
    QByteArray result;

    if (std::holds_alternative<std::string>(data)) {
        const auto &str = std::get<std::string>(data);
        result = QByteArray::fromStdString(str);
    }
    else if (std::holds_alternative<rtc::binary>(data)) {
        const rtc::binary &binaryData = std::get<rtc::binary>(data);
        result = QByteArray(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
    } else {
        qWarning() << "Unsupported data type in rtc::message_variant";
        return result;
    }

    const int rtpHeaderSize = sizeof(RtpHeader);

    if (result.size() >= rtpHeaderSize) {
        result = result.mid(rtpHeaderSize);
    } else {
        qWarning() << "Data size is smaller than RTP header size. Unable to remove header.";
    }

    return result;
}

// Utility function to convert rtc::Description to JSON format
QString WebRTC::descriptionToJson(const rtc::Description& description) {
    QJsonObject json;
    json["type"] = QString::fromStdString(description.typeString());
    json["sdp"] = QString::fromStdString(static_cast<std::string>(description));

    // Convert to JSON string
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact);
}

// Retrieves the current bit rate
int WebRTC::bitRate() const {
    return m_bitRate;
}

// Set a new bit rate and emit the bitRateChanged signal
void WebRTC::setBitRate(int newBitRate) {
    if (m_bitRate == newBitRate)
    return;

    m_bitRate = newBitRate;
    Q_EMIT bitRateChanged();
}

// Reset the bit rate to its default value
void WebRTC::resetBitRate() {
    setBitRate(48000);
}

// Sets a new payload type and emit the payloadTypeChanged signal
void WebRTC::setPayloadType(int newPayloadType) {
    if (m_payloadType == newPayloadType)
        return;

    m_payloadType = newPayloadType;
    emit payloadTypeChanged();
}

// Resets the payload type to its default value
void WebRTC::resetPayloadType() {
    setPayloadType(111);
}

// Retrieve the current payload type
int WebRTC::payloadType() const {
    return m_payloadType;
}
// Retrieve the current SSRC value
rtc::SSRC WebRTC::ssrc() const {
    return m_ssrc;
}

// Set a new SSRC and emit the ssrcChanged signal
void WebRTC::setSsrc(rtc::SSRC newSsrc) {
    if (m_ssrc == newSsrc)
        return;
    m_ssrc = newSsrc;
    Q_EMIT ssrcChanged();
}

// Reset the SSRC to its default value
void WebRTC::resetSsrc() {
    setSsrc(2);
}


/**
 * ====================================================
 * ================= getters setters ==================
 * ====================================================
 */

bool WebRTC::isOfferer() const {
    return m_isOfferer;
}

void WebRTC::setIsOfferer(bool newIsOfferer) {
    if (m_isOfferer == newIsOfferer)
        return;
    m_isOfferer = newIsOfferer;
    Q_EMIT isOffererChanged();
}

void WebRTC::resetIsOfferer() {
    setIsOfferer(false);
}

void WebRTC::closeConnection(const QString& peerId) {
    m_peerConnections[peerId]->close();
    m_peerTracks[peerId]->close();
    m_peerConnections.remove(peerId);
    m_gatheringComplited = false;
}