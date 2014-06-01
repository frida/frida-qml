#include "device.h"

Device::Device(FridaDevice *handle, QObject *parent) :
    QObject(parent),
    m_handle(handle),
    m_id(frida_device_get_id(handle)),
    m_name(frida_device_get_name(handle)),
    m_type(static_cast<Device::Type>(frida_device_get_dtype(handle))),
    m_processes(new Processes(handle, this)),
    m_mainContext(frida_get_main_context())
{
    g_object_ref(m_handle);
    g_object_set_data(G_OBJECT(m_handle), "qdevice", this);
}

Device::~Device()
{
    m_mainContext.perform([this] () { dispose(); });
}

void Device::dispose()
{
    g_object_set_data(G_OBJECT(m_handle), "qdevice", NULL);
    g_object_unref(m_handle);
}
