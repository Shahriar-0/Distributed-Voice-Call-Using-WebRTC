#include "distributedlivevoicecall.h"

DistributedLiveVoiceCall::DistributedLiveVoiceCall(QObject *parent)
    : QObject{parent}
{}

void DistributedLiveVoiceCall::startCall(const QString &callerID)
{
    // emit signal
    // emit signalStartCall(callerID);

    // Debug print to ensure the method is called
    qDebug() << "Start call triggered with Caller ID:" << callerID;
}

void DistributedLiveVoiceCall::endCall() {
    qDebug() << "End call triggered!";
}
