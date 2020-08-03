QMLPATHS += $$top_builddir/qml

win32 {
    LIBS_PRIVATE += "$${FRIDA_CORE_DEVKIT}/frida-core.lib"
}
unix {
    LIBS_PRIVATE += "-L$${FRIDA_CORE_DEVKIT}" -lfrida-core
}

win32 {
    QMAKE_LFLAGS += /INCLUDE:?qml_register_types_Frida@@YAXXZ
}
macx {
    QMAKE_LFLAGS += -Wl,-u,__Z24qml_register_types_Fridav
    LIBS_PRIVATE += -lbsm -lresolv
}
