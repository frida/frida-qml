#ifndef FRIDAQML_PROCESSES_H
#define FRIDAQML_PROCESSES_H

#include <frida-core.h>

#include "maincontext.h"
#include "process.h"

#include <QHash>
#include <QObject>
#include <QSet>

class Processes : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Processes)
    Q_PROPERTY(QList<QObject *> items READ items NOTIFY itemsChanged)

public:
    explicit Processes(FridaDevice *handle, QObject *parent = 0);
    ~Processes();

    QList<QObject *> items() const { return m_items.values(); }

signals:
    void itemsChanged(QList<QObject *> newProcesses);

private:
    void dispose();

protected:
    void connectNotify(const QMetaMethod &signal);
    void disconnectNotify(const QMetaMethod &signal);

private:
    void increaseListenerCount();
    void decreaseListenerCount();
    void resetListenerCount();
    void reconsiderRefreshScheduling();
    static void onEnumerateReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onEnumerateReady(GAsyncResult *res);

private slots:
    void updateItems(QList<Process *> added, QSet<unsigned int> removed);

private:
    void destroyRefreshTimer();
    static gboolean onRefreshTimerTickWrapper(gpointer data);
    void onRefreshTimerTick();

    FridaDevice *m_handle;
    QHash<unsigned int, QObject *> m_items;
    QSet<unsigned int> m_pids;
    int m_listenerCount;
    bool m_refreshing;
    GSource *m_refreshTimer;
    MainContext m_mainContext;
};

#endif
