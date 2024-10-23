#include "distributedlivevoicecall.h"

#include <QAudioFormat>
#include <QByteArray>
#include <QMediaDevices>

DistributedLiveVoiceCall::DistributedLiveVoiceCall(QObject* parent)
    : QObject(parent), audioSource(nullptr), audioSink(nullptr) {
    playbackTimer.setSingleShot(true);
    connect(&playbackTimer, &QTimer::timeout, this, &DistributedLiveVoiceCall::playRecordedAudio);

    int sampleRate = 48000;
    int channels = 1;

    int opusEncError, opusDecError;
}

DistributedLiveVoiceCall::~DistributedLiveVoiceCall() {
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
        delete audioSource; // Clean up old audioSource
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
