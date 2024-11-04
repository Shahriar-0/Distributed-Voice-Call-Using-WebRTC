#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QTimer>

#include "client.h"

int main(int argc, char *argv[]) {
    QGuiApplication  app(argc, argv);

    app.setWindowIcon(QIcon(":/assets/icon.png"));
    
    QQmlApplicationEngine engine;

    DistributedLiveVoiceCall* voiceCall = new DistributedLiveVoiceCall(&engine);

    qmlRegisterSingletonInstance("voiceCall", 1, 0, "DistributedLiveVoiceCall", voiceCall);

    const QUrl url(QStringLiteral("qrc:/src/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) return -1;

    Client offerer(nullptr, "client1", true);
    Client answerer(nullptr, "client2", false);

    offerer.connectToServer("127.0.0.1", 9000);
    answerer.connectToServer("127.0.0.1", 9000);

    QTimer::singleShot(2000, [&] { offerer.offerCall("client2"); });
    
    
    return app.exec();
}
