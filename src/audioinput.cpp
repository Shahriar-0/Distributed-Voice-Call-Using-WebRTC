#include "audioinput.h"

AudioInput::AudioInput(QObject* parent)
    : QIODevice(parent),
      audioSource(nullptr),
      opusEncoder(nullptr),
      opusFrameSize(960) {
    int error;
    opusEncoder = opus_encoder_create(48000, 1, OPUS_APPLICATION_VOIP, &error); // 48kHz, mono
    opus_encoder_ctl(opusEncoder, OPUS_SET_BITRATE(64000));  // Set bitrate to 64 kbps
    opus_encoder_ctl(opusEncoder, OPUS_SET_COMPLEXITY(10));  // Keep maximum complexity for better quality
    opus_encoder_ctl(opusEncoder, OPUS_SET_INBAND_FEC(1));   // Enable Forward Error Correction (FEC)
    opus_encoder_ctl(opusEncoder, OPUS_SET_DTX(0));          // Disable DTX to avoid noise during silence

    if (error != OPUS_OK) {
        qWarning() << "Failed to create Opus encoder:" << opus_strerror(error);
    }

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice info = QMediaDevices::defaultAudioInput();
    qDebug() << info.description();
    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
    }

    audioSource = new QAudioSource(info, format, this);
    if (!audioSource) {
        qWarning() << "Failed to create QAudioSource.";
        return;
    }
}

AudioInput::~AudioInput() {
    opus_encoder_destroy(opusEncoder);
}

bool AudioInput::startAudio() {
    if (!audioSource)
        return false;
    if (!isOpen()) {
        open(QIODevice::ReadWrite);
    }
    audioSource->start(this);
    if (audioSource->state() != QAudio::ActiveState) {
        qWarning() << "Audio source is not active. State:" << audioSource->state();
        return false;
    }
    return true;
}

bool AudioInput::stopAudio() {
    if (!audioSource)
        return false;
    if (!isOpen()) {
        open(QIODevice::ReadWrite);
    }
    audioSource->stop();
    if (audioSource->state() != QAudio::StoppedState) {
        qWarning() << "Audio source is active. State:" << audioSource->state();
        return false;
    }
    return true;
}

qint64 AudioInput::writeData(const char* data, qint64 len) {
    unsigned char encodedData[4000];
    int encodedBytes = opus_encode(opusEncoder, reinterpret_cast<const opus_int16*>(data), opusFrameSize, encodedData, 960);

    if (encodedBytes < 0) {
        qWarning() << "Opus encoding failed:" << opus_strerror(encodedBytes);
        return -1;
    }
    // raw audio
    // QByteArray packet(data, len);
    QByteArray packet(reinterpret_cast<const char*>(encodedData), encodedBytes);
    emit newAudioData(packet);

    qDebug() << "Encoded" << len << "bytes of raw audio into" << encodedBytes << "bytes of Opus";

    return len;
}

qint64 AudioInput::readData(char* data, qint64 len) {
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}