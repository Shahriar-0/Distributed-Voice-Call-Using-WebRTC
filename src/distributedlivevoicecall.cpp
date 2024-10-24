#include "distributedlivevoicecall.h"
#include <QDebug>

DistributedLiveVoiceCall::DistributedLiveVoiceCall(QObject* parent)
    : QObject(parent), audioInput(nullptr), audioOutput(nullptr) {

    // Initialize audio input and output objects
    audioInput = new AudioInput(this);
    audioOutput = new AudioOutput(this);

    // Connect signal from AudioInput to process and immediately play encoded audio data
    connect(audioInput, &AudioInput::newAudioData, this, &DistributedLiveVoiceCall::handleNewAudioData);
}

DistributedLiveVoiceCall::~DistributedLiveVoiceCall() {
    delete audioInput;
    delete audioOutput;
}

void DistributedLiveVoiceCall::startCall(const QString& callerID) {
    setCallerID(callerID);
    qDebug() << "Start call with Caller ID:" << callerID;

    // Start audio recording and encoding
    if (!audioInput->startAudio()) {
        qWarning() << "Failed to start audio input.";
    }
}

void DistributedLiveVoiceCall::endCall() {
    qDebug() << "End call triggered!";

    // Stop audio recording
    if (!audioInput->stopAudio()) {
        qWarning() << "Failed to stop audio input.";
    }

    qDebug() << "Audio recording stopped.";
}

void DistributedLiveVoiceCall::handleNewAudioData(const QByteArray &data) {
    // Immediately decode and play the received Opus-encoded audio packet
    audioOutput->addData(data);

    qDebug() << "Playing new Opus packet of size:" << data.size();
}
