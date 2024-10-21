#include "distributedlivevoicecall.h"

DistributedLiveVoiceCall::DistributedLiveVoiceCall(QObject* parent)
    : QObject{parent},
      m_callerID("") // Initialize the callerID to an empty string
{}

// Getter for callerID
QString DistributedLiveVoiceCall::callerID() const { return m_callerID; }

// Setter for callerID
void DistributedLiveVoiceCall::setCallerID(const QString& callerID) {
    if (m_callerID != callerID) {
        m_callerID = callerID;
        emit callerIDChanged(); // Emit signal when callerID changes
    }
}

void DistributedLiveVoiceCall::startCall(const QString& callerID) {
    // Set the caller ID and emit the callerIDChanged signal if it has changed
    setCallerID(callerID);

    // Debug print to ensure the method is called
    qDebug() << "Start call triggered with Caller ID:" << callerID;
}

void DistributedLiveVoiceCall::endCall() { qDebug() << "End call triggered!"; }
