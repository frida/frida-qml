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

public:
    explicit Script(QString source, QObject *parent = 0);
    ~Script();

    QString source() const { return m_source; }
    Device *device() const { return m_device; }
    unsigned int pid() const { return m_pid; }

    bool bind(Device *device, unsigned int pid);

signals:
    void sourceChanged(QString newSource);
    void deviceChanged(Device *newDevice);
    void pidChanged(unsigned int newPid);

private:
    QString m_source;
    Device *m_device;
    unsigned int m_pid;
};

#endif
