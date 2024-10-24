#ifndef DISTRIBUTEDLIVEVOICECALL_H
#define DISTRIBUTEDLIVEVOICECALL_H

#include <QAudioSink>
#include <QAudioSource>
#include <QBuffer>
#include <QDebug>
#include <QObject>
#include <QTimer>
#include <opus.h>
#include <QByteArray>
#include <QList>

class DistributedLiveVoiceCall : public QObject {
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

    QAudioSource* audioSource;
    QAudioSink* audioSink;
    QIODevice* audioInputDevice;
    QBuffer audioBuffer;
    QTimer playbackTimer;

    OpusEncoder* opusEncoder;
    OpusDecoder* opusDecoder;
    int frameSize;

    // Store encoded Opus packets
    QList<QByteArray> opusPacketList;

    // Buffer for storing the final decoded audio stream
    QByteArray decodedAudioBuffer;

    // Audio handling methods
    void startAudioRecording();
    void stopAudioRecording();
    void encodeAndStoreAudio(const QByteArray& audioData);
    void playRecordedAudio();
    void decodeAndAccumulateAudio(const QByteArray& encodedOpusData);
};

#endif // DISTRIBUTEDLIVEVOICECALL_H
