# Modify this if your locally compiled Frida isn't next to this directory
FRIDA = $$absolute_path("../")

win32 {
    win32-msvc*:contains(QMAKE_TARGET.arch, x86_64): {
        FRIDA_HOST = x64-Release
    } else {
        FRIDA_HOST = Win32-Release
    }
}
macx {
    FRIDA_BUILD = mac-x86_64
    FRIDA_HOST = mac-x86_64
}
linux {
    FRIDA_BUILD = linux-x86_64
    FRIDA_HOST = linux-x86_64
}

TEMPLATE = lib
TARGET = frida-qml
TARGETPATH = Frida
QT += qml quick
CONFIG += qt plugin create_prl c++11

TARGET = $$qtLibraryTarget($$TARGET)
QMAKE_MOC_OPTIONS += -Muri=$$TARGETPATH

# Input
SOURCES += \
    plugin.cpp \
    device.cpp \
    process.cpp \
    maincontext.cpp \
    frida.cpp \
    script.cpp \
    devicelistmodel.cpp \
    processlistmodel.cpp \
    iconprovider.cpp

HEADERS += \
    plugin.h \
    device.h \
    process.h \
    maincontext.h \
    frida.h \
    script.h \
    devicelistmodel.h \
    processlistmodel.h \
    iconprovider.h

OTHER_FILES = qmldir frida-qml.qmltypes

qmldir.files = qmldir
qmltypes.files = frida-qml.qmltypes
prlmeta.files = frida-qml.prl
win32:installPath = $${FRIDA}/build/frida-windows/$${FRIDA_HOST}/lib/qt5/qml/Frida
unix:installPath = $${FRIDA}/build/frida-$${FRIDA_HOST}/lib/qt5/qml/Frida
target.path = $$installPath
qmldir.path = $$installPath
qmltypes.path = $$installPath
prlmeta.path = $$installPath
INSTALLS += target qmldir qmltypes prlmeta

win32 {
    FRIDA_SDK_LIBS = \
        intl.lib \
        ffi.lib \
        z.lib \
        glib-2.0.lib gmodule-2.0.lib gobject-2.0.lib gthread-2.0.lib gio-2.0.lib \
        gee-0.8.lib \
        json-glib-1.0.lib

    INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/glib-2.0"
    INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/lib/glib-2.0/include"
    INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/gee-0.8"
    INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/json-glib-1.0"
    INCLUDEPATH += "$${FRIDA}/build/tmp-windows/$${FRIDA_HOST}/frida-core"

    LIBS += dnsapi.lib iphlpapi.lib ole32.lib psapi.lib shlwapi.lib winmm.lib ws2_32.lib
    LIBS += -L"$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/lib" $${FRIDA_SDK_LIBS}
    LIBS += -L"$${FRIDA}/build/tmp-windows/$${FRIDA_HOST}/frida-core" frida-core.lib
    QMAKE_LFLAGS_DEBUG += /LTCG /NODEFAULTLIB:libcmtd.lib
    QMAKE_LFLAGS_RELEASE += /LTCG /NODEFAULTLIB:libcmt.lib

    QMAKE_LIBFLAGS += /LTCG
}

!win32 {
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
    PKG_CONFIG = PKG_CONFIG_PATH=$${FRIDA}/build/sdk-$${FRIDA_HOST}/lib/pkgconfig:$${FRIDA}/build/frida-$${FRIDA_HOST}/lib/pkgconfig $${FRIDA}/build/toolchain-$${FRIDA_BUILD}/bin/pkg-config --define-variable=frida_sdk_prefix=$${FRIDA}/build/sdk-$${FRIDA_HOST} --static
    PKGCONFIG += frida-core-1.0
}

macx {
    QMAKE_CXXFLAGS = -Wno-deprecated-register
    QMAKE_LFLAGS += -Wl,-exported_symbol,_qt_plugin_query_metadata -Wl,-exported_symbol,_qt_plugin_instance -Wl,-dead_strip -Wl,-no_compact_unwind
}

linux {
    QMAKE_LFLAGS += -Wl,--version-script -Wl,frida-qml.version -Wl,--gc-sections
}
