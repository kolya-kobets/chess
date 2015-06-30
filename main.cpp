#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "chessfieldmodel.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    ChessFieldModel chess_model;

    QQmlApplicationEngine engine;
    QQmlContext *ctxt = engine.rootContext();
    ctxt->setContextProperty("chess_board_model", &chess_model);

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}
