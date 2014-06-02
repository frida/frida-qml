#include "scripts.h"

#include "script.h"

Scripts::Scripts(QObject *parent) :
    QObject(parent)
{
}

Scripts::~Scripts()
{
}

Script *Scripts::create(QString source)
{
    auto script = new Script(source, this);
    m_items.append(script);
    emit itemsChanged(m_items);
    return script;
}
