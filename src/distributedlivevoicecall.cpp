#include "distributedlivevoicecall.h"

#include <QAudioFormat>
#include <QByteArray>
#include <QMediaDevices>

#define SAMPLE_RATE 48000
#define CHANNELS    1
#define FRAME_SIZE  960 // Opus frame size for 20ms @ 48kHz, or 960 samples

DistributedLiveVoiceCall::DistributedLiveVoiceCall(QObject* parent)
    : QObject(parent), audioSource(nullptr), audioSink(nullptr), audioInputDevice(nullptr), opusEncoder(nullptr), opusDecoder(nullptr) {
    playbackTimer.setSingleShot(true);

    int opusEncError, opusDecError;
    opusEncoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &opusEncError);
    opusDecoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusDecError);

    if (opusEncError != OPUS_OK || opusDecError != OPUS_OK) {
        qWarning() << "Failed to create Opus encoder or decoder!";
    }

    frameSize = FRAME_SIZE;
}

DistributedLiveVoiceCall::~DistributedLiveVoiceCall() {
    if (opusEncoder) opus_encoder_destroy(opusEncoder);
    if (opusDecoder) opus_decoder_destroy(opusDecoder);
}

void DistributedLiveVoiceCall::startCall(const QString& callerID) {
    setCallerID(callerID);
    qDebug() << "Start call triggered with Caller ID:" << callerID;
    startAudioRecording();
}

void DistributedLiveVoiceCall::endCall() {
    qDebug() << "End call triggered!";
    stopAudioRecording();
    qDebug() << "Recorded audio data size:" << opusPacketList.size() << " packets";

    // Start playing back the recorded audio after decoding all packets
    playbackTimer.start(3000);
    connect(&playbackTimer, &QTimer::timeout, this, &DistributedLiveVoiceCall::playRecordedAudio);
}

void DistributedLiveVoiceCall::startAudioRecording() {
    if (audioSource != nullptr) {
        delete audioSource;
    }

    opusPacketList.clear();     // Clear any previous audio data
    decodedAudioBuffer.clear(); // Clear the decoded buffer

    // Clear buffer for fresh recording
    audioBuffer.close();
    audioBuffer.setData(QByteArray());
    audioBuffer.open(QIODevice::WriteOnly);

    QAudioFormat format;
    format.setSampleRate(SAMPLE_RATE);
    format.setChannelCount(CHANNELS);
    format.setSampleFormat(QAudioFormat::Int16);

    audioSource = new QAudioSource(QMediaDevices::defaultAudioInput(), format, this);

    audioInputDevice = audioSource->start();

    connect(audioInputDevice, &QIODevice::readyRead, this, [this]() {
        QByteArray audioData = audioInputDevice->readAll();

        if (audioData.size() >= frameSize * 2) {                // Ensure at least 1 frame of data (2 bytes per sample)
            encodeAndStoreAudio(audioData.left(frameSize * 2)); // Only pass frame-sized chunks to encoder
        }
    });

    qDebug() << "Audio recording started.";
}

void DistributedLiveVoiceCall::stopAudioRecording() {
    if (audioSource) {
        audioSource->stop();
        delete audioSource;
        audioSource = nullptr;
    }

    audioBuffer.close();
    qDebug() << "Audio recording stopped.";
}

void DistributedLiveVoiceCall::encodeAndStoreAudio(const QByteArray& audioData) {
    std::vector<unsigned char> opusData(4000); // Max packet size
    int frameSize = audioData.size() / 2;      // 2 bytes per sample (16-bit audio)

    int encodedBytes = opus_encode(opusEncoder,
                                   reinterpret_cast<const opus_int16*>(audioData.data()),
                                   frameSize,
                                   opusData.data(),
                                   opusData.size());

    if (encodedBytes > 0) {
        QByteArray encodedOpusData(reinterpret_cast<const char*>(opusData.data()), encodedBytes);
        opusPacketList.append(encodedOpusData);
    }
    else {
        qWarning() << "Opus encoding failed with error:" << encodedBytes;
    }
}

void DistributedLiveVoiceCall::playRecordedAudio() {
    if (opusPacketList.isEmpty()) {
        qDebug() << "No audio data to play.";
        return;
    }

    qDebug() << "Playing recorded audio. Number of packets:" << opusPacketList.size();

    // Decode each packet and accumulate the decoded audio data
    for (const QByteArray& packet : opusPacketList) {
        decodeAndAccumulateAudio(packet);
    }

    if (!decodedAudioBuffer.isEmpty()) {
        QAudioFormat format;
        format.setSampleRate(SAMPLE_RATE);
        format.setChannelCount(CHANNELS);
        format.setSampleFormat(QAudioFormat::Int16);

        if (audioSink != nullptr) {
            delete audioSink;
        }
        audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format, this);

        audioBuffer.setData(decodedAudioBuffer);
        audioBuffer.open(QIODevice::ReadOnly);
        audioSink->start(&audioBuffer);

        qDebug() << "Playing accumulated decoded audio.";
    }
}

void DistributedLiveVoiceCall::decodeAndAccumulateAudio(const QByteArray& encodedOpusData) {
    std::vector<opus_int16> decodedOutput(FRAME_SIZE * CHANNELS);

    int decodedBytes = opus_decode(opusDecoder,
                                   reinterpret_cast<const unsigned char*>(encodedOpusData.data()),
                                   encodedOpusData.size(),
                                   decodedOutput.data(),
                                   FRAME_SIZE,
                                   0);

    if (decodedBytes > 0) {
        QByteArray decodedAudio(reinterpret_cast<const char*>(decodedOutput.data()), decodedBytes * 2);
        decodedAudioBuffer.append(decodedAudio);
    }
    else {
        qWarning() << "Opus decoding failed with error:" << decodedBytes;
    }
}
