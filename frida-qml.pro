TEMPLATE = lib
DESTDIR = $$DESTDIR/Frida
TARGET = $$qtLibraryTarget(frida-qml)
TARGETPATH = Frida
QT += qml quick
CONFIG += qt plugin create_prl c++11

QMAKE_MOC_OPTIONS += -Muri=$$TARGETPATH

include(frida-qml.pri)

QMLDIRFILE.input = $$PWD/qmldir.in
QMLDIRFILE.output = $$DESTDIR/qmldir

QMLTYPESFILE.input = $$PWD/frida-qml.qmltypes.in
QMLTYPESFILE.output = $$DESTDIR/frida-qml.qmltypes

QMAKE_SUBSTITUTES += QMLDIRFILE QMLTYPESFILE

qmldir.files = $$QMLDIRFILE.output
qmltypes.files = $$QMLTYPESFILES.output
prlmeta.files = frida-qml.prl
!isEmpty(FRIDA_CORE_DEVKIT) {
    installPath = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
} else {
    win32:installPath = $${FRIDA}/build/frida-windows/$${FRIDA_HOST}/lib/qt5/qml/Frida
    unix:installPath = $${FRIDA}/build/frida-$${FRIDA_HOST}/lib/qt5/qml/Frida
}
target.path = $$installPath
qmldir.path = $$installPath
qmltypes.path = $$installPath
prlmeta.path = $$installPath
INSTALLS += target qmldir qmltypes prlmeta

!static {
    macx {
        QMAKE_CXXFLAGS = -stdlib=libc++ -Wno-deprecated-register
        QMAKE_LFLAGS += -Wl,-exported_symbol,_qt_plugin_query_metadata -Wl,-exported_symbol,_qt_plugin_instance -Wl,-dead_strip
    }

    linux {
        QMAKE_LFLAGS += -Wl,--version-script -Wl,$${PWD}/frida-qml.version -Wl,--gc-sections -Wl,-z,noexecstack
    }
}
