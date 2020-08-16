#ifndef FRIDAQML_DEVICELISTMODEL_H
#define FRIDAQML_DEVICELISTMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>

class Device;

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceListModel)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    QML_ELEMENT

public:
    explicit DeviceListModel(QObject *parent = nullptr);

    Q_INVOKABLE int count() const { return m_devices.size(); }
    Q_INVOKABLE Device *get(int index) const;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

signals:
    void countChanged(int newCount);

private slots:
    void onDeviceAdded(Device *device);
    void onDeviceRemoved(Device *device);

private:
    QList<Device *> m_devices;
};

#endif
