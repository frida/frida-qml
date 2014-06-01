#include "devicemanager.h"

#include "device.h"

DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent),
    m_mainContext(frida_get_main_context())
{
    frida_init();
    m_mainContext.schedule([this] () { initialize(); });
}

DeviceManager::~DeviceManager()
{
    m_mainContext.schedule([this] () { dispose(); });
    frida_deinit();
}

void DeviceManager::initialize()
{
    m_handle = frida_device_manager_new();
    g_signal_connect_swapped(m_handle, "added", G_CALLBACK(onDeviceAdded), this);
    g_signal_connect_swapped(m_handle, "removed", G_CALLBACK(onDeviceRemoved), this);
    frida_device_manager_enumerate_devices(m_handle, NULL, NULL);
}

void DeviceManager::dispose()
{
    g_signal_handlers_disconnect_by_func(m_handle, GSIZE_TO_POINTER(onDeviceAdded), this);
    g_object_unref(m_handle);
    m_handle = NULL;
}

void DeviceManager::onDeviceAdded(DeviceManager *self, FridaDevice *deviceHandle)
{
    auto device = new Device(deviceHandle);
    device->moveToThread(self->thread());
    QMetaObject::invokeMethod(self, "add", Qt::QueuedConnection, Q_ARG(Device *, device));
}

void DeviceManager::onDeviceRemoved(DeviceManager *self, FridaDevice *deviceHandle)
{
    QMetaObject::invokeMethod(self, "removeById", Qt::QueuedConnection, Q_ARG(unsigned int, frida_device_get_id(deviceHandle)));
}

void DeviceManager::add(Device *device)
{
    m_devices.append(device);
    emit devicesChanged(m_devices);
}

void DeviceManager::removeById(unsigned int id)
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
