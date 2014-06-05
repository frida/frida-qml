#include "frida.h"

#include "device.h"

Frida::Frida(QObject *parent) :
    QObject(parent),
    m_local(0),
    m_mainContext(frida_get_main_context())
{
    frida_init();
    m_mainContext.schedule([this] () { initialize(); });
}

void Frida::initialize()
{
    m_handle = frida_device_manager_new();
    g_signal_connect_swapped(m_handle, "added", G_CALLBACK(onDeviceAdded), this);
    g_signal_connect_swapped(m_handle, "removed", G_CALLBACK(onDeviceRemoved), this);
    frida_device_manager_enumerate_devices(m_handle, NULL, NULL);
}

void Frida::dispose()
{
    g_signal_handlers_disconnect_by_func(m_handle, GSIZE_TO_POINTER(onDeviceAdded), this);
    g_object_unref(m_handle);
    m_handle = NULL;
}

Frida::~Frida()
{
    m_mainContext.schedule([this] () { dispose(); });
    frida_deinit();
}

void Frida::onDeviceAdded(Frida *self, FridaDevice *deviceHandle)
{
    auto device = new Device(deviceHandle);
    device->moveToThread(self->thread());
    QMetaObject::invokeMethod(self, "add", Qt::QueuedConnection, Q_ARG(Device *, device));
}

void Frida::onDeviceRemoved(Frida *self, FridaDevice *deviceHandle)
{
    QMetaObject::invokeMethod(self, "removeById", Qt::QueuedConnection, Q_ARG(unsigned int, frida_device_get_id(deviceHandle)));
}

void Frida::add(Device *device)
{
    m_devices.append(device);
    emit devicesChanged(m_devices);

    if (device->type() == Device::Local) {
        m_local = device;
        emit localChanged(m_local);
    }
}

void Frida::removeById(unsigned int id)
{
    for (int i = 0; i != m_devices.size(); i++) {
        auto device = reinterpret_cast<Device *>(m_devices.at(i));
        if (device->id() == id) {
            m_devices.removeAt(i);
            delete device;
            break;
        }
    }
}
