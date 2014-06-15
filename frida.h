#ifndef FRIDAQML_FRIDA_H
#define FRIDAQML_FRIDA_H

#include <frida-core.h>

#include "maincontext.h"

#include <QObject>

class Device;
class Scripts;

class Frida : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Frida)
    Q_PROPERTY(Device *localSystem READ localSystem NOTIFY localSystemChanged)

public:
    explicit Frida(QObject *parent = nullptr);
private:
    void initialize();
    void dispose();
public:
    ~Frida();

    static Frida *instance();

    Device *localSystem() const { return m_localSystem; }

    QList<Device *> deviceItems() const { return m_deviceItems; }

signals:
    void localSystemChanged(Device *newLocalSystem);
    void deviceAdded(Device *device);
    void deviceRemoved(Device *device);

private:
    static void onDeviceAdded(Frida *self, FridaDevice *deviceHandle);
    static void onDeviceRemoved(Frida *self, FridaDevice *deviceHandle);

private slots:
    void add(Device *device);
    void removeById(unsigned int id);

private:
    FridaDeviceManager *m_handle;
    QList<Device *> m_deviceItems;
    Device *m_localSystem;
    MainContext m_mainContext;

    static Frida *s_instance;
};

#endif
