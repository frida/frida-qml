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

    Q_INVOKABLE int count() const { return m_processes.size(); }
    Q_INVOKABLE Process *get(int index) const;
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
    void error(QString message);

private:
    void updateActiveDevice(Device *device);
    void enumerateProcesses();
    static void onEnumerateReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onEnumerateReady(GAsyncResult *res);

private slots:
    void updateItems(Device *device, QList<Process *> added, QSet<unsigned int> removed);
    void beginLoading();
    void endLoading();
    void onError(QString message);

private:
    Device *m_device;
    QList<Process *> m_processes;
    bool m_isLoading;

    Device *m_activeDevice;
    EnumerateProcessesRequest *m_pendingRequest;
    QSet<unsigned int> m_pids;

    MainContext m_mainContext;
};

#endif
