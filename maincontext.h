#ifndef FRIDAQML_MAINCONTEXT_H
#define FRIDAQML_MAINCONTEXT_H

#include <frida-core.h>

#include <functional>

class MainContext
{
public:
    MainContext(GMainContext *mainContext);

    void schedule(std::function<void ()> f);

private:
    static gboolean performCallback(gpointer data);
    static void destroyCallback(gpointer data);

    GMainContext *m_mainContext;
};

#endif
