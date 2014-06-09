#include "devicelistmodel.h"

#include "device.h"
#include "frida.h"

static const int DeviceNameRole = Qt::UserRole + 0;
static const int DeviceTypeRole = Qt::UserRole + 1;

DeviceListModel::DeviceListModel(QObject *parent) :
    QAbstractListModel(parent)
{
    auto frida = Frida::instance();
    m_devices = frida->deviceItems();
    connect(frida, &Frida::deviceAdded, this, &DeviceListModel::onDeviceAdded);
    connect(frida, &Frida::deviceRemoved, this, &DeviceListModel::onDeviceRemoved);
}

QHash<int, QByteArray> DeviceListModel::roleNames() const
{
    return QHash<int, QByteArray>({
        {Qt::DisplayRole, "display"},
        {DeviceNameRole, "name"},
        {DeviceTypeRole, "type"}
    });
}

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_devices.size();
}

QVariant DeviceListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);

    switch (role) {
    case Qt::DisplayRole:
    case DeviceNameRole:
        return QVariant("Name");
    case DeviceTypeRole:
        return QVariant("Type");
    default:
        return QVariant();
    }
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    auto device = m_devices[index.row()];
    switch (role) {
    case Qt::DisplayRole:
    case DeviceNameRole:
        return QVariant(device->name());
    case DeviceTypeRole:
        return QVariant(device->type());
    default:
        return QVariant();
    }
}

void DeviceListModel::onDeviceAdded(Device *device)
{
    auto rowIndex = m_devices.size();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    m_devices.append(device);
    endInsertRows();
}

void DeviceListModel::onDeviceRemoved(Device *device)
{
    auto rowIndex = m_devices.indexOf(device);
    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    m_devices.removeAt(rowIndex);
    endRemoveRows();
}
