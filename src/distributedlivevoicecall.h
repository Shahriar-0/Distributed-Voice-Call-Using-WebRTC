#ifndef DISTRIBUTEDLIVEVOICECALL_H
#define DISTRIBUTEDLIVEVOICECALL_H

#include <QObject>
#include "audioinput.h"
#include "audiooutput.h"

class DistributedLiveVoiceCall : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString callerID READ callerID WRITE setCallerID NOTIFY callerIDChanged)

public:
    explicit DistributedLiveVoiceCall(QObject* parent = nullptr);
    ~DistributedLiveVoiceCall();

    QString callerID() const { return m_callerID; }
    void setCallerID(const QString& callerID) {
        if (m_callerID != callerID) {
            m_callerID = callerID;
            emit callerIDChanged();
        }
    }

public Q_SLOTS:
    void startCall(const QString& callerID);
    void endCall();

Q_SIGNALS:
    void callerIDChanged();

private:
    QString m_callerID;

    AudioInput* audioInput;
    AudioOutput* audioOutput;

private Q_SLOTS:
    void handleNewAudioData(const QByteArray &data);
};

#endif // DISTRIBUTEDLIVEVOICECALL_H
