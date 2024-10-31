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

WebRTC::WebRTC(QObject* parent) : QObject{parent}, m_audio("Audio") {
    connect(this, &WebRTC::gatheringComplited, [this](const QString& peerID) {
        m_localDescription =
            descriptionToJson(m_peerConnections[peerID]->localDescription().value());
        Q_EMIT localDescriptionGenerated(peerID, m_localDescription);

        if (m_isOfferer)
            Q_EMIT this->offerIsReady(peerID, m_localDescription);
        else
            Q_EMIT this->answerIsReady(peerID, m_localDescription);
    });
}

WebRTC::~WebRTC() {}

// QByteArray WebRTC::readVariant(const rtc::message_variant& data) {
//     if (auto binary = std::get_if<rtc::binary>(&data))
//         return QByteArray::fromStdString(*binary);
//     return {};
// }

// QByteArray WebRTC::readVariant(const rtc::message_variant& data) {
//     if (std::holds_alternative<rtc::binary>(data)) {
//         const rtc::binary& binaryData = std::get<rtc::binary>(data);
//         return QByteArray(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
//     }
//     return {};
// }


/**
 * ====================================================
 * ================= public methods ===================
 * ====================================================
 */

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
    auto newPeer = std::make_shared<rtc::PeerConnection>(this->m_config);
    this->m_peerConnections[peerId] = newPeer;



    addAudioTrack(peerId, "audio");
    // Create and add a new peer connection

    // Set up a callback for when the local description is generated
    newPeer->onLocalDescription([this, peerId](const rtc::Description& description) {
        if (m_isOfferer)
            Q_EMIT offerIsReady(peerId, descriptionToJson(description));
        else
            Q_EMIT answerIsReady(peerId, descriptionToJson(description));
    });

    // Set up a callback for handling local ICE candidates
    newPeer->onLocalCandidate([this, peerId](const rtc::Candidate& candidate) {
        QString candidateStr = QString::fromStdString(candidate.candidate());
        Q_EMIT localCandidateGenerated(peerId, QString::fromStdString(candidate.candidate()), QString());
    });

    // // Set up a callback for when the state of the peer connection changes
    newPeer->onStateChange([this, peerId](rtc::PeerConnection::State state) {
        if (state == rtc::PeerConnection::State::Connected) {
            Q_EMIT gatheringComplited(peerId);
        } else if (state == rtc::PeerConnection::State::Disconnected) {
            // FIXME
            qDebug() << "Peer disconnected:" << peerId;
        }
    });

    // // Set up a callback for monitoring the gathering state
    newPeer->onGatheringStateChange([this, peerId](rtc::PeerConnection::GatheringState state) {
        if (state == rtc::PeerConnection::GatheringState::Complete) {
            Q_EMIT gatheringComplited(peerId);
        }
    });

    // // Set up a callback for handling incoming tracks

    qDebug() << "Received audio track from peer:" << peerId;

    newPeer->onTrack([this, peerId](std::shared_ptr<rtc::Track> track) {
        track->onMessage([this, peerId](rtc::message_variant data) {
            QByteArray buffer = readVariant(data);
            Q_EMIT incommingPacket(peerId, buffer, buffer.size());
        });
    });

    // Add an audio track to the peer connection
    // newPeer->addTrack(m_audio);
    addAudioTrack(peerId, "Audio");
}

// Set the local description for the peer's connection
void WebRTC::generateOfferSDP(const QString& peerId) {
    auto peer = m_peerConnections[peerId];
    peer->setLocalDescription(rtc::Description::Type::Offer);
}

// Generate an answer SDP for the peer
void WebRTC::generateAnswerSDP(const QString& peerId) {
    auto peer = m_peerConnections[peerId];
    peer->setLocalDescription(rtc::Description::Type::Answer);
}

// Add an audio track to the peer connection
void WebRTC::addAudioTrack(const QString& peerId, const QString& trackName) {
    auto peer = m_peerConnections[peerId];
    this->m_audio = rtc::Description::Audio(trackName.toStdString(), rtc::Description::Direction::SendRecv);
    auto audioTrack = peer->addTrack(m_audio);

    // Add an audio track to the peer connection

    audioTrack->onMessage([this, peerId](rtc::message_variant data) {
        QByteArray buffer = readVariant(data);
        Q_EMIT incommingPacket(peerId, buffer, buffer.size());
    });


    // audioTrack->onFrame([this] (rtc::binary frame, rtc::FrameInfo info) {
    //     // TODO: ?
    // });
}

// Sends audio track data to the peer

void WebRTC::sendTrack(const QString& peerId, const QByteArray& buffer) {

    auto track = this->m_peerTracks[peerId];

    // Create the RTP header and initialize an RtpHeader struct
    RtpHeader header;
    header.first = 0x80;
    header.marker = 1;
    header.payloadType = this->m_payloadType;
    header.sequenceNumber = qToBigEndian(++this->m_sequenceNumber);
    header.timestamp = qToBigEndian(getCurrentTimestamp());
    header.ssrc = qToBigEndian(this->m_ssrc);

    // Create the RTP packet by appending the RTP header and the payload buffer
    QByteArray rtpPacket;
    rtpPacket.append(reinterpret_cast<char*>(&header), sizeof(RtpHeader));
    rtpPacket.append(buffer);

    // Send the packet, catch and handle any errors that occur during sending
    try {
        track->send(rtpPacket.toStdString());  // Ensuring it's sent as a std::string (binary data)
    }
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
void WebRTC::setRemoteDescription(const QString& peerID, const QString& sdp) {
    auto peer = m_peerConnections[peerID];
    peer->setRemoteDescription(rtc::Description(sdp.toStdString(), rtc::Description::Type::Offer));
}

// Add remote ICE candidates to the peer connection
void WebRTC::setRemoteCandidate(const QString& peerID, const QString& candidate, const QString& sdpMid) {
    auto peer = m_peerConnections[peerID];
    peer->addRemoteCandidate(rtc::Candidate(candidate.toStdString(), sdpMid.toStdString()));
}


/*
 * ====================================================
 * ================= private methods ==================
 * ====================================================
 */

// Utility function to read the rtc::message_variant into a QByteArray
QByteArray WebRTC::readVariant(const rtc::message_variant& data) {
    return QByteArray::fromStdString(std::get<std::string>(data));
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
