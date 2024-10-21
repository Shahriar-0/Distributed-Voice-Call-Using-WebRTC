#include "distributedlivevoicecall.h"
#include <QAudioFormat>
#include <QMediaDevices>

DistributedLiveVoiceCall::DistributedLiveVoiceCall(QObject* parent)
    : QObject(parent), audioSource(nullptr), audioSink(nullptr) {
    playbackTimer.setSingleShot(true);
    connect(&playbackTimer, &QTimer::timeout, this, &DistributedLiveVoiceCall::playRecordedAudio);
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

    // Start the playback timer to play audio after 3 seconds
    playbackTimer.start(3000);
}

void DistributedLiveVoiceCall::startAudioRecording() {
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    audioSource = new QAudioSource(QMediaDevices::defaultAudioInput(), format, this);
    audioBuffer.setBuffer(new QByteArray());
    audioBuffer.open(QIODevice::WriteOnly);

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
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format, this);

    audioBuffer.open(QIODevice::ReadOnly);
    audioSink->start(&audioBuffer);

    qDebug() << "Playing recorded audio. Data size:" << audioBuffer.data().size() << "bytes";
}


QString DistributedLiveVoiceCall::callerID() const { return m_callerID; }

void DistributedLiveVoiceCall::setCallerID(const QString& callerID) {
    if (m_callerID != callerID) {
        m_callerID = callerID;
        emit callerIDChanged();
    }
}
