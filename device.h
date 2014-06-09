#ifndef FRIDAQML_DEVICE_H
#define FRIDAQML_DEVICE_H

#include <frida-core.h>

#include "maincontext.h"
#include "script.h"

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QQueue>

class Processes;
class SessionEntry;
class ScriptEntry;

class Device : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Device)
    Q_PROPERTY(unsigned int id READ id NOTIFY idChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Q_PROPERTY(Processes *processes READ processes NOTIFY processesChanged)
    Q_ENUMS(Type)

public:
    explicit Device(FridaDevice *handle, QObject *parent = nullptr);
private:
    void dispose();
public:
    ~Device();

    FridaDevice *handle() const { return m_handle; }
    unsigned int id() const { return m_id; }
    QString name() const { return m_name; }
    enum Type { Local, Tether, Remote };
    Type type() const { return m_type; }
    Processes *processes() const { return m_processes; }

    Q_INVOKABLE void inject(Script *script, unsigned int pid);

signals:
    void idChanged(unsigned int newId);
    void nameChanged(QString newName);
    void typeChanged(Type newType);
    void processesChanged(Processes *newProcesses);

private:
    void performInject(Script *wrapper, Script::Status initialStatus, unsigned int pid);
    void performStop(Script *wrapper);
    void performAckStatus(Script *wrapper, Script::Status status);
    void performPost(Script *wrapper, QJsonObject object);
    void scheduleGarbageCollect();
    static gboolean onGarbageCollectTimeoutWrapper(gpointer data);
    void onGarbageCollectTimeout();

    FridaDevice *m_handle;
    unsigned int m_id;
    QString m_name;
    Type m_type;
    Processes *m_processes;

    QHash<unsigned int, SessionEntry *> m_sessions;
    QHash<Script *, ScriptEntry *> m_scripts;
    GSource *m_gcTimer;

    MainContext m_mainContext;
};

class SessionEntry : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SessionEntry)

public:
    explicit SessionEntry(Device *device, unsigned int pid, QObject *parent = nullptr);
    ~SessionEntry();

    QList<ScriptEntry *> scripts() const { return m_scripts; }

    ScriptEntry *add(Script *wrapper, Script::Status initialStatus);
    void remove(ScriptEntry *script);

signals:
    void detached();

private:
    static void onAttachReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onAttachReady(GAsyncResult *res);
    static void onDetachedWrapper(SessionEntry *self);
    void onDetached();

    Device *m_device;
    unsigned int m_pid;
    FridaSession *m_handle;
    QList<ScriptEntry *> m_scripts;
};

class ScriptEntry : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ScriptEntry)

public:
    explicit ScriptEntry(Device *device, unsigned int pid, Script *wrapper, Script::Status initialStatus, QObject *parent = nullptr);
    ~ScriptEntry();

    unsigned int pid() const { return m_pid; }

    void updateSessionHandle(FridaSession *sessionHandle);
    void notifySessionError(GError *error);
    void stop();
    void ackStatus(Script::Status status);
    void post(QJsonObject object);

signals:
    void stopped();

private:
    void updateStatus(Script::Status status);
    void updateError(GError *error);

    void start();
    static void onCreateReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onCreateReady(GAsyncResult *res);
    static void onLoadReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onLoadReady(GAsyncResult *res);
    void performPost(QJsonObject object);
    static void onMessage(ScriptEntry *self, const gchar *message, const gchar *data, gint dataSize);

    Device *m_device;
    unsigned int m_pid;
    Script *m_wrapper;
    Script::Status m_status;
    FridaScript *m_handle;
    FridaSession *m_sessionHandle;
    QQueue<QJsonObject> m_pending;
};

#endif
