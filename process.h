#ifndef FRIDAQML_PROCESS_H
#define FRIDAQML_PROCESS_H

#include <frida-core.h>

#include <QObject>

class Process : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Process)
    Q_PROPERTY(unsigned int pid READ pid CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    explicit Process(FridaProcess *handle, QObject *parent = nullptr);

    unsigned int pid() const { return m_pid; }
    QString name() const { return m_name; }

private:
    unsigned int m_pid;
    QString m_name;
};

#endif
