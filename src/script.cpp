#include "script.h"

#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>

Script::Script(QObject *parent) :
    QObject(parent),
    m_status(Status::Loaded),
    m_runtime(Runtime::Default),
    m_source("")
{
}

void Script::setUrl(QUrl url)
{
    if (url == m_url)
        return;

    QNetworkRequest request(url);
    auto reply = m_networkAccessManager.get(request);
    m_status = Status::Loading;
    emit statusChanged(m_status);
    connect(reply, &QNetworkReply::finished, [=] () {
        if (m_status == Status::Loading) {
            if (reply->error() == QNetworkReply::NoError) {
                if (m_name.isEmpty()) {
                    setName(url.fileName(QUrl::FullyDecoded).section(".", 0, 0));
                }

                m_source = QString::fromUtf8(reply->readAll());
                emit sourceChanged(m_source);

                m_status = Status::Loaded;
                emit statusChanged(m_status);
            } else {
                emit error(nullptr, QString("Failed to load “").append(url.toString()).append("”"));

                m_status = Status::Error;
                emit statusChanged(m_status);
            }
        }

        reply->deleteLater();
    });
}

void Script::setName(QString name)
{
    if (name == m_name)
        return;

    m_name = name;
    emit nameChanged(m_name);
}

void Script::setRuntime(Runtime runtime)
{
    if (runtime == m_runtime)
        return;

    m_runtime = runtime;
    emit runtimeChanged(m_runtime);
}

void Script::setSource(QString source)
{
    if (source == m_source)
        return;

    m_source = source;
    emit sourceChanged(m_source);

    if (m_status == Status::Loading) {
        m_status = Status::Loaded;
        emit statusChanged(m_status);
    }
}

void Script::stop()
{
    foreach (QObject *obj, m_instances)
        reinterpret_cast<ScriptInstance *>(obj)->stop();
}

void Script::post(QJsonObject object)
{
    foreach (QObject *obj, m_instances)
        reinterpret_cast<ScriptInstance *>(obj)->post(object);
}

void Script::enableDebugger()
{
    enableDebugger(0);
}

void Script::enableDebugger(quint16 basePort)
{
    int i = 0;
    foreach (QObject *obj, m_instances) {
        reinterpret_cast<ScriptInstance *>(obj)->enableDebugger(basePort + i);
        i++;
    }
}

void Script::disableDebugger()
{
    foreach (QObject *obj, m_instances)
        reinterpret_cast<ScriptInstance *>(obj)->disableDebugger();
}

void Script::enableJit()
{
    foreach (QObject *obj, m_instances)
        reinterpret_cast<ScriptInstance *>(obj)->enableJit();
}

ScriptInstance *Script::bind(Device *device, unsigned int pid)
{
    foreach (QObject *obj, m_instances) {
        auto instance = reinterpret_cast<ScriptInstance *>(obj);
        if (instance->device() == device && instance->pid() == pid)
            return nullptr;
    }

    auto instance = new ScriptInstance(device, pid, this);
    connect(instance, &ScriptInstance::error, [=] (QString message) {
        emit error(instance, message);
    });
    connect(instance, &ScriptInstance::message, [=] (QJsonObject object, QVariant data) {
        emit message(instance, object, data);
    });

    m_instances.append(instance);
    emit instancesChanged(m_instances);

    return instance;
}

void Script::unbind(ScriptInstance *instance)
{
    m_instances.removeOne(instance);
    emit instancesChanged(m_instances);

    instance->deleteLater();
}

ScriptInstance::ScriptInstance(Device *device, unsigned int pid, QObject *parent) :
    QObject(parent),
    m_status(Status::Loading),
    m_device(device),
    m_pid(pid)
{
}

void ScriptInstance::stop()
{
    if (m_status == Status::Destroyed)
        return;

    emit stopRequest();

    m_status = Status::Destroyed;
    emit statusChanged(m_status);
}

void ScriptInstance::post(QJsonObject object)
{
    emit send(object);
}

void ScriptInstance::enableDebugger()
{
    emit enableDebuggerRequest(0);
}

void ScriptInstance::enableDebugger(quint16 port)
{
    emit enableDebuggerRequest(port);
}

void ScriptInstance::disableDebugger()
{
    emit disableDebuggerRequest();
}

void ScriptInstance::enableJit()
{
    emit enableJitRequest();
}

void ScriptInstance::onStatus(Status status)
{
    if (m_status == Status::Destroyed)
        return;

    m_status = status;
    emit statusChanged(status);

    if (status == Status::Error)
        emit stopRequest();
}

void ScriptInstance::onError(QString message)
{
    if (m_status == Status::Destroyed)
        return;

    emit error(message);
}

void ScriptInstance::onMessage(QJsonObject object, QVariant data)
{
    if (m_status == Status::Destroyed)
        return;

    emit message(object, data);
}
