#ifndef FRIDAQML_SCRIPT_H
#define FRIDAQML_SCRIPT_H

#include <frida-core.h>

#include <QObject>

class Device;

class Script : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Script)
    Q_PROPERTY(QString source READ source CONSTANT)
    Q_PROPERTY(Device *device READ device NOTIFY deviceChanged)
    Q_PROPERTY(unsigned int pid READ pid NOTIFY pidChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_ENUMS(Status)

public:
    explicit Script(QString source, QObject *parent = 0);
    ~Script();

    QString source() const { return m_source; }
    Device *device() const { return m_device; }
    unsigned int pid() const { return m_pid; }
    enum Status { Created, Establishing, Compiling, Loading, Running, Error, Destroyed };
    Status status() const { return m_status; }

    bool bind(Device *device, unsigned int pid);

private slots:
    void onStatus(Script::Status status);
    void onError(QString message);
    void onMessage(QString message, QByteArray data);

signals:
    void sourceChanged(QString newSource);
    void deviceChanged(Device *newDevice);
    void pidChanged(unsigned int newPid);
    void statusChanged(Status newStatus);
    void error(QString message);
    void message(QString message, QByteArray data);

private:
    QString m_source;
    Device *m_device;
    unsigned int m_pid;
    Status m_status;
};

#endif
