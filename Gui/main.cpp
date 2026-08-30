#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QUrl>

#include "../Engine/config/Config.hpp"
#include "Bridge.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("Artha Bazaar");
    app.setOrganizationName("TMS");

    // Same .env-based configuration the console/engine code uses
    // (API_KEY for live market data fetches).
    Config::load(".env");

    Bridge engine;
    if (!engine.initialize()) {
        // Non-fatal: the UI will show whatever data is available (or a
        // login error) and the user can still use the app.
    }

    QQmlApplicationEngine qmlEngine;
    qmlEngine.rootContext()->setContextProperty("engine", &engine);

    // loadFromModule() requires Qt 6.5+; load the module's compiled QML URL
    // directly instead so this also builds against Qt 6.4. The resource
    // prefix qt_add_qml_module() generates for a given URI can vary by Qt
    // version (older Qt: qrc:/<uri-path>/, newer Qt: qrc:/qt/qml/<uri-path>/),
    // so try both rather than hard-coding one.
    const QStringList candidates = {
        QStringLiteral("qrc:/qt/qml/com/arthabazar/app/Main.qml"),
        QStringLiteral("qrc:/com/arthabazar/app/Main.qml"),
    };
    for (const QString& candidate : candidates) {
        qmlEngine.load(QUrl(candidate));
        if (!qmlEngine.rootObjects().isEmpty()) {
            break;
        }
    }
    if (qmlEngine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
