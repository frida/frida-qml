#include "process.h"

Process::Process(FridaProcess *handle, QObject *parent) :
    QObject(parent),
    m_pid(frida_process_get_pid(handle)),
    m_name(frida_process_get_name(handle)),
    m_smallIcon(IconProvider::instance()->add(frida_process_get_small_icon(handle))),
    m_largeIcon(IconProvider::instance()->add(frida_process_get_large_icon(handle)))
{
}

Process::~Process()
{
    auto iconProvider = IconProvider::instance();
    iconProvider->remove(m_smallIcon);
    iconProvider->remove(m_largeIcon);
}
