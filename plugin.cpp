#include <frida-core.h>

#include "plugin.h"

#include "device.h"
#include "frida.h"
#include "process.h"
#include "processes.h"
#include "script.h"

#include <qqml.h>

static QObject *createFridaSingleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    return new Frida(0);
}

void Frida_QmlPlugin::registerTypes(const char *uri)
{
    qRegisterMetaType<Device *>("Device *");
    qRegisterMetaType<Script *>("Script *");
    qRegisterMetaType<Script::Status>("Script::Status");
    qRegisterMetaType<QList<Process *>>("QList<Process *>");
    qRegisterMetaType<QSet<unsigned int>>("QSet<unsigned int>");

    // @uri Frida
    qmlRegisterSingletonType<Frida>(uri, 1, 0, "Frida", createFridaSingleton);
    qmlRegisterUncreatableType<Device>(uri, 1, 0, "Device", "Device objects cannot be instantiated from Qml");
    qmlRegisterUncreatableType<Processes>(uri, 1, 0, "Processes", "Processes objects cannot be instantiated from Qml");
    qmlRegisterUncreatableType<Process>(uri, 1, 0, "Process", "Process objects cannot be instantiated from Qml");
    qmlRegisterType<Script>(uri, 1, 0, "Script");
}
