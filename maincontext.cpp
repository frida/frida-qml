#include "maincontext.h"

MainContext::MainContext(GMainContext *mainContext) :
    m_mainContext(mainContext)
{
}

void MainContext::schedule(std::function<void ()> f)
{
    auto source = g_idle_source_new();
    g_source_set_callback(source, performCallback, new std::function<void ()>(f), destroyCallback);
    g_source_attach(source, m_mainContext);
    g_source_unref(source);
}

gboolean MainContext::performCallback(gpointer data)
{
    auto f = static_cast<std::function<void ()> *>(data);
    (*f)();
    return FALSE;
}

void MainContext::destroyCallback(gpointer data)
{
    auto f = static_cast<std::function<void ()> *>(data);
    delete f;
}
