#ifndef FRIDAQML_DEVICE_MANAGER_H
#define FRIDAQML_DEVICE_MANAGER_H

#include <frida-core.h>

#include "maincontext.h"

#include <QObject>

class Device;
class Scripts;

class Frida : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Frida)
    Q_PROPERTY(QList<QObject *> devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(Device *local READ local NOTIFY localChanged)

public:
    explicit Frida(QObject *parent = 0);
private:
    void initialize();
    void dispose();
public:
    ~Frida();

    QList<QObject *> devices() const { return m_devices; }
    Device *local() const { return m_local; }

signals:
    void devicesChanged(QList<QObject *> newDevices);
    void localChanged(Device *newLocal);

private:
    static void onDeviceAdded(Frida *self, FridaDevice *deviceHandle);
    static void onDeviceRemoved(Frida *self, FridaDevice *deviceHandle);

private slots:
    void add(Device *device);
    void removeById(unsigned int id);

private:
    FridaDeviceManager *m_handle;
    QList<QObject *> m_devices;
    Device *m_local;
    MainContext m_mainContext;
};

#endif
