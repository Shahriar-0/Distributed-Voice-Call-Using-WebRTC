#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QTimer>

#include "distributedlivevoicecall.h"
#include "client.h"
#include "signalingserver.h"
#include <windows.h>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // app.setWindowIcon(QIcon(":/assets/icon.png"));
    
    // QQmlApplicationEngine engine;

    // DistributedLiveVoiceCall* voiceCall = new DistributedLiveVoiceCall(&engine);

    // qmlRegisterSingletonInstance("voiceCall", 1, 0, "DistributedLiveVoiceCall", voiceCall);

    // const QUrl url(QStringLiteral("qrc:/src/main.qml"));
    // QObject::connect(
    //     &engine, &QQmlApplicationEngine::objectCreated, &app,
    //     [url](QObject *obj, const QUrl &objUrl) {
    //         if (!obj && url == objUrl) QCoreApplication::exit(-1);
    //     },
    //     Qt::QueuedConnection);

    // engine.load(url);

    // if (engine.rootObjects().isEmpty()) return -1;

    Client client1(nullptr, "client1");
    Client client2(nullptr, "client2");
    SignalingServer server;
    if (!server.startServer(QHostAddress("127.0.0.1"), 9000)) {
        qDebug() << "Failed to start server!";
        return -1;
    }


    // client1.sendSdpToClient("sdddddp", "client2");
    client1.connectToServer("127.0.0.1", 9000);
    client2.connectToServer("127.0.0.1", 9000);



    // while(1) {
    //     if((client1.socket->state() == QAbstractSocket::ConnectedState)
    //         && (client2.socket->state() == QAbstractSocket::ConnectedState))

    //         break;   
    // }

    QTimer::singleShot(3000, [&] { client1.sendSdpToClient("sdddp", "client2"); });
    qDebug() << "Here";
    

    
    return app.exec();
}
