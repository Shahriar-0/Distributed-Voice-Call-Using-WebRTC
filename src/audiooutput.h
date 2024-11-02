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
    QAudioFormat audioFormat;
    QAudioSink* audioSink;
    QIODevice* audioDevice;
    QQueue<QByteArray> audioQueue;
    QMutex mutex;

    OpusDecoder* opusDecoder;
    int opusFrameSize;
};

#endif // AUDIOOUTPUT_H
