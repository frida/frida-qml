#include "frida.h"

#include "device.h"
#include "devicelistmodel.h"

Frida *Frida::s_instance = nullptr;

Frida::Frida(QObject *parent) :
    QObject(parent),
    m_localSystem(nullptr),
    m_mainContext(frida_get_main_context())
{
    frida_init();
    s_instance = this;
    m_mainContext.schedule([this] () { initialize(); });
}

void Frida::initialize()
{
    m_handle = frida_device_manager_new();
    g_signal_connect_swapped(m_handle, "added", G_CALLBACK(onDeviceAdded), this);
    g_signal_connect_swapped(m_handle, "removed", G_CALLBACK(onDeviceRemoved), this);
    frida_device_manager_enumerate_devices(m_handle, nullptr, nullptr);
}

void Frida::dispose()
{
    g_signal_handlers_disconnect_by_func(m_handle, GSIZE_TO_POINTER(onDeviceAdded), this);
    g_object_unref(m_handle);
    m_handle = nullptr;
}

Frida::~Frida()
{
    frida_device_manager_close_sync(m_handle);
    m_mainContext.perform([this] () { dispose(); });
    s_instance = nullptr;
    frida_deinit();
}

Frida *Frida::instance()
{
    if (s_instance == nullptr)
        s_instance = new Frida();
    return s_instance;
}

void Frida::onDeviceAdded(Frida *self, FridaDevice *deviceHandle)
{
    auto device = new Device(deviceHandle);
    device->moveToThread(self->thread());
    device->setParent(self);
    QMetaObject::invokeMethod(self, "add", Qt::QueuedConnection, Q_ARG(Device *, device));
}

void Frida::onDeviceRemoved(Frida *self, FridaDevice *deviceHandle)
{
    QMetaObject::invokeMethod(self, "removeById", Qt::QueuedConnection, Q_ARG(unsigned int, frida_device_get_id(deviceHandle)));
}

void Frida::add(Device *device)
{
    m_deviceItems.append(device);
    emit deviceAdded(device);

    if (device->type() == Device::Local) {
        m_localSystem = device;
        emit localSystemChanged(m_localSystem);
    }
}

void Frida::removeById(unsigned int id)
{
    for (int i = 0; i != m_deviceItems.size(); i++) {
        auto device = m_deviceItems.at(i);
        if (device->id() == id) {
            m_deviceItems.removeAt(i);
            emit deviceRemoved(device);
            delete device;
            break;
        }
    }
}
