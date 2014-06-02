#ifndef FRIDAQML_DEVICE_H
#define FRIDAQML_DEVICE_H

#include <frida-core.h>

#include "maincontext.h"

#include <QHash>
#include <QObject>

class Processes;
class SessionEntry;
class Script;
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
    explicit Device(FridaDevice *handle, QObject *parent = 0);
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
    void doInject(Script *script, unsigned int pid);
    static void onAttachReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onAttachReady(SessionEntry *session, GAsyncResult *res);
    void loadScript(ScriptEntry *script, SessionEntry *session);

    FridaDevice *m_handle;
    unsigned int m_id;
    QString m_name;
    Type m_type;
    Processes *m_processes;

    QHash<unsigned int, SessionEntry *> m_sessions;

    MainContext m_mainContext;
};

class SessionEntry : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SessionEntry)

public:
    explicit SessionEntry(Device *device, unsigned int pid, QObject *parent = 0);
    ~SessionEntry();

    void load(Script *wrapper);

private:
    static void onAttachReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onAttachReady(GAsyncResult *res);

    Device *m_device;
    FridaSession *m_handle;
    QList<ScriptEntry *> m_scripts;
};

class ScriptEntry : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ScriptEntry)

public:
    explicit ScriptEntry(Device *device, Script *wrapper, QObject *parent = 0);
    ~ScriptEntry();

    void load(FridaSession *sessionHandle);

private:
    static void onCreateScriptReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onCreateScriptReady(GAsyncResult *res);

    Device *m_device;
    Script *m_wrapper;
    FridaScript *m_handle;
    FridaSession *m_sessionHandle;
};

#endif
