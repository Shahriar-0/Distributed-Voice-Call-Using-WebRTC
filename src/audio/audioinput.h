#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <opus.h>

#include <QAudioFormat>
#include <QAudioSource>
#include <QDebug>
#include <QIODevice>
#include <QMediaDevices>
#include <QObject>

class AudioInput : public QIODevice {
    Q_OBJECT

public:
    explicit AudioInput(QObject* parent = nullptr);
    ~AudioInput();

    bool startAudio();
    bool stopAudio();

Q_SIGNALS:
    void newAudioData(const QByteArray& data);

protected:
    qint64 writeData(const char* data, qint64 len) override;
    qint64 readData(char* data, qint64 len) override;        // Unused but required by QIODevice

private:
    QAudioSource* audioSource;
    OpusEncoder* opusEncoder;
    int opusFrameSize;
};

#endif // AUDIOINPUT_HPP