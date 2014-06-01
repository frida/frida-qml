#include "device.h"

Device::Device(FridaDevice *handle, QObject *parent) :
    QObject(parent),
    m_handle(handle),
    m_id(frida_device_get_id(handle)),
    m_name(frida_device_get_name(handle)),
    m_type(static_cast<Device::Type>(frida_device_get_dtype(handle)))
{
    g_object_ref(m_handle);
}

Device::~Device()
{
    g_object_unref(m_handle);
}
