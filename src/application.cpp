#include <frida-core.h>

#include "application.h"

Application::Application(FridaApplication *handle, QObject *parent) :
    QObject(parent),
    m_identifier(frida_application_get_identifier(handle)),
    m_name(frida_application_get_name(handle)),
    m_pid(frida_application_get_pid(handle)),
    m_smallIcon(IconProvider::instance()->add(frida_application_get_small_icon(handle))),
    m_largeIcon(IconProvider::instance()->add(frida_application_get_large_icon(handle)))
{
}

Application::~Application()
{
    auto iconProvider = IconProvider::instance();
    iconProvider->remove(m_smallIcon);
    iconProvider->remove(m_largeIcon);
}
