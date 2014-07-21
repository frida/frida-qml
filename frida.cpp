#include "frida.h"

#include "device.h"
#include "devicelistmodel.h"

Frida *Frida::s_instance = nullptr;

Frida::Frida(QObject *parent) :
    QObject(parent),
    m_localSystem(nullptr),
    m_mainContext(nullptr)
{
    frida_init();

    g_mutex_init(&m_mutex);
    g_cond_init(&m_cond);

    m_mainContext = new MainContext(frida_get_main_context());
    m_mainContext->schedule([this] () { initialize(); });

    g_mutex_lock(&m_mutex);
    while (m_localSystem == nullptr)
        g_cond_wait(&m_cond, &m_mutex);
    g_mutex_unlock(&m_mutex);
}

void Frida::initialize()
{
    m_handle = frida_device_manager_new();
    g_signal_connect_swapped(m_handle, "added", G_CALLBACK(onDeviceAddedWrapper), this);
    g_signal_connect_swapped(m_handle, "removed", G_CALLBACK(onDeviceRemovedWrapper), this);
    frida_device_manager_enumerate_devices(m_handle, nullptr, nullptr);
}

void Frida::dispose()
{
    g_signal_handlers_disconnect_by_func(m_handle, GSIZE_TO_POINTER(onDeviceRemovedWrapper), this);
    g_signal_handlers_disconnect_by_func(m_handle, GSIZE_TO_POINTER(onDeviceAddedWrapper), this);
    g_object_unref(m_handle);
    m_handle = nullptr;
}

Frida::~Frida()
{
    m_localSystem = nullptr;
    foreach (Device *device, m_deviceItems)
        delete device;
    m_deviceItems.clear();

    frida_device_manager_close_sync(m_handle);
    m_mainContext->perform([this] () { dispose(); });
    delete m_mainContext;

    g_cond_clear(&m_cond);
    g_mutex_clear(&m_mutex);

    s_instance = nullptr;

    frida_deinit();
}

Frida *Frida::instance()
{
    if (s_instance == nullptr)
        s_instance = new Frida();
    return s_instance;
}

void Frida::onDeviceAddedWrapper(Frida *self, FridaDevice *deviceHandle)
{
    self->onDeviceAdded(deviceHandle);
}

void Frida::onDeviceRemovedWrapper(Frida *self, FridaDevice *deviceHandle)
{
    self->onDeviceRemoved(deviceHandle);
}

void Frida::onDeviceAdded(FridaDevice *deviceHandle)
{
    auto device = new Device(deviceHandle);
    device->moveToThread(this->thread());
    device->setParent(this);
    if (device->type() == Device::Local) {
        g_mutex_lock(&m_mutex);
        m_localSystem = device;
        g_cond_signal(&m_cond);
        g_mutex_unlock(&m_mutex);
    }
    QMetaObject::invokeMethod(this, "add", Qt::QueuedConnection, Q_ARG(Device *, device));
}

void Frida::onDeviceRemoved(FridaDevice *deviceHandle)
{
    QMetaObject::invokeMethod(this, "removeById", Qt::QueuedConnection, Q_ARG(unsigned int, frida_device_get_id(deviceHandle)));
}

void Frida::add(Device *device)
{
    m_deviceItems.append(device);
    emit deviceAdded(device);
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
