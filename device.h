#ifndef FRIDAQML_DEVICE_H
#define FRIDAQML_DEVICE_H

#include <frida-core.h>

#include "maincontext.h"
#include "process.h"

#include <QHash>
#include <QObject>
#include <QSet>

class Device : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Device)
    Q_PROPERTY(unsigned int id READ id NOTIFY idChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Q_PROPERTY(QList<QObject *> processes READ processes NOTIFY processesChanged)
    Q_ENUMS(Type)

public:
    explicit Device(FridaDevice *handle, QObject *parent = 0);
    ~Device();

    unsigned int id() const { return m_id; }
    QString name() const { return m_name; }
    enum Type { Local, Tether, Remote };
    Type type() const { return m_type; }
    QList<QObject *> processes() const { return m_processes.values(); }

signals:
    void idChanged(unsigned int newId);
    void nameChanged(QString newName);
    void typeChanged(Type newType);
    void processesChanged(QList<QObject *> newProcesses);

private:
    void dispose();

protected:
    void connectNotify(const QMetaMethod &signal);
    void disconnectNotify(const QMetaMethod &signal);

private:
    void increaseProcessesListenerCount();
    void decreaseProcessesListenerCount();
    void resetProcessesListenerCount();
    void reconsiderProcessesRefreshScheduling();
    static void onEnumerateProcessesReadyWrapper(GObject *obj, GAsyncResult *res, gpointer data);
    void onEnumerateProcessesReady(GAsyncResult *res);

private slots:
    void updateProcesses(QList<Process *> added, QSet<unsigned int> removed);

private:
    void destroyProcessesRefreshTimer();
    static gboolean onProcessesRefreshTimerTickWrapper(gpointer data);
    void onProcessesRefreshTimerTick();

    FridaDevice *m_handle;
    unsigned int m_id;
    QString m_name;
    Type m_type;
    QHash<unsigned int, QObject *> m_processes;
    QSet<unsigned int> m_pids;
    int m_processesListenerCount;
    bool m_processesRefreshing;
    GSource *m_processesRefreshTimer;
    MainContext m_mainContext;
};

#endif
