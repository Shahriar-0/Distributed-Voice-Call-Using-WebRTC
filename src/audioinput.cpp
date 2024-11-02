#include "audioinput.h"

AudioInput::AudioInput(QObject* parent)
    : QIODevice(parent),
      audioSource(nullptr),
      opusEncoder(nullptr),
      opusFrameSize(960) {
    int error;
    opusEncoder = opus_encoder_create(48000, 1, OPUS_APPLICATION_VOIP, &error); // 48kHz
    opus_encoder_ctl(opusEncoder, OPUS_SET_BITRATE(64000));  // Set bitrate to 64 kbps
    opus_encoder_ctl(opusEncoder, OPUS_SET_COMPLEXITY(10));  // Keep maximum complexity for better quality
    opus_encoder_ctl(opusEncoder, OPUS_SET_INBAND_FEC(1));   // Enable Forward Error Correction (FEC)
    opus_encoder_ctl(opusEncoder, OPUS_SET_DTX(0));          // Disable DTX to avoid noise during silence

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice info = QMediaDevices::defaultAudioInput();
    audioSource = new QAudioSource(info, format, this);
}

AudioInput::~AudioInput() {
    opus_encoder_destroy(opusEncoder);
}

bool AudioInput::startAudio() {
    if (!audioSource)
        return false;
    if (!isOpen())
        open(QIODevice::ReadWrite);
    audioSource->start(this);
    return true;
}

bool AudioInput::stopAudio() {
    if (!audioSource)
        return false;
    if (!isOpen())
        open(QIODevice::ReadWrite);
    audioSource->stop();
    return true;
}

qint64 AudioInput::writeData(const char* data, qint64 len) {
    unsigned char encodedData[4000];
    int encodedBytes = opus_encode(opusEncoder, reinterpret_cast<const opus_int16*>(data), opusFrameSize, encodedData, 960);
    QByteArray packet(reinterpret_cast<const char*>(encodedData), encodedBytes);
    emit newAudioData(packet);
    return len;
}

qint64 AudioInput::readData(char* data, qint64 len) {
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}
