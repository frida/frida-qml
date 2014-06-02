#include "script.h"

Script::Script(QString source, QObject *parent) :
    QObject(parent),
    m_source(source),
    m_device(0),
    m_pid(0),
    m_status(Created)
{
}

Script::~Script()
{
}

bool Script::bind(Device *device, unsigned int pid)
{
    if (m_device != 0)
        return false;

    m_device = device;
    m_pid = pid;
    emit deviceChanged(device);
    emit pidChanged(pid);

    return true;
}

void Script::onStatus(Status status)
{
    m_status = status;
    emit statusChanged(status);
}

void Script::onError(QString message)
{
    emit error(message);
}

void Script::onMessage(QString m, QByteArray data)
{
    emit message(m, data);
}
