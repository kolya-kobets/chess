TEMPLATE = app

QT += qml quick

SOURCES += main.cpp \
    chessfieldmodel.cpp \
    chessboard.cpp

RESOURCES += qml.qrc

QMAKE_CXXFLAGS += -std=c++11

RCC_DIR =
# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    chessfieldmodel.h \
    chessboard.h


