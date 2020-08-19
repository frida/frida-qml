#include <frida-core.h>

#include "iconprovider.h"

#include <QMutexLocker>

IconProvider *IconProvider::s_instance = nullptr;

IconProvider::IconProvider() :
    QQuickImageProvider(QQuickImageProvider::Image),
    m_lastId(0)
{
}

IconProvider::~IconProvider()
{
    s_instance = nullptr;
}

IconProvider *IconProvider::instance()
{
    if (s_instance == nullptr)
        s_instance = new IconProvider();
    return s_instance;
}

Icon IconProvider::add(FridaIcon *iconHandle)
{
    if (iconHandle == nullptr)
        return Icon();

    auto id = ++m_lastId;

    g_object_ref(iconHandle);
    {
        QMutexLocker locker(&m_mutex);
        m_icons[id] = iconHandle;
    }

    QUrl url;
    url.setScheme("image");
    url.setHost("frida");
    url.setPath(QString("/").append(QString::number(id)));

    return Icon(id, url);
}

void IconProvider::remove(Icon icon)
{
    if (!icon.isValid())
        return;

    auto id = icon.id();
    FridaIcon *iconHandle;
    {
        QMutexLocker locker(&m_mutex);
        iconHandle = m_icons[id];
        m_icons.remove(id);
    }
    g_object_unref(iconHandle);
}

QImage IconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    auto rawId = id.toInt();

    FridaIcon *iconHandle;
    {
        QMutexLocker locker(&m_mutex);
        iconHandle = m_icons[rawId];
        if (iconHandle != nullptr)
            g_object_ref(iconHandle);
        else
            return QImage();
    }

    auto width = frida_icon_get_width(iconHandle);
    auto height = frida_icon_get_height(iconHandle);
    auto pixels = frida_icon_get_pixels(iconHandle);

    *size = QSize(width, height);

    QImage result(static_cast<const uchar *>(g_bytes_get_data(pixels, nullptr)),
        width, height, frida_icon_get_rowstride(iconHandle),
        QImage::Format_RGBA8888,
        reinterpret_cast<QImageCleanupFunction>(g_bytes_unref), g_bytes_ref(pixels));

    if (requestedSize.isValid())
        return result.scaled(requestedSize, Qt::KeepAspectRatio);

    return result;
}
