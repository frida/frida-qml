#ifndef FRIDAQML_SCRIPT_H
#define FRIDAQML_SCRIPT_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QQmlEngine>

class Device;
class ScriptInstance;

class Script : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Script)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(Runtime runtime READ runtime WRITE setRuntime NOTIFY runtimeChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QList<QObject *> instances READ instances NOTIFY instancesChanged)
    QML_ELEMENT

public:
    enum class Status { Loading, Loaded, Error };
    Q_ENUM(Status)

    enum class Runtime { Default, Duk, V8 };
    Q_ENUM(Runtime)

    explicit Script(QObject *parent = nullptr);

    Status status() const { return m_status; }
    QUrl url() const { return m_url; }
    void setUrl(QUrl url);
    QString name() const { return m_name; }
    void setName(QString name);
    Runtime runtime() const { return m_runtime; }
    void setRuntime(Runtime runtime);
    QString source() const { return m_source; }
    void setSource(QString source);
    QList<QObject *> instances() const { return m_instances; }
    Q_INVOKABLE void resumeProcess();

    Q_INVOKABLE void stop();
    Q_INVOKABLE void post(QJsonObject object);

    Q_INVOKABLE void enableDebugger();
    Q_INVOKABLE void enableDebugger(quint16 basePort);
    Q_INVOKABLE void disableDebugger();
    Q_INVOKABLE void enableJit();

private:
    ScriptInstance *bind(Device *device, int pid);
    void unbind(ScriptInstance *instance);

signals:
    void statusChanged(Status newStatus);
    void urlChanged(QUrl newUrl);
    void nameChanged(QString newName);
    void runtimeChanged(Runtime newRuntime);
    void sourceChanged(QString newSource);
    void instancesChanged(QList<QObject *> newInstances);
    void error(ScriptInstance *sender, QString message);
    void message(ScriptInstance *sender, QJsonObject object, QVariant data);

private:
    Status m_status;
    QUrl m_url;
    QString m_name;
    Runtime m_runtime;
    QString m_source;
    QNetworkAccessManager m_networkAccessManager;
    QList<QObject *> m_instances;

    friend class Device;
};

class ScriptInstance : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ScriptInstance)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(Device *device READ device CONSTANT FINAL)
    Q_PROPERTY(int pid READ pid NOTIFY pidChanged)
    Q_PROPERTY(ProcessState processState READ processState NOTIFY processStateChanged)
    QML_ELEMENT
    QML_UNCREATABLE("ScriptInstance objects cannot be instantiated from Qml");

public:
    enum class Status { Loading, Loaded, Establishing, Compiling, Starting, Started, Error, Destroyed };
    Q_ENUM(Status)

    enum class ProcessState { Spawning, Paused, Resuming, Running };
    Q_ENUM(ProcessState)

    explicit ScriptInstance(Device *device, int pid, Script *parent);

    Status status() const { return m_status; }
    Device *device() const { return m_device; }
    int pid() const { return m_pid; }
    ProcessState processState() const { return m_processState; }
    Q_INVOKABLE void resumeProcess();

    Q_INVOKABLE void stop();
    Q_INVOKABLE void post(QJsonObject object);

    Q_INVOKABLE void enableDebugger();
    Q_INVOKABLE void enableDebugger(quint16 port);
    Q_INVOKABLE void disableDebugger();
    Q_INVOKABLE void enableJit();

private slots:
    void onStatus(ScriptInstance::Status status);
    void onSpawnComplete(int pid);
    void onResumeComplete();
    void onError(QString message);
    void onMessage(QJsonObject object, QVariant data);

signals:
    void statusChanged(Status newStatus);
    void pidChanged(int newPid);
    void processStateChanged(ProcessState newState);
    void error(QString message);
    void message(QJsonObject object, QVariant data);
    void resumeProcessRequest();
    void stopRequest();
    void send(QJsonObject object);
    void enableDebuggerRequest(quint16 port);
    void disableDebuggerRequest();
    void enableJitRequest();

private:
    Status m_status;
    Device *m_device;
    int m_pid;
    ProcessState m_processState;

    friend class Device;
};

#endif
