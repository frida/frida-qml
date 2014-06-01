#ifndef FRIDAQML_DEVICE_MANAGER_H
#define FRIDAQML_DEVICE_MANAGER_H

#include <frida-core.h>

#include "device.h"
#include "maincontext.h"

#include <QObject>

class DeviceManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceManager)
    Q_PROPERTY(QList<QObject *> devices READ devices NOTIFY devicesChanged)

public:
    explicit DeviceManager(QObject *parent = 0);
    ~DeviceManager();

    QList<QObject *> devices() const { return m_devices; }

signals:
    void devicesChanged(QList<QObject *> newDevices);

private:
    void initialize();
    void dispose();

    static void onDeviceAdded(DeviceManager *self, FridaDevice *deviceHandle);
    static void onDeviceRemoved(DeviceManager *self, FridaDevice *deviceHandle);

private slots:
    void add(Device *device);
    void removeById(unsigned int id);

private:
    FridaDeviceManager *m_handle;
    QList<QObject *> m_devices;
    MainContext m_mainContext;
};

#endif
