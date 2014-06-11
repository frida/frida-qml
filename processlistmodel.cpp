#include "processlistmodel.h"

#include "device.h"
#include "process.h"

#include <QMetaMethod>

static const int ProcessPidRole = Qt::UserRole + 0;
static const int ProcessNameRole = Qt::UserRole + 1;

struct _EnumerateProcessesRequest
{
    ProcessListModel *model;
};

ProcessListModel::ProcessListModel(QObject *parent) :
    QAbstractListModel(parent),
    m_device(nullptr),
    m_isLoading(false),
    m_deviceHandle(nullptr),
    m_pendingRequest(nullptr),
    m_mainContext(frida_get_main_context())
{
}

void ProcessListModel::dispose()
{
    if (m_pendingRequest != nullptr) {
        m_pendingRequest->model = nullptr;
        m_pendingRequest = nullptr;
    }

    if (m_deviceHandle != nullptr) {
        g_object_unref(m_deviceHandle);
        m_deviceHandle = nullptr;
    }
}

ProcessListModel::~ProcessListModel()
{
    m_mainContext.perform([this] () { dispose(); });
}

void ProcessListModel::refresh()
{
    if (m_device == nullptr)
        return;

    m_mainContext.schedule([this] () { enumerateProcesses(); });
}

void ProcessListModel::setDevice(Device *device)
{
    if (device == m_device)
        return;

    m_device = device;
    emit deviceChanged(m_device);

    FridaDevice *deviceHandle = nullptr;
    if (device != nullptr) {
        deviceHandle = device->handle();
        g_object_ref(deviceHandle);
    }
    m_mainContext.schedule([=] () { updateDeviceHandle(deviceHandle); });

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
    return QHash<int, QByteArray>({
        {Qt::DisplayRole, "display"},
        {ProcessPidRole, "pid"},
        {ProcessNameRole, "name"}
    });
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
    default:
        return QVariant();
    }
}

void ProcessListModel::updateDeviceHandle(FridaDevice *deviceHandle)
{
    if (m_deviceHandle != nullptr) {
        g_object_unref(m_deviceHandle);
        m_deviceHandle = nullptr;
    }

    m_deviceHandle = deviceHandle;
    m_pids.clear();

    if (m_deviceHandle != nullptr)
        enumerateProcesses();
}

void ProcessListModel::enumerateProcesses()
{
    QMetaObject::invokeMethod(this, "beginLoading", Qt::QueuedConnection);

    if (m_pendingRequest != nullptr)
        m_pendingRequest->model = nullptr;
    auto request = g_slice_new(EnumerateProcessesRequest);
    request->model = this;
    m_pendingRequest = request;
    frida_device_enumerate_processes(m_deviceHandle, onEnumerateReadyWrapper, request);
}

void ProcessListModel::onEnumerateReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data)
{
    Q_UNUSED(obj);

    auto request = static_cast<EnumerateProcessesRequest *>(data);
    if (request->model != nullptr)
        request->model->onEnumerateReady(res);
    g_slice_free(EnumerateProcessesRequest, request);
}

void ProcessListModel::onEnumerateReady(GAsyncResult *res)
{
    m_pendingRequest = nullptr;

    QMetaObject::invokeMethod(this, "endLoading", Qt::QueuedConnection);

    GError *error = nullptr;
    auto processHandles = frida_device_enumerate_processes_finish(m_deviceHandle, res, &error);
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
                process->setParent(this);
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
            QMetaObject::invokeMethod(this, "updateItems", Qt::QueuedConnection,
                Q_ARG(FridaDevice *, m_deviceHandle),
                Q_ARG(QList<Process *>, added),
                Q_ARG(QSet<unsigned int>, removed));
        }
    } else {
        // TODO: report error
        g_printerr("Failed to enumerate processes of \"%s\": %s\n", frida_device_get_name(m_deviceHandle), error->message);
        g_clear_error(&error);
    }
}

void ProcessListModel::updateItems(FridaDevice *deviceHandle, QList<Process *> added, QSet<unsigned int> removed)
{
    if (m_device == nullptr || m_device->handle() != deviceHandle)
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
        int index = -1;
        auto size = m_processes.size();
        for (int i = 0; i != size; i++) {
            if (m_processes[i]->name().compare(name, Qt::CaseInsensitive) > 0) {
                index = i;
                break;
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
