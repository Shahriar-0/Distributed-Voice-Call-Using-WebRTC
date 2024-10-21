#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "distributedlivevoicecall.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<DistributedLiveVoiceCall>("VoiceCall", 1, 0, "DistributedLiveVoiceCall");

    const QUrl url(QStringLiteral("qrc:/src/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) return -1;

    return app.exec();
}
