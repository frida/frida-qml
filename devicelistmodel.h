#ifndef FRIDAQML_DEVICELISTMODEL_H
#define FRIDAQML_DEVICELISTMODEL_H

#include <frida-core.h>

#include <QAbstractListModel>

class Device;

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceListModel)

public:
    explicit DeviceListModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;

private slots:
    void onDeviceAdded(Device *device);
    void onDeviceRemoved(Device *device);

private:
    QList<Device *> m_devices;
};

#endif
