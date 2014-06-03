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
    Q_PROPERTY(QString source READ source NOTIFY sourceChanged)
    Q_PROPERTY(Device *device READ device NOTIFY deviceChanged)
    Q_PROPERTY(unsigned int pid READ pid NOTIFY pidChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_ENUMS(Status)

public:
    explicit Script(QString source, QNetworkAccessManager &networkAccessManager, QObject *parent = 0);
    explicit Script(QUrl source, QNetworkAccessManager &networkAccessManager, QObject *parent = 0);
    ~Script();

    QString source() const { return m_source; }
    Device *device() const { return m_device; }
    unsigned int pid() const { return m_pid; }
    enum Status { Loading, Loaded, Establishing, Compiling, Starting, Started, Error, Destroyed };
    Status status() const { return m_status; }

    Q_INVOKABLE void post(QJsonObject object);

    bool bind(Device *device, unsigned int pid);

private slots:
    void onStatus(Script::Status status);
    void onError(QString message);
    void onMessage(QJsonObject object, QByteArray data);

signals:
    void sourceChanged(QString newSource);
    void deviceChanged(Device *newDevice);
    void pidChanged(unsigned int newPid);
    void statusChanged(Status newStatus);
    void error(QString message);
    void message(QJsonObject object, QByteArray data);
    void send(QJsonObject object);

private:
    QString m_source;
    QNetworkAccessManager &m_networkAccessManager;
    Device *m_device;
    unsigned int m_pid;
    Status m_status;
};

#endif
