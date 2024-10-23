#ifndef DISTRIBUTEDLIVEVOICECALL_H
#define DISTRIBUTEDLIVEVOICECALL_H

#include <QAudioSink>
#include <QAudioSource>
#include <QBuffer>
#include <QDebug>
#include <QObject>
#include <QTimer>

class DistributedLiveVoiceCall : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString callerID READ callerID WRITE setCallerID NOTIFY callerIDChanged)

public:
    explicit DistributedLiveVoiceCall(QObject* parent = nullptr);
    ~DistributedLiveVoiceCall(); // Destructor to clean up Opus objects

    QString callerID() const { return m_callerID; }
    void setCallerID(const QString& callerID) {
        if (m_callerID != callerID) {
            m_callerID = callerID;
            emit callerIDChanged();
        }
    }

public slots:
    void startCall(const QString& callerID);
    void endCall();

signals:
    void callerIDChanged();

private:
    QString m_callerID;

    // Audio members
    QAudioSource* audioSource;
    QAudioSink* audioSink;
    QBuffer audioBuffer;
    QTimer playbackTimer;

    // Audio handling methods
    void startAudioRecording();
    void stopAudioRecording();
    void playRecordedAudio();
};

#endif // DISTRIBUTEDLIVEVOICECALL_H
