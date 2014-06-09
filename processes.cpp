#include "processes.h"

#include "process.h"

#include <QMetaMethod>

Processes::Processes(FridaDevice *handle, QObject *parent) :
    QObject(parent),
    m_handle(handle),
    m_isLoading(false),
    m_isEnumerating(false),
    m_listenerCount(0),
    m_refreshTimer(nullptr),
    m_mainContext(frida_get_main_context())
{
    g_object_ref(m_handle);
    g_object_set_data(G_OBJECT(m_handle), "qprocesses", this);
}

void Processes::dispose()
{
    destroyRefreshTimer();
    g_object_set_data(G_OBJECT(m_handle), "qprocesses", nullptr);
    g_object_unref(m_handle);
}

Processes::~Processes()
{
    m_mainContext.perform([this] () { dispose(); });
}

void Processes::connectNotify(const QMetaMethod &signal)
{
    if (signal == QMetaMethod::fromSignal(&Processes::itemsChanged)) {
        m_mainContext.schedule([this] () { increaseListenerCount(); });
    }
}

void Processes::disconnectNotify(const QMetaMethod &signal)
{
    if (!signal.isValid()) {
        m_mainContext.schedule([this] () { resetListenerCount(); });
    } else if (signal == QMetaMethod::fromSignal(&Processes::itemsChanged)) {
        m_mainContext.schedule([this] () { decreaseListenerCount(); });
    }
}

void Processes::increaseListenerCount()
{
    m_listenerCount++;
    reconsiderRefreshScheduling();
}

void Processes::decreaseListenerCount()
{
    m_listenerCount--;
    reconsiderRefreshScheduling();
}

void Processes::resetListenerCount()
{
    m_listenerCount = 0;
    reconsiderRefreshScheduling();
}

void Processes::reconsiderRefreshScheduling()
{
    if (m_listenerCount > 0) {
        if (!m_isEnumerating) {
            m_isEnumerating = true;

            if (m_pids.isEmpty()) {
                QMetaObject::invokeMethod(this, "beginLoading", Qt::QueuedConnection);
            }

            frida_device_enumerate_processes(m_handle, onEnumerateReadyWrapper, this);
        }
    } else {
        destroyRefreshTimer();
    }
}

void Processes::onEnumerateReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data)
{
    if (g_object_get_data(obj, "qprocesses") != nullptr) {
        static_cast<Processes *>(data)->onEnumerateReady(res);
    }
}

void Processes::onEnumerateReady(GAsyncResult *res)
{
    m_isEnumerating = false;

    if (m_pids.isEmpty()) {
        QMetaObject::invokeMethod(this, "endLoading", Qt::QueuedConnection);
    }

    GError *error = nullptr;
    auto processHandles = frida_device_enumerate_processes_finish(m_handle, res, &error);
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

        if (!added.empty() || !removed.empty()) {
            QMetaObject::invokeMethod(this, "updateItems", Qt::QueuedConnection,
                Q_ARG(QList<Process *>, added),
                Q_ARG(QSet<unsigned int>, removed));
        }
    } else {
        // TODO: report error
        g_printerr("Failed to enumerate processes of \"%s\": %s\n", frida_device_get_name(m_handle), error->message);
        g_clear_error(&error);
    }

    if (m_listenerCount > 0) {
        m_refreshTimer = g_timeout_source_new_seconds(5);
        g_source_set_callback(m_refreshTimer, onRefreshTimerTickWrapper, this, nullptr);
        g_source_attach(m_refreshTimer, m_mainContext.handle());
        g_source_unref(m_refreshTimer);
    }
}

void Processes::updateItems(QList<Process *> added, QSet<unsigned int> removed)
{
    foreach (unsigned int pid, removed)
        m_items.remove(pid);
    foreach (Process *process, added)
        m_items[process->pid()] = process;
    emit itemsChanged(m_items.values());
}

void Processes::beginLoading()
{
    m_isLoading = true;
    emit isLoadingChanged(m_isLoading);
}

void Processes::endLoading()
{
    m_isLoading = false;
    emit isLoadingChanged(m_isLoading);
}

void Processes::destroyRefreshTimer()
{
    if (m_refreshTimer != nullptr) {
        g_source_destroy(m_refreshTimer);
        m_refreshTimer = nullptr;
    }
}

gboolean Processes::onRefreshTimerTickWrapper(gpointer data)
{
    static_cast<Processes *>(data)->onRefreshTimerTick();
    return FALSE;
}

void Processes::onRefreshTimerTick()
{
    m_refreshTimer = nullptr;

    reconsiderRefreshScheduling();
}
