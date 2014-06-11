#ifndef FRIDAQML_DEVICELISTMODEL_H
#define FRIDAQML_DEVICELISTMODEL_H

#include <frida-core.h>

#include <QAbstractListModel>

class Device;

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceListModel)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit DeviceListModel(QObject *parent = nullptr);

    Q_INVOKABLE int count() const { return m_devices.size(); }
    Q_INVOKABLE Device *get(int index) const;

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

signals:
    void countChanged(int newCount);

private slots:
    void onDeviceAdded(Device *device);
    void onDeviceRemoved(Device *device);

private:
    QList<Device *> m_devices;
};

#endif
