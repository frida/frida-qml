#include "script.h"

#include <QNetworkRequest>
#include <QNetworkReply>

Script::Script(QString source, QNetworkAccessManager &networkAccessManager, QObject *parent) :
    QObject(parent),
    m_source(source),
    m_networkAccessManager(networkAccessManager),
    m_device(0),
    m_pid(0),
    m_status(Loaded)
{
}

Script::Script(QUrl source, QNetworkAccessManager &networkAccessManager, QObject *parent) :
    QObject(parent),
    m_source(""),
    m_networkAccessManager(networkAccessManager),
    m_device(0),
    m_pid(0),
    m_status(Loading)
{
    QNetworkRequest request(source);
    auto reply = m_networkAccessManager.get(request);
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

Script::~Script()
{
}

void Script::post(QJsonObject object)
{
    emit send(object);
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

void Script::onMessage(QJsonObject object, QByteArray data)
{
    emit message(object, data);
}
