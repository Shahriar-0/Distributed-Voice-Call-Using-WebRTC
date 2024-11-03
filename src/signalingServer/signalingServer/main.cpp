#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QTimer>

#include "signalingserver.h"

int main(int argc, char *argv[]) {
    QCoreApplication  app(argc, argv);

    SignalingServer server;
    if (!server.startServer(QHostAddress("127.0.0.1"), 9000)) {
        qDebug() << "Failed to start server!";
        return -1;
    }
    return app.exec();
}
