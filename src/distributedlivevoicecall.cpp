#include "distributedlivevoicecall.h"
#include <QDebug>

DistributedLiveVoiceCall::DistributedLiveVoiceCall(QObject* parent)
    : QObject(parent), audioInput(nullptr), audioOutput(nullptr) {

    audioInput = new AudioInput(this);
    audioOutput = new AudioOutput(this);

    connect(audioInput, &AudioInput::newAudioData, this, &DistributedLiveVoiceCall::handleNewAudioData);
}

DistributedLiveVoiceCall::~DistributedLiveVoiceCall() {
    delete audioInput;
    delete audioOutput;
}

void DistributedLiveVoiceCall::startCall(const QString& callerID) {
    setCallerID(callerID);
    qDebug() << "Start call with Caller ID:" << callerID;

    if (!audioInput->startAudio()) {
        qWarning() << "Failed to start audio input.";
    }
}

void DistributedLiveVoiceCall::endCall() {
    qDebug() << "End call triggered!";

    if (!audioInput->stopAudio()) {
        qWarning() << "Failed to stop audio input.";
    }

    qDebug() << "Audio recording stopped.";
}

void DistributedLiveVoiceCall::handleNewAudioData(const QByteArray &data) {
    audioOutput->addData(data);

    qDebug() << "Playing new Opus packet of size:" << data.size();
}
