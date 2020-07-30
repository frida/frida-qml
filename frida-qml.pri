FRIDA_QML_BUILDDIR = $$top_builddir/qml/Frida
win32:LIBS_PRIVATE += $${FRIDA_QML_BUILDDIR}/declarative_frida.lib
unix:LIBS_PRIVATE += -L$${FRIDA_QML_BUILDDIR} -ldeclarative_frida
