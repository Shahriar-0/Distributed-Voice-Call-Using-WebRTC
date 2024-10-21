QT += quick

SOURCES += \
        distributedlivevoicecall.cpp \
        main.cpp \
        webrtc.cpp

resources.files = main.qml
resources.prefix = /$${TARGET}
RESOURCES += resources \
    resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    distributedlivevoicecall.h \
    webrtc.h

# libdatachannel.dll
INCLUDEPATH += D:/DEATH/libdatachannel/include
LIBS += -LD:/DEATH/libdatachannel/Windows/Mingw64 -ldatachannel.dll

# opus
INCLUDEPATH += D:/DEATH/opus/include
LIBS += -LD:/DEATH/opus/Windows/Mingw64 -lopus

# qt
LIBS += -LC:/Qt/Tools/OpenSSLv3/Win_x64/bin -lcrypto-3-x64 -lssl-3-x64
LIBS += -lws2_32
LIBS += -lssp

# # SocketIO Headers
# HEADERS += \
# $$PWD/SocketIO/sio_client.h \
# $$PWD/SocketIO/sio_message.h \
# $$PWD/SocketIO/sio_socket.h \
# $$PWD/SocketIO/internal/sio_client_impl.h \
# $$PWD/SocketIO/internal/sio_packet.h
#
# # SocketIO Sources
# SOURCES += \
# $$PWD/SocketIO/sio_client.cpp \
# $$PWD/SocketIO/sio_socket.cpp \
# $$PWD/SocketIO/internal/sio_client_impl.cpp \
# $$PWD/SocketIO/internal/sio_packet.cpp
#
# # Include paths for SocketIO and its dependencies
# INCLUDEPATH += $$PWD/SocketIO/lib/websocketpp
# INCLUDEPATH += $$PWD/SocketIO/lib/asio/asio/include
# INCLUDEPATH += $$PWD/SocketIO/lib/rapidjson/include
#
# # Defines for WebSocket++
# DEFINES += _WEBSOCKETPP_CPP11_STL_
# DEFINES += _WEBSOCKETPP_CPP11_FUNCTIONAL_
# DEFINES += SIO_TLS
#
# # Include paths for boost
# INCLUDEPATH += D:/DEATH/boost_1_82_0
