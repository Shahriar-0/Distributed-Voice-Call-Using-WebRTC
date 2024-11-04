#include "audiooutput.h"

AudioOutput::AudioOutput(QObject* parent)
    : QObject(parent),
      audioDevice(nullptr),
      audioSink(nullptr),
      opusDecoder(nullptr),
      opusFrameSize(960) {
    audioFormat.setSampleRate(48000);
    audioFormat.setChannelCount(1);
    audioFormat.setSampleFormat(QAudioFormat::Int16);

    int error;
    opusDecoder = opus_decoder_create(48000, 1, &error);
    if (error != OPUS_OK) {
        qWarning() << "Failed to create Opus decoder:" << opus_strerror(error);
    }

    audioSink = new QAudioSink(audioFormat, this);
    audioDevice = audioSink->start();

    connect(this, &AudioOutput::newPacket, this, &AudioOutput::play);
}

AudioOutput::~AudioOutput() {
    opus_decoder_destroy(opusDecoder);
}

void AudioOutput::addData(const QByteArray& data) {
    QMutexLocker locker(&mutex);
    audioQueue.enqueue(data);
    emit newPacket();
}

void AudioOutput::play() {
    if (audioQueue.isEmpty())
        return;

    QByteArray encodedData = audioQueue.dequeue();
    opus_int16 pcmData[960];
    int decodedSamples = opus_decode(opusDecoder, reinterpret_cast<const unsigned char *>(encodedData.data()),
                                        encodedData.size(), pcmData, 960, 0);

    if (decodedSamples < 0) {
        qDebug() << "Opus decoding error:" << opus_strerror(decodedSamples);
    }

    if (audioDevice) {
        audioDevice->write(reinterpret_cast<const char *>(pcmData), decodedSamples * sizeof(opus_int16));
    }
}