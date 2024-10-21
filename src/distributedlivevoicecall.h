#ifndef DISTRIBUTEDLIVEVOICECALL_H
#define DISTRIBUTEDLIVEVOICECALL_H

#include <QObject>
#include <QDebug>

class DistributedLiveVoiceCall : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString callerID READ callerID WRITE setCallerID NOTIFY callerIDChanged)

public:
    explicit DistributedLiveVoiceCall(QObject *parent = nullptr);

    QString callerID() const;
    void setCallerID(const QString &callerID);

public slots:
    void startCall(const QString &callerID);
    void endCall();

signals:
    void callerIDChanged();

private:
    QString m_callerID;
};

#endif // DISTRIBUTEDLIVEVOICECALL_H
