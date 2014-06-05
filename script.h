#ifndef FRIDAQML_SCRIPT_H
#define FRIDAQML_SCRIPT_H

#include <frida-core.h>

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>

class Device;

class Script : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Script)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(Device *device READ device NOTIFY deviceChanged)
    Q_PROPERTY(unsigned int pid READ pid NOTIFY pidChanged)
    Q_ENUMS(Status)

public:
    explicit Script(QObject *parent = 0);
    ~Script();

    enum Status { Loading, Loaded, Establishing, Compiling, Starting, Started, Error, Destroyed };
    Status status() const { return m_status; }
    QUrl url() const { return m_url; }
    void setUrl(QUrl url);
    QString source() const { return m_source; }
    void setSource(QString source);
    Device *device() const { return m_device; }
    unsigned int pid() const { return m_pid; }

    Q_INVOKABLE void stop();
    Q_INVOKABLE void post(QJsonObject object);

private:
    bool bind(Device *device, unsigned int pid);

private slots:
    void onStatus(Device *device, unsigned int pid, Script::Status status);
    void onError(Device *device, unsigned int pid, QString message);
    void onMessage(Device *device, unsigned int pid, QJsonObject object, QByteArray data);

signals:
    void statusChanged(Status newStatus);
    void urlChanged(QUrl newUrl);
    void sourceChanged(QString newSource);
    void deviceChanged(Device *newDevice);
    void pidChanged(unsigned int newPid);
    void error(QString message);
    void message(QJsonObject object, QByteArray data);
    void stopRequest();
    void send(QJsonObject object);

private:
    Status m_status;
    QUrl m_url;
    QString m_source;
    Device *m_device;
    unsigned int m_pid;
    QNetworkAccessManager m_networkAccessManager;

    friend class Device;
};

#endif
