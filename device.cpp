#include "device.h"

#include "script.h"

#include <memory>
#include <QJsonDocument>

Device::Device(FridaDevice *handle, QObject *parent) :
    QObject(parent),
    m_handle(handle),
    m_id(frida_device_get_id(handle)),
    m_name(frida_device_get_name(handle)),
    m_icon(IconProvider::instance()->add(frida_device_get_icon(handle))),
    m_type(static_cast<Device::Type>(frida_device_get_dtype(handle))),
    m_gcTimer(nullptr),
    m_mainContext(frida_get_main_context())
{
    g_object_ref(m_handle);
    g_object_set_data(G_OBJECT(m_handle), "qdevice", this);
}

void Device::dispose()
{
    if (m_gcTimer != nullptr) {
        g_source_destroy(m_gcTimer);
        m_gcTimer = nullptr;
    }

    auto it = m_sessions.constBegin();
    while (it != m_sessions.constEnd()) {
        delete it.value();
        ++it;
    }

    g_object_set_data(G_OBJECT(m_handle), "qdevice", nullptr);
    g_object_unref(m_handle);
}

Device::~Device()
{
    IconProvider::instance()->remove(m_icon);

    m_mainContext.perform([this] () { dispose(); });
}

void Device::inject(Script *script, unsigned int pid)
{
    ScriptInstance *scriptInstance = script != nullptr ? script->bind(this, pid) : nullptr;
    if (scriptInstance != nullptr) {
        auto onStatusChanged = std::make_shared<QMetaObject::Connection>();
        auto onStopRequest = std::make_shared<QMetaObject::Connection>();
        auto onSend = std::make_shared<QMetaObject::Connection>();
        *onStatusChanged = connect(script, &Script::statusChanged, [=] (Script::Status newStatus) {
            if (newStatus == Script::Loaded) {
                auto source = script->source();
                m_mainContext.schedule([=] () { performLoad(scriptInstance, source); });
            }
        });
        *onStopRequest = connect(scriptInstance, &ScriptInstance::stopRequest, [=] () {
            QObject::disconnect(*onStatusChanged);
            QObject::disconnect(*onStopRequest);
            QObject::disconnect(*onSend);

            script->unbind(scriptInstance);

            m_mainContext.schedule([=] () { performStop(scriptInstance); });
        });
        *onSend = connect(scriptInstance, &ScriptInstance::send, [=] (QJsonObject object) {
            m_mainContext.schedule([=] () { performPost(scriptInstance, object); });
        });

        m_mainContext.schedule([=] () { performInject(pid, scriptInstance); });

        if (script->status() == Script::Loaded) {
            auto source = script->source();
            m_mainContext.schedule([=] () { performLoad(scriptInstance, source); });
        }
    }
}

void Device::performInject(unsigned int pid, ScriptInstance *wrapper)
{
    auto session = m_sessions[pid];
    if (session == nullptr) {
        session = new SessionEntry(this, pid);
        m_sessions[pid] = session;
        connect(session, &SessionEntry::detached, [=] () {
            foreach (ScriptEntry *script, session->scripts())
                m_scripts.remove(script->wrapper());
            m_sessions.remove(pid);
            m_mainContext.schedule([=] () {
                delete session;
            });
        });
    }

    auto script = session->add(wrapper);
    m_scripts[wrapper] = script;
    connect(script, &ScriptEntry::stopped, [=] () {
        m_mainContext.schedule([=] () { delete script; });
    });
}

void Device::performLoad(ScriptInstance *wrapper, QString source)
{
    auto script = m_scripts[wrapper];
    if (script == nullptr)
        return;
    script->load(source);
}

void Device::performStop(ScriptInstance *wrapper)
{
    auto script = m_scripts[wrapper];
    if (script == nullptr)
        return;
    m_scripts.remove(wrapper);

    script->session()->remove(script);

    scheduleGarbageCollect();
}

void Device::performPost(ScriptInstance *wrapper, QJsonObject object)
{
    auto script = m_scripts[wrapper];
    if (script == nullptr)
        return;
    script->post(object);
}

void Device::scheduleGarbageCollect()
{
    if (m_gcTimer != nullptr) {
        g_source_destroy(m_gcTimer);
        m_gcTimer = nullptr;
    }

    auto timer = g_timeout_source_new_seconds(5);
    g_source_set_callback(timer, onGarbageCollectTimeoutWrapper, this, nullptr);
    g_source_attach(timer, m_mainContext.handle());
    g_source_unref(timer);
    m_gcTimer = timer;
}

gboolean Device::onGarbageCollectTimeoutWrapper(gpointer data)
{
    static_cast<Device *>(data)->onGarbageCollectTimeout();

    return FALSE;
}

void Device::onGarbageCollectTimeout()
{
    m_gcTimer = nullptr;

    auto newSessions = QHash<unsigned int, SessionEntry *>();
    auto it = m_sessions.constBegin();
    while (it != m_sessions.constEnd()) {
        auto pid = it.key();
        auto session = it.value();
        if (session->scripts().isEmpty()) {
            delete session;
        } else {
            newSessions[pid] = session;
        }
        ++it;
    }
    m_sessions = newSessions;
}

SessionEntry::SessionEntry(Device *device, unsigned int pid, QObject *parent) :
    QObject(parent),
    m_device(device),
    m_pid(pid),
    m_handle(nullptr)
{
    frida_device_attach(device->handle(), pid, onAttachReadyWrapper, this);
}

SessionEntry::~SessionEntry()
{
    if (m_handle != nullptr) {
        frida_session_detach(m_handle, nullptr, nullptr);

        g_signal_handlers_disconnect_by_func(m_handle, GSIZE_TO_POINTER(onDetachedWrapper), this);

        g_object_set_data(G_OBJECT(m_handle), "qsession", nullptr);
        g_object_unref(m_handle);
    }
}

ScriptEntry *SessionEntry::add(ScriptInstance *wrapper)
{
    auto script = new ScriptEntry(this, wrapper, this);
    m_scripts.append(script);
    script->updateSessionHandle(m_handle);
    return script;
}

void SessionEntry::remove(ScriptEntry *script)
{
    script->stop();
    m_scripts.removeOne(script);
}

void SessionEntry::onAttachReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data)
{
    if (g_object_get_data(obj, "qdevice") != nullptr) {
        static_cast<SessionEntry *>(data)->onAttachReady(res);
    }
}

void SessionEntry::onAttachReady(GAsyncResult *res)
{
    GError *error = nullptr;
    m_handle = frida_device_attach_finish(m_device->handle(), res, &error);
    if (error == nullptr) {
        g_object_set_data(G_OBJECT(m_handle), "qsession", this);

        g_signal_connect_swapped(m_handle, "detached", G_CALLBACK(onDetachedWrapper), this);

        foreach (ScriptEntry *script, m_scripts) {
            script->updateSessionHandle(m_handle);
        }
    } else {
        foreach (ScriptEntry *script, m_scripts) {
            script->notifySessionError(error);
        }
        g_clear_error(&error);
    }
}

void SessionEntry::onDetachedWrapper(SessionEntry *self)
{
    self->onDetached();
}

void SessionEntry::onDetached()
{
    foreach (ScriptEntry *script, m_scripts)
        script->notifySessionError("Target process terminated");

    emit detached();
}

ScriptEntry::ScriptEntry(SessionEntry *session, ScriptInstance *wrapper, QObject *parent) :
    QObject(parent),
    m_status(ScriptInstance::Loading),
    m_session(session),
    m_wrapper(wrapper),
    m_handle(nullptr),
    m_sessionHandle(nullptr)
{
}

ScriptEntry::~ScriptEntry()
{
    if (m_handle != nullptr) {
        frida_script_unload(m_handle, nullptr, nullptr);

        g_signal_handlers_disconnect_by_func(m_handle, GSIZE_TO_POINTER(onMessage), this);

        g_object_set_data(G_OBJECT(m_handle), "qscript", nullptr);
        g_object_unref(m_handle);
    }
}

void ScriptEntry::updateSessionHandle(FridaSession *sessionHandle)
{
    m_sessionHandle = sessionHandle;
    start();
}

void ScriptEntry::notifySessionError(GError *error)
{
    updateError(error);
    updateStatus(ScriptInstance::Error);
}

void ScriptEntry::notifySessionError(QString message)
{
    updateError(message);
    updateStatus(ScriptInstance::Error);
}

void ScriptEntry::post(QJsonObject object)
{
    if (m_status == ScriptInstance::Started) {
        performPost(object);
    } else if (m_status < ScriptInstance::Started) {
        m_pending.enqueue(object);
    } else {
        // Drop silently
    }
}

void ScriptEntry::updateStatus(ScriptInstance::Status status)
{
    if (status == m_status)
        return;

    m_status = status;

    QMetaObject::invokeMethod(m_wrapper, "onStatus", Qt::QueuedConnection,
        Q_ARG(ScriptInstance::Status, status));

    if (status == ScriptInstance::Started) {
        while (!m_pending.isEmpty())
            performPost(m_pending.dequeue());
    } else if (status > ScriptInstance::Started) {
        m_pending.clear();
    }
}

void ScriptEntry::updateError(GError *error)
{
    updateError(QString::fromUtf8(error->message));
}

void ScriptEntry::updateError(QString message)
{
    QMetaObject::invokeMethod(m_wrapper, "onError", Qt::QueuedConnection,
        Q_ARG(QString, message));
}

void ScriptEntry::load(QString source)
{
    if (m_status != ScriptInstance::Loading)
        return;

    m_source = source;
    updateStatus(ScriptInstance::Loaded);

    start();
}

void ScriptEntry::start()
{
    if (m_status == ScriptInstance::Loading)
        return;

    if (m_sessionHandle != nullptr) {
        updateStatus(ScriptInstance::Compiling);
        auto source = m_source.toUtf8();
        frida_session_create_script(m_sessionHandle, source.data(), onCreateReadyWrapper, this);
    } else {
        updateStatus(ScriptInstance::Establishing);
    }
}

void ScriptEntry::stop()
{
    bool canStopNow = m_status != ScriptInstance::Compiling && m_status != ScriptInstance::Starting;

    m_status = ScriptInstance::Destroyed;

    if (canStopNow)
        emit stopped();
}

void ScriptEntry::onCreateReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data)
{
    if (g_object_get_data(obj, "qsession") != nullptr) {
        static_cast<ScriptEntry *>(data)->onCreateReady(res);
    }
}

void ScriptEntry::onCreateReady(GAsyncResult *res)
{
    if (m_status == ScriptInstance::Destroyed) {
        emit stopped();
        return;
    }

    GError *error = nullptr;
    m_handle = frida_session_create_script_finish(m_sessionHandle, res, &error);
    if (error == nullptr) {
        g_object_set_data(G_OBJECT(m_handle), "qscript", this);

        g_signal_connect_swapped(m_handle, "message", G_CALLBACK(onMessage), this);

        updateStatus(ScriptInstance::Starting);
        frida_script_load(m_handle, onLoadReadyWrapper, this);
    } else {
        updateError(error);
        updateStatus(ScriptInstance::Error);
        g_clear_error(&error);
    }
}

void ScriptEntry::onLoadReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data)
{
    if (g_object_get_data(obj, "qscript") != nullptr) {
        static_cast<ScriptEntry *>(data)->onLoadReady(res);
    }
}

void ScriptEntry::onLoadReady(GAsyncResult *res)
{
    if (m_status == ScriptInstance::Destroyed) {
        emit stopped();
        return;
    }

    GError *error = nullptr;
    frida_script_load_finish(m_handle, res, &error);
    if (error == nullptr) {
        updateStatus(ScriptInstance::Started);
    } else {
        updateError(error);
        updateStatus(ScriptInstance::Error);
        g_clear_error(&error);
    }
}

void ScriptEntry::performPost(QJsonObject object)
{
    QJsonDocument document(object);
    auto json = document.toJson(QJsonDocument::Compact);
    frida_script_post_message(m_handle, json.data(), nullptr, nullptr);
}

void ScriptEntry::onMessage(ScriptEntry *self, const gchar *message, const gchar *data, gint dataSize)
{
    auto messageJson = QByteArray::fromRawData(message, static_cast<int>(strlen(message)));
    auto messageDocument = QJsonDocument::fromJson(messageJson);
    auto dataValue = QByteArray::fromRawData(data, dataSize);
    QMetaObject::invokeMethod(self->m_wrapper, "onMessage", Qt::QueuedConnection,
        Q_ARG(QJsonObject, messageDocument.object()),
        Q_ARG(QByteArray, dataValue));
}
