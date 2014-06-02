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
    Q_PROPERTY(Scripts *scripts READ scripts NOTIFY scriptsChanged)

public:
    explicit Frida(QObject *parent = 0);
private:
    void initialize();
    void dispose();
public:
    ~Frida();

    QList<QObject *> devices() const { return m_devices; }
    Device *local() const { return m_local; }
    Scripts *scripts() const { return m_scripts; }

signals:
    void devicesChanged(QList<QObject *> newDevices);
    void localChanged(Device *newLocal);
    void scriptsChanged(Scripts *newScripts);

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
    Scripts *m_scripts;
    MainContext m_mainContext;
};

#endif
