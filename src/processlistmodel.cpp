#include <frida-core.h>

#include "processlistmodel.h"

#include "device.h"
#include "maincontext.h"
#include "process.h"

#include <QMetaMethod>

static const int ProcessPidRole = Qt::UserRole + 0;
static const int ProcessNameRole = Qt::UserRole + 1;
static const int ProcessSmallIconRole = Qt::UserRole + 2;
static const int ProcessLargeIconRole = Qt::UserRole + 3;

struct EnumerateProcessesRequest
{
    ProcessListModel *model;
    FridaDevice *handle;
};

ProcessListModel::ProcessListModel(QObject *parent) :
    QAbstractListModel(parent),
    m_isLoading(false),
    m_pendingRequest(nullptr),
    m_mainContext(new MainContext(frida_get_main_context()))
{
}

void ProcessListModel::dispose()
{
    if (m_pendingRequest != nullptr) {
        m_pendingRequest->model = nullptr;
        m_pendingRequest = nullptr;
    }
}

ProcessListModel::~ProcessListModel()
{
    m_mainContext->perform([this] () { dispose(); });
}

Process *ProcessListModel::get(int index) const
{
    if (index < 0 || index >= m_processes.size())
        return nullptr;

    return m_processes[index];
}

void ProcessListModel::refresh()
{
    if (m_device.isNull())
        return;

    auto handle = m_device->handle();
    g_object_ref(handle);
    m_mainContext->schedule([this, handle] () { enumerateProcesses(handle); });
}

Device *ProcessListModel::device() const
{
    return m_device;
}

void ProcessListModel::setDevice(Device *device)
{
    if (device == m_device)
        return;

    m_device = device;
    emit deviceChanged(device);

    FridaDevice *handle = nullptr;
    if (device != nullptr) {
        handle = device->handle();
        g_object_ref(handle);
    }
    m_mainContext->schedule([=] () { updateActiveDevice(handle); });

    if (!m_processes.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_processes.size() - 1);
        foreach (Process *process, m_processes)
            delete process;
        m_processes.clear();
        endRemoveRows();
    }
}

QHash<int, QByteArray> ProcessListModel::roleNames() const
{
    QHash<int, QByteArray> r;
    r[Qt::DisplayRole] = "display";
    r[ProcessPidRole] = "pid";
    r[ProcessNameRole] = "name";
    r[ProcessSmallIconRole] = "smallIcon";
    r[ProcessLargeIconRole] = "largeIcon";
    return r;
}

int ProcessListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_processes.size();
}

QVariant ProcessListModel::data(const QModelIndex &index, int role) const
{
    auto process = m_processes[index.row()];
    switch (role) {
    case ProcessPidRole:
        return QVariant(process->pid());
    case Qt::DisplayRole:
    case ProcessNameRole:
        return QVariant(process->name());
    case ProcessSmallIconRole:
        return QVariant(process->smallIcon());
    case ProcessLargeIconRole:
        return QVariant(process->largeIcon());
    default:
        return QVariant();
    }
}

void ProcessListModel::updateActiveDevice(FridaDevice *handle)
{
    m_pids.clear();

    if (handle != nullptr)
        enumerateProcesses(handle);
}

void ProcessListModel::enumerateProcesses(FridaDevice *handle)
{
    QMetaObject::invokeMethod(this, "beginLoading", Qt::QueuedConnection);

    if (m_pendingRequest != nullptr)
        m_pendingRequest->model = nullptr;
    auto request = g_slice_new(EnumerateProcessesRequest);
    request->model = this;
    request->handle = handle;
    m_pendingRequest = request;
    frida_device_enumerate_processes(handle, nullptr, onEnumerateReadyWrapper, request);
}

void ProcessListModel::onEnumerateReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data)
{
    Q_UNUSED(obj);

    auto request = static_cast<EnumerateProcessesRequest *>(data);
    if (request->model != nullptr)
        request->model->onEnumerateReady(request->handle, res);
    g_object_unref(request->handle);
    g_slice_free(EnumerateProcessesRequest, request);
}

void ProcessListModel::onEnumerateReady(FridaDevice *handle, GAsyncResult *res)
{
    m_pendingRequest = nullptr;

    QMetaObject::invokeMethod(this, "endLoading", Qt::QueuedConnection);

    GError *error = nullptr;
    auto processHandles = frida_device_enumerate_processes_finish(handle, res, &error);
    if (error == nullptr) {
        QSet<unsigned int> current;
        QList<Process *> added;
        QSet<unsigned int> removed;

        const int size = frida_process_list_size(processHandles);
        for (int i = 0; i != size; i++) {
            auto processHandle = frida_process_list_get(processHandles, i);
            auto pid = frida_process_get_pid(processHandle);
            current.insert(pid);
            if (!m_pids.contains(pid)) {
                auto process = new Process(processHandle);
                process->moveToThread(this->thread());
                added.append(process);
                m_pids.insert(pid);
            }
            g_object_unref(processHandle);
        }

        foreach (unsigned int pid, m_pids) {
            if (!current.contains(pid)) {
                removed.insert(pid);
            }
        }

        foreach (unsigned int pid, removed) {
            m_pids.remove(pid);
        }

        g_object_unref(processHandles);

        if (!added.isEmpty() || !removed.isEmpty()) {
            g_object_ref(handle);
            QMetaObject::invokeMethod(this, "updateItems", Qt::QueuedConnection,
                Q_ARG(void *, handle),
                Q_ARG(QList<Process *>, added),
                Q_ARG(QSet<unsigned int>, removed));
        }
    } else {
        auto message = QString("Failed to enumerate processes: ").append(QString::fromUtf8(error->message));
        QMetaObject::invokeMethod(this, "onError", Qt::QueuedConnection,
            Q_ARG(QString, message));
        g_clear_error(&error);
    }
}

int ProcessListModel::score(Process *process)
{
    return process->hasIcons() ? 1 : 0;
}

void ProcessListModel::updateItems(void *handle, QList<Process *> added, QSet<unsigned int> removed)
{
    foreach (Process *process, added) {
        process->setParent(this);
    }

    g_object_unref(handle);

    if (m_device.isNull() || handle != m_device->handle())
        return;

    QModelIndex parentRow;

    foreach (unsigned int pid, removed) {
        auto size = m_processes.size();
        for (int i = 0; i != size; i++) {
            auto process = m_processes[i];
            if (process->pid() == pid) {
                beginRemoveRows(parentRow, i, i);
                m_processes.removeAt(i);
                endRemoveRows();
                delete process;
                break;
            }
        }
    }

    foreach (Process *process, added) {
        QString name = process->name();
        auto processScore = score(process);
        int index = -1;
        auto size = m_processes.size();
        for (int i = 0; i != size && index == -1; i++) {
            auto curProcess = m_processes[i];
            auto curProcessScore = score(curProcess);
            if (processScore > curProcessScore) {
                index = i;
            } else if (processScore == curProcessScore) {
                auto nameDifference = name.compare(curProcess->name(), Qt::CaseInsensitive);
                if (nameDifference < 0 || (nameDifference == 0 && process->pid() < curProcess->pid())) {
                    index = i;
                }
            }
        }
        if (index == -1)
            index = size;
        beginInsertRows(parentRow, index, index);
        m_processes.insert(index, process);
        endInsertRows();
    }
}

void ProcessListModel::beginLoading()
{
    m_isLoading = true;
    emit isLoadingChanged(m_isLoading);
}

void ProcessListModel::endLoading()
{
    m_isLoading = false;
    emit isLoadingChanged(m_isLoading);
}

void ProcessListModel::onError(QString message)
{
    emit error(message);
}
