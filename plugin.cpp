#include <frida-core.h>

#include "plugin.h"

#include "device.h"
#include "devicelistmodel.h"
#include "frida.h"
#include "iconprovider.h"
#include "process.h"
#include "processlistmodel.h"
#include "script.h"

#include <qqml.h>

static QObject *createFridaSingleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    return Frida::instance();
}

void Frida_QmlPlugin::registerTypes(const char *uri)
{
    qRegisterMetaType<Device *>("Device *");
    qRegisterMetaType<QList<Process *>>("QList<Process *>");
    qRegisterMetaType<QSet<unsigned int>>("QSet<unsigned int>");
    qRegisterMetaType<ScriptInstance::Status>("ScriptInstance::Status");

    // @uri Frida
    qmlRegisterSingletonType<Frida>(uri, 1, 0, "Frida", createFridaSingleton);
    qmlRegisterType<DeviceListModel>(uri, 1, 0, "DeviceListModel");
    qmlRegisterType<ProcessListModel>(uri, 1, 0, "ProcessListModel");
    qmlRegisterType<Script>(uri, 1, 0, "Script");

    qmlRegisterUncreatableType<Device>(uri, 1, 0, "Device", "Device objects cannot be instantiated from Qml");
    qmlRegisterUncreatableType<Process>(uri, 1, 0, "Process", "Process objects cannot be instantiated from Qml");
}

void Frida_QmlPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri);

    // Ensure Frida is initialized
    Frida::instance();

    engine->addImageProvider("frida", IconProvider::instance());
}
