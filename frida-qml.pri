QMLPATHS += $$top_builddir/qml

unix {
    LIBS_PRIVATE += "-L$${FRIDA_CORE_DEVKIT}" -lfrida-core
}

macx {
    LIBS_PRIVATE += -lbsm -lresolv
}
