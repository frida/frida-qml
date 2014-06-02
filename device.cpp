#include "device.h"

#include "processes.h"
#include "script.h"

Device::Device(FridaDevice *handle, QObject *parent) :
    QObject(parent),
    m_handle(handle),
    m_id(frida_device_get_id(handle)),
    m_name(frida_device_get_name(handle)),
    m_type(static_cast<Device::Type>(frida_device_get_dtype(handle))),
    m_processes(new Processes(handle, this)),
    m_mainContext(frida_get_main_context())
{
    g_object_ref(m_handle);
    g_object_set_data(G_OBJECT(m_handle), "qdevice", this);
}

void Device::dispose()
{
    auto it = m_sessions.constBegin();
    while (it != m_sessions.constEnd()) {
        delete it.value();
        ++it;
    }

    g_object_set_data(G_OBJECT(m_handle), "qdevice", NULL);
    g_object_unref(m_handle);
}

Device::~Device()
{
    m_mainContext.perform([this] () { dispose(); });
}

void Device::inject(Script *script, unsigned int pid)
{
    if (script != 0 && script->bind(this, pid)) {
        m_mainContext.schedule([this, script, pid] () { doInject(script, pid); });
    }
}

void Device::doInject(Script *wrapper, unsigned int pid)
{
    auto session = m_sessions[pid];
    if (session == 0) {
        session = new SessionEntry(this, pid);
        m_sessions[pid] = session;
    }

    session->load(wrapper);
}

SessionEntry::SessionEntry(Device *device, unsigned int pid, QObject *parent) :
    QObject(parent),
    m_device(device),
    m_handle(0)
{
    frida_device_attach(device->handle(), pid, onAttachReadyWrapper, this);
}

SessionEntry::~SessionEntry()
{
    if (m_handle != 0) {
        g_object_set_data(G_OBJECT(m_handle), "qsession", NULL);
        g_object_unref(m_handle);
    }
}

void SessionEntry::load(Script *wrapper)
{
    auto script = new ScriptEntry(m_device, wrapper, this);
    m_scripts.append(script);

    if (m_handle != 0) {
        script->load(m_handle);
    } else {
        script->notifyStatus(Script::Establishing);
    }
}

void SessionEntry::onAttachReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data)
{
    if (g_object_get_data(obj, "qdevice") != NULL) {
        static_cast<SessionEntry *>(data)->onAttachReady(res);
    }
}

void SessionEntry::onAttachReady(GAsyncResult *res)
{
    GError *error = NULL;
    m_handle = frida_device_attach_finish(m_device->handle(), res, &error);
    if (error == NULL) {
        g_object_set_data(G_OBJECT(m_handle), "qsession", this);
        foreach (ScriptEntry *script, m_scripts)
            script->load(m_handle);
    } else {
        foreach (ScriptEntry *script, m_scripts) {
            script->notifyError(error);
            script->notifyStatus(Script::Error);
        }
        g_clear_error(&error);
    }
}

ScriptEntry::ScriptEntry(Device *device, Script *wrapper, QObject *parent) :
    QObject(parent),
    m_device(device),
    m_wrapper(wrapper),
    m_handle(0),
    m_sessionHandle(0)
{
}

ScriptEntry::~ScriptEntry()
{
    if (m_handle != 0) {
        g_signal_handlers_disconnect_by_func(m_handle, GSIZE_TO_POINTER(onMessage), this);

        g_object_set_data(G_OBJECT(m_handle), "qscript", NULL);
        g_object_unref(m_handle);
    }
}

void ScriptEntry::notifyStatus(Script::Status status)
{
    QMetaObject::invokeMethod(m_wrapper, "onStatus", Qt::QueuedConnection,
        Q_ARG(Script::Status, status));
}

void ScriptEntry::notifyError(GError *error)
{
    auto message = QString::fromUtf8(error->message);
    QMetaObject::invokeMethod(m_wrapper, "onError", Qt::QueuedConnection,
        Q_ARG(QString, message));
}

void ScriptEntry::load(FridaSession *sessionHandle)
{
    m_sessionHandle = sessionHandle;

    notifyStatus(Script::Compiling);
    auto source = m_wrapper->source().toUtf8();
    frida_session_create_script(m_sessionHandle, source.data(), onCreateReadyWrapper, this);
}

void ScriptEntry::onCreateReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data)
{
    if (g_object_get_data(obj, "qsession") != NULL) {
        static_cast<ScriptEntry *>(data)->onCreateReady(res);
    }
}

void ScriptEntry::onCreateReady(GAsyncResult *res)
{
    GError *error = NULL;
    m_handle = frida_session_create_script_finish(m_sessionHandle, res, &error);
    if (error == NULL) {
        g_object_set_data(G_OBJECT(m_handle), "qscript", this);

        g_signal_connect_swapped(m_handle, "message", G_CALLBACK(onMessage), this);

        notifyStatus(Script::Loading);
        frida_script_load(m_handle, onLoadReadyWrapper, this);
    } else {
        notifyError(error);
        notifyStatus(Script::Error);
        g_clear_error(&error);
    }
}

void ScriptEntry::onLoadReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data)
{
    if (g_object_get_data(obj, "qscript") != NULL) {
        static_cast<ScriptEntry *>(data)->onLoadReady(res);
    }
}

void ScriptEntry::onLoadReady(GAsyncResult *res)
{
    GError *error = NULL;
    frida_script_load_finish(m_handle, res, &error);
    if (error == NULL) {
        notifyStatus(Script::Running);
    } else {
        notifyError(error);
        notifyStatus(Script::Error);
        g_clear_error(&error);
    }
}

void ScriptEntry::onMessage(ScriptEntry *self, const gchar *message, const gchar *data, gint dataSize)
{
    auto messageValue = QString::fromUtf8(message);
    auto dataValue = QByteArray::fromRawData(data, dataSize);
    QMetaObject::invokeMethod(self->m_wrapper, "onMessage", Qt::QueuedConnection,
        Q_ARG(QString, messageValue),
        Q_ARG(QByteArray, dataValue));
}
