FRIDA_QML_BUILDDIR = $$top_builddir/qml/Frida
win32:LIBS_PRIVATE += $${FRIDA_QML_BUILDDIR}/frida-qml.lib
unix:LIBS_PRIVATE += -L$${FRIDA_QML_BUILDDIR} -lfrida-qml

unix {
    LIBS_PRIVATE += "-L$${FRIDA_CORE_DEVKIT}" -lfrida-core
}

macx {
    LIBS_PRIVATE += -lbsm -lresolv
}
