#ifndef FRIDAQML_SCRIPTS_H
#define FRIDAQML_SCRIPTS_H

#include <frida-core.h>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>

class Script;

class Scripts : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Scripts)
    Q_PROPERTY(QList<QObject *> items READ items NOTIFY itemsChanged)

public:
    explicit Scripts(QObject *parent = 0);
    ~Scripts();

    QList<QObject *> items() const { return m_items; }

    Q_INVOKABLE Script *createFromString(QString source);
    Q_INVOKABLE Script *createFromUrl(QUrl source);

signals:
    void itemsChanged(QList<QObject *> newProcesses);

private:
    QList<QObject *> m_items;
    QNetworkAccessManager m_networkAccessManager;
};

#endif
