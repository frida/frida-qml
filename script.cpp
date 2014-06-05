#include "script.h"

#include <QNetworkRequest>
#include <QNetworkReply>

Script::Script(QObject *parent) :
    QObject(parent),
    m_status(Loaded),
    m_source(""),
    m_device(0),
    m_pid(0)
{
}

Script::~Script()
{
}

void Script::setUrl(QUrl url)
{
    if (url == m_url || m_device != 0)
        return;

    QNetworkRequest request(url);
    auto reply = m_networkAccessManager.get(request);
    m_status = Loading;
    connect(reply, &QNetworkReply::finished, [=] () {
        m_source = QString::fromUtf8(reply->readAll());
        emit sourceChanged(m_source);

        if (m_status == Loading) {
            m_status = Loaded;
            emit statusChanged(m_status);
        }

        reply->deleteLater();
    });
}

void Script::setSource(QString source)
{
    if (source == m_source || m_device != 0)
        return;

    m_source = source;
    m_status = Loaded;
}

void Script::stop()
{
    if (m_device == 0)
        return;

    emit stopRequest();

    if (m_status > Loaded) {
        m_status = Loaded;
        emit statusChanged(m_status);
    }

    m_device = 0;
    m_pid = 0;
    emit deviceChanged(m_device);
    emit pidChanged(m_pid);
}

void Script::post(QJsonObject object)
{
    emit send(object);
}

bool Script::bind(Device *device, unsigned int pid)
{
    if (device == m_device && pid == m_pid) {
        return false;
    }

    stop();

    m_device = device;
    m_pid = pid;
    emit deviceChanged(device);
    emit pidChanged(pid);

    return true;
}

void Script::onStatus(Device *device, unsigned int pid, Status status)
{
    if (device == m_device && pid == m_pid) {
        m_status = status;
        emit statusChanged(status);
    }
}

void Script::onError(Device *device, unsigned int pid, QString message)
{
    if (device == m_device && pid == m_pid) {
        emit error(message);
    }
}

void Script::onMessage(Device *device, unsigned int pid, QJsonObject object, QByteArray data)
{
    if (device == m_device && pid == m_pid) {
        emit message(object, data);
    }
}
