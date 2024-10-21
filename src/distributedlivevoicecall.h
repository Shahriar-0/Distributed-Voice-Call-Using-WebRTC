// File: distributedlivevoicecall.h
#ifndef DISTRIBUTEDLIVEVOICECALL_H
#define DISTRIBUTEDLIVEVOICECALL_H

#include <QDebug>
#include <QObject>
#include <QAudioSource>
#include <QAudioSink>
#include <QBuffer>
#include <QTimer>

class DistributedLiveVoiceCall : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString callerID READ callerID WRITE setCallerID NOTIFY callerIDChanged)

public:
    explicit DistributedLiveVoiceCall(QObject* parent = nullptr);

    QString callerID() const;
    void setCallerID(const QString& callerID);

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

    // New audio methods
    void startAudioRecording();
    void stopAudioRecording();
    void playRecordedAudio();
};

#endif // DISTRIBUTEDLIVEVOICECALL_H
