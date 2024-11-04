#include "audioinput.h"

AudioInput::AudioInput(QObject* parent)
    : QIODevice(parent),
      audioSource(nullptr),
      opusEncoder(nullptr),
      opusFrameSize(960) {
    int error;
    opusEncoder = opus_encoder_create(48000, 1, OPUS_APPLICATION_VOIP, &error); // 48kHz, mono

    if (error != OPUS_OK) {
        qWarning() << "Failed to create Opus encoder:" << opus_strerror(error);
    }

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    audioSource = new QAudioSource(format, this);
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
        open(QIODevice::WriteOnly);
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
    const int maxPacketSize = 4000;
    QByteArray encodedData(maxPacketSize, 0);  // Initialize QByteArray with max size

    const opus_int16 *pcmData = reinterpret_cast<const opus_int16 *>(data);
    int encodedBytes = opus_encode(opusEncoder, pcmData, len / sizeof(opus_int16),
                                   reinterpret_cast<unsigned char *>(encodedData.data()),
                                   maxPacketSize);

    if (encodedBytes < 0) {
        qDebug() << "Opus encoding error:" << opus_strerror(encodedBytes);
        return -1;
    }

    encodedData.resize(encodedBytes);  // Resize to actual encoded data size
    qDebug() << "Encoded data size:" << encodedBytes;

    emit newAudioData(encodedData);

    return encodedBytes;  // Return the encoded data safely
}

qint64 AudioInput::readData(char* data, qint64 len) {
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}