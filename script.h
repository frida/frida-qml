#ifndef FRIDAQML_SCRIPT_H
#define FRIDAQML_SCRIPT_H

#include <frida-core.h>

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>

class Device;
class ScriptInstance;

class Script : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Script)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QList<QObject *> instances READ instances NOTIFY instancesChanged)
    Q_ENUMS(Status)

public:
    explicit Script(QObject *parent = nullptr);

    enum Status { Loading, Loaded, Error };
    Status status() const { return m_status; }
    QUrl url() const { return m_url; }
    void setUrl(QUrl url);
    QString source() const { return m_source; }
    void setSource(QString source);
    QList<QObject *> instances() const { return m_instances; }

    Q_INVOKABLE void stop();
    Q_INVOKABLE void post(QJsonObject object);

private:
    ScriptInstance *bind(Device *device, unsigned int pid);
    void unbind(ScriptInstance *instance);

signals:
    void statusChanged(Status newStatus);
    void urlChanged(QUrl newUrl);
    void sourceChanged(QString newSource);
    void instancesChanged(QList<QObject *> newInstances);
    void error(ScriptInstance *sender, QString message);
    void message(ScriptInstance *sender, QJsonObject object, QByteArray data);

private:
    Status m_status;
    QUrl m_url;
    QString m_source;
    QNetworkAccessManager m_networkAccessManager;
    QList<QObject *> m_instances;

    friend class Device;
};

class ScriptInstance : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ScriptInstance)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(Device *device READ device CONSTANT FINAL)
    Q_PROPERTY(unsigned int pid READ pid CONSTANT FINAL)
    Q_ENUMS(Status)
public:
    explicit ScriptInstance(Device *device, unsigned int pid, QObject *parent = nullptr);

    enum Status { Loading, Loaded, Establishing, Compiling, Starting, Started, Error, Destroyed };
    Status status() const { return m_status; }
    Device *device() const { return m_device; }
    unsigned int pid() const { return m_pid; }

    Q_INVOKABLE void stop();
    Q_INVOKABLE void post(QJsonObject object);

private slots:
    void onStatus(ScriptInstance::Status status);
    void onError(QString message);
    void onMessage(QJsonObject object, QByteArray data);

signals:
    void statusChanged(ScriptInstance::Status newStatus);
    void error(QString message);
    void message(QJsonObject object, QByteArray data);
    void stopRequest();
    void send(QJsonObject object);

private:
    Status m_status;
    Device *m_device;
    unsigned int m_pid;
};

#endif
