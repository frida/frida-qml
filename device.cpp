#include "device.h"

#include <QMetaMethod>

Device::Device(FridaDevice *handle, QObject *parent) :
    QObject(parent),
    m_handle(handle),
    m_id(frida_device_get_id(handle)),
    m_name(frida_device_get_name(handle)),
    m_type(static_cast<Device::Type>(frida_device_get_dtype(handle))),
    m_processesListenerCount(0),
    m_processesRefreshing(false),
    m_processesRefreshTimer(NULL),
    m_mainContext(frida_get_main_context())
{
    g_object_ref(m_handle);
    g_object_set_data(G_OBJECT(m_handle), "qobject", this);
}

Device::~Device()
{
    m_mainContext.perform([this] () { dispose(); });
}

void Device::dispose()
{
    destroyProcessesRefreshTimer();
    g_object_set_data(G_OBJECT(m_handle), "qobject", NULL);
    g_object_unref(m_handle);
}

void Device::connectNotify(const QMetaMethod &signal)
{
    if (signal == QMetaMethod::fromSignal(&Device::processesChanged)) {
        m_mainContext.schedule([this] () { increaseProcessesListenerCount(); });
    }
}

void Device::disconnectNotify(const QMetaMethod &signal)
{
    if (!signal.isValid()) {
        m_mainContext.schedule([this] () { resetProcessesListenerCount(); });
    } else if (signal == QMetaMethod::fromSignal(&Device::processesChanged)) {
        m_mainContext.schedule([this] () { decreaseProcessesListenerCount(); });
    }
}

void Device::increaseProcessesListenerCount()
{
    m_processesListenerCount++;
    reconsiderProcessesRefreshScheduling();
}

void Device::decreaseProcessesListenerCount()
{
    m_processesListenerCount--;
    reconsiderProcessesRefreshScheduling();
}

void Device::resetProcessesListenerCount()
{
    m_processesListenerCount = 0;
    reconsiderProcessesRefreshScheduling();
}

void Device::reconsiderProcessesRefreshScheduling()
{
    if (m_processesListenerCount > 0) {
        if (!m_processesRefreshing) {
            m_processesRefreshing = true;
            frida_device_enumerate_processes(m_handle, onEnumerateProcessesReadyWrapper, this);
        }
    } else {
        destroyProcessesRefreshTimer();
    }
}

void Device::onEnumerateProcessesReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data)
{
    if (g_object_get_data(obj, "qobject") != NULL) {
        static_cast<Device *>(data)->onEnumerateProcessesReady(res);
    }
}

void Device::onEnumerateProcessesReady(GAsyncResult *res)
{
    m_processesRefreshing = false;

    GError *error = NULL;
    FridaProcessList *processHandles = frida_device_enumerate_processes_finish(m_handle, res, &error);
    if (error == NULL) {
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

        if (!added.empty() || !removed.empty()) {
            QMetaObject::invokeMethod(this, "updateProcesses", Qt::QueuedConnection,
                Q_ARG(QList<Process *>, added),
                Q_ARG(QSet<unsigned int>, removed));
        }
    } else {
        // TODO: report error
        g_printerr("Failed to enumerate processes of \"%s\": %s\n", frida_device_get_name(m_handle), error->message);
        g_clear_error(&error);
    }

    if (m_processesListenerCount > 0) {
        m_processesRefreshTimer = g_timeout_source_new_seconds(5);
        g_source_set_callback(m_processesRefreshTimer, onProcessesRefreshTimerTickWrapper, this, NULL);
        g_source_attach(m_processesRefreshTimer, m_mainContext.handle());
        g_source_unref(m_processesRefreshTimer);
    }
}

void Device::updateProcesses(QList<Process *> added, QSet<unsigned int> removed)
{
    foreach (unsigned int pid, removed)
        m_processes.remove(pid);
    foreach (Process *process, added)
        m_processes[process->pid()] = process;
    emit processesChanged(m_processes.values());
}

void Device::destroyProcessesRefreshTimer()
{
    if (m_processesRefreshTimer != NULL) {
        g_source_destroy(m_processesRefreshTimer);
        m_processesRefreshTimer = NULL;
    }
}

gboolean Device::onProcessesRefreshTimerTickWrapper(gpointer data)
{
    static_cast<Device *>(data)->onProcessesRefreshTimerTick();
    return FALSE;
}

void Device::onProcessesRefreshTimerTick()
{
    m_processesRefreshTimer = NULL;

    reconsiderProcessesRefreshScheduling();
}
