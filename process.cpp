#include "process.h"

Process::Process(FridaProcess *handle, QObject *parent) :
    QObject(parent),
    m_pid(frida_process_get_pid(handle)),
    m_name(frida_process_get_name(handle))
{
}
