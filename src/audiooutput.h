#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <opus.h>

#include <QAudioFormat>
#include <QAudioSink>
#include <QByteArray>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QQueue>

class AudioOutput : public QObject {
    Q_OBJECT

public:
    explicit AudioOutput(QObject* parent = nullptr);
    ~AudioOutput();

    void addData(const QByteArray& data);

Q_SIGNALS:
    void newPacket();

private Q_SLOTS:
    void play();

private:
    QAudioFormat audioFormat; // Audio format settings
    QAudioSink* audioSink;    // Audio sink (output)
    QIODevice* audioDevice;   // Audio device for playback
    OpusDecoder* opusDecoder; // Opus decoder instance
    int opusFrameSize;        // Frame size for decoding

    QQueue<QByteArray> audioQueue; // Queue to hold encoded audio data
    QMutex mutex;                  // Mutex for thread safety when accessing the queue
};

#endif // AUDIOOUTPUT_H
