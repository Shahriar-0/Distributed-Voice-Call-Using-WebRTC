#include "distributedlivevoicecall.h"
#include <QAudioFormat>
#include <QMediaDevices>
#include <QByteArray>

DistributedLiveVoiceCall::DistributedLiveVoiceCall(QObject* parent)
    : QObject(parent), audioSource(nullptr), audioSink(nullptr),
      opusEncoder(nullptr), opusDecoder(nullptr) {

    playbackTimer.setSingleShot(true);
    connect(&playbackTimer, &QTimer::timeout, this, &DistributedLiveVoiceCall::playRecordedAudio);

    int sampleRate = 48000;
    int channels = 1;

    int opusEncError, opusDecError;

    opusEncoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_VOIP, &opusEncError);
    if (opusEncError != OPUS_OK) {
        qDebug() << "Failed to initialize Opus encoder: " << opusEncError;
    }

    opusDecoder = opus_decoder_create(sampleRate, channels, &opusDecError);
    if (opusDecError != OPUS_OK) {
        qDebug() << "Failed to initialize Opus decoder: " << opusDecError;
    }
}

DistributedLiveVoiceCall::~DistributedLiveVoiceCall() {
    opus_encoder_destroy(opusEncoder);
    opus_decoder_destroy(opusDecoder);
}

void DistributedLiveVoiceCall::startCall(const QString& callerID) {
    setCallerID(callerID);
    qDebug() << "Start call triggered with Caller ID:" << callerID;

    startAudioRecording();
}

void DistributedLiveVoiceCall::endCall() {
    qDebug() << "End call triggered!";

    stopAudioRecording();

    qDebug() << "Recorded audio data size:" << audioBuffer.data().size() << "bytes";

    playbackTimer.start(3000);
}

void DistributedLiveVoiceCall::startAudioRecording() {
    if (audioSource != nullptr) {
        delete audioSource;  // Clean up old audioSource
    }

    // Clear the buffer to ensure no leftover data from previous recordings
    audioBuffer.close();
    audioBuffer.setData(QByteArray());
    audioBuffer.setBuffer(new QByteArray());
    audioBuffer.open(QIODevice::WriteOnly);

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    audioSource = new QAudioSource(QMediaDevices::defaultAudioInput(), format, this);
    audioSource->start(&audioBuffer);

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
    qDebug() << "Buffer data size after recording:" << audioBuffer.data().size() << "bytes";
}

void DistributedLiveVoiceCall::playRecordedAudio() {
    if (audioBuffer.data().isEmpty()) {
        qDebug() << "No audio data to play.";
        return;
    }

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format, this);

    audioBuffer.close();
    audioBuffer.open(QIODevice::ReadOnly);

    audioSink->start(&audioBuffer);

    qDebug() << "Playing recorded audio. Data size:" << audioBuffer.data().size() << "bytes";
}

QByteArray DistributedLiveVoiceCall::encodeOpus(const QByteArray& pcmData) {
    QByteArray encodedData;
    const int maxPacketSize = 4000;
    int frameSize = pcmData.size() / sizeof(opus_int16);

    encodedData.resize(maxPacketSize);
    int encodedBytes = opus_encode(opusEncoder, reinterpret_cast<const opus_int16*>(pcmData.constData()), frameSize, reinterpret_cast<unsigned char*>(encodedData.data()), maxPacketSize);

    if (encodedBytes < 0) {
        qDebug() << "Opus encoding error:" << encodedBytes;
        return QByteArray();
    }

    return encodedData.left(encodedBytes);
}

QByteArray DistributedLiveVoiceCall::decodeOpus(const QByteArray& opusData) {
    QByteArray decodedData;
    int frameSize = 960;  // Typical frame size for Opus in WebRTC (20ms at 48kHz)

    decodedData.resize(frameSize * sizeof(opus_int16));
    int decodedSamples = opus_decode(opusDecoder, reinterpret_cast<const unsigned char*>(opusData.constData()), opusData.size(), reinterpret_cast<opus_int16*>(decodedData.data()), frameSize, 0);

    if (decodedSamples < 0) {
        qDebug() << "Opus decoding error:" << decodedSamples;
        return QByteArray();
    }

    decodedData.resize(decodedSamples * sizeof(opus_int16));
    return decodedData;
}
