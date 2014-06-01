#include <frida-core.h>

#include "plugin.h"

#include "devicemanager.h"
#include "device.h"

#include <qqml.h>

static QObject *createDeviceManagerSingleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    return new DeviceManager(0);
}

void Frida_QmlPlugin::registerTypes(const char *uri)
{
    qRegisterMetaType<Device *>("Device *");

    // @uri Frida
    qmlRegisterSingletonType<DeviceManager>(uri, 1, 0, "DeviceManager", createDeviceManagerSingleton);
    // @uri Frida
    qmlRegisterUncreatableType<Device>(uri, 1, 0, "Device", "Device objects cannot be instantiated from Qml");
}
