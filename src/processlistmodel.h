#ifndef FRIDAQML_PROCESSLISTMODEL_H
#define FRIDAQML_PROCESSLISTMODEL_H

#include "fridafwd.h"

#include <QAbstractListModel>
#include <QQmlEngine>

class Device;
class MainContext;
class Process;
struct EnumerateProcessesRequest;

class ProcessListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ProcessListModel)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(Device *device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    QML_ELEMENT

public:
    explicit ProcessListModel(QObject *parent = nullptr);
private:
    void dispose();
public:
    ~ProcessListModel();

    int count() const { return m_processes.size(); }
    Q_INVOKABLE Process *get(int index) const;
    Q_INVOKABLE void refresh();

    Device *device() const;
    void setDevice(Device *device);
    bool isLoading() const { return m_isLoading; }

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

Q_SIGNALS:
    void countChanged(int newCount);
    void deviceChanged(Device *newDevice);
    void isLoadingChanged(bool newIsLoading);
    void error(QString message);

private:
    void updateActiveDevice(FridaDevice *handle);
    void enumerateProcesses(FridaDevice *handle);
    static void onEnumerateReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onEnumerateReady(FridaDevice *handle, GAsyncResult *res);

    static int score(Process *process);

private Q_SLOTS:
    void updateItems(void *handle, QList<Process *> added, QSet<unsigned int> removed);
    void beginLoading();
    void endLoading();
    void onError(QString message);

private:
    QPointer<Device> m_device;
    QList<Process *> m_processes;
    bool m_isLoading;

    EnumerateProcessesRequest *m_pendingRequest;
    QSet<unsigned int> m_pids;

    QScopedPointer<MainContext> m_mainContext;
};

#endif
