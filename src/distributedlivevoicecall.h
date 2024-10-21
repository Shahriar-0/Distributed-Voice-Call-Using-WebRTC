#ifndef DISTRIBUTEDLIVEVOICECALL_H
#define DISTRIBUTEDLIVEVOICECALL_H

#include <QObject>
#include <QDebug>

class DistributedLiveVoiceCall : public QObject {
    Q_OBJECT
public:
    explicit DistributedLiveVoiceCall(QObject *parent = nullptr);

public slots:
    void startCall(const QString &callerID);
    void endCall();

private:
};

#endif // DISTRIBUTEDLIVEVOICECALL_H
