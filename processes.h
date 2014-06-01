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
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:
    explicit Processes(FridaDevice *handle, QObject *parent = 0);
    ~Processes();

    QList<QObject *> items() const { return m_items.values(); }
    bool isLoading() const { return m_isLoading; }

signals:
    void itemsChanged(QList<QObject *> newProcesses);
    void isLoadingChanged(bool newIsLoading);

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
    void beginLoading();
    void endLoading();

private:
    void destroyRefreshTimer();
    static gboolean onRefreshTimerTickWrapper(gpointer data);
    void onRefreshTimerTick();

    FridaDevice *m_handle;

    QHash<unsigned int, QObject *> m_items;
    bool m_isLoading;

    bool m_isEnumerating;
    int m_listenerCount;
    GSource *m_refreshTimer;
    QSet<unsigned int> m_pids;

    MainContext m_mainContext;
};

#endif
