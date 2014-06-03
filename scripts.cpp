#include "scripts.h"

#include "script.h"

Scripts::Scripts(QObject *parent) :
    QObject(parent)
{
}

Scripts::~Scripts()
{
}

Script *Scripts::createFromString(QString source)
{
    auto script = new Script(source, m_networkAccessManager, this);
    m_items.append(script);
    emit itemsChanged(m_items);
    return script;
}

Script *Scripts::createFromUrl(QUrl source)
{
    auto script = new Script(source, m_networkAccessManager, this);
    m_items.append(script);
    emit itemsChanged(m_items);
    return script;
}
