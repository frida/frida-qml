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

Device *DeviceListModel::get(int index) const
{
    if (index < 0 || index >= m_devices.size())
        return nullptr;

    return m_devices[index];
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
