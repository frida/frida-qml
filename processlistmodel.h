#ifndef FRIDAQML_PROCESSLISTMODEL_H
#define FRIDAQML_PROCESSLISTMODEL_H

#include <frida-core.h>

#include "maincontext.h"

#include <QAbstractListModel>
#include <QList>
#include <QSet>

class Device;
class Process;

typedef struct _EnumerateProcessesRequest EnumerateProcessesRequest;

class ProcessListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ProcessListModel)
    Q_PROPERTY(Device *device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:
    explicit ProcessListModel(QObject *parent = nullptr);
private:
    void dispose();
public:
    ~ProcessListModel();

    Q_INVOKABLE void refresh();

    Device *device() const { return m_device; }
    void setDevice(Device *device);
    bool isLoading() const { return m_isLoading; }

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

signals:
    void deviceChanged(Device *newDevice);
    void isLoadingChanged(bool newIsLoading);

private:
    void updateDeviceHandle(FridaDevice *deviceHandle);
    void enumerateProcesses();
    static void onEnumerateReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onEnumerateReady(GAsyncResult *res);

private slots:
    void updateItems(FridaDevice *deviceHandle, QList<Process *> added, QSet<unsigned int> removed);
    void beginLoading();
    void endLoading();

private:
    Device *m_device;
    QList<Process *> m_processes;
    bool m_isLoading;

    FridaDevice *m_deviceHandle;
    EnumerateProcessesRequest *m_pendingRequest;
    QSet<unsigned int> m_pids;

    MainContext m_mainContext;
};

#endif
