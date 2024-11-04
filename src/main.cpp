#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QTimer>

#include "web/client.h"

int main(int argc, char *argv[]) {
    QGuiApplication  app(argc, argv);

    app.setWindowIcon(QIcon(":/assets/icon.png"));

    QQmlApplicationEngine engine;

    QString id = "client1";

    Client* client = new Client(&engine, id);

    qmlRegisterSingletonInstance("Client", 1, 0, "Client", client);

    const QUrl url(QStringLiteral("qrc:/src/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) return -1;


    client->connectToServer("127.0.0.1", 9000);

    return app.exec();
}
