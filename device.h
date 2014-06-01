#ifndef FRIDAQML_DEVICE_H
#define FRIDAQML_DEVICE_H

#include <frida-core.h>

#include <QObject>

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
    QList<QObject *> processes() const { return m_processes; }

signals:
    void idChanged(unsigned int newId);
    void nameChanged(QString newName);
    void typeChanged(Type newType);
    void processesChanged(QList<QObject *> newProcesses);

private:
    FridaDevice *m_handle;
    unsigned int m_id;
    QString m_name;
    Type m_type;
    QList<QObject *> m_processes;
};

#endif
