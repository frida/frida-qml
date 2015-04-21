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
    Q_PROPERTY(Device *localSystem READ localSystem CONSTANT)

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
    static void onEnumerateDevicesReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onEnumerateDevicesReady(GAsyncResult *res);
    static void onDeviceAddedWrapper(Frida *self, FridaDevice *deviceHandle);
    static void onDeviceRemovedWrapper(Frida *self, FridaDevice *deviceHandle);
    void onDeviceAdded(FridaDevice *deviceHandle);
    void onDeviceRemoved(FridaDevice *deviceHandle);

private slots:
    void add(Device *device);
    void removeById(unsigned int id);

private:
    FridaDeviceManager *m_handle;
    QList<Device *> m_deviceItems;
    Device *m_localSystem;
    MainContext *m_mainContext;
    GMutex m_mutex;
    GCond m_cond;

    static Frida *s_instance;
};

#endif
