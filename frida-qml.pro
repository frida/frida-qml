# Modify this if your locally compiled Frida isn't next to this directory
FRIDA = $$absolute_path("../")

TEMPLATE = lib
TARGET = frida-qml
QT += qml quick
CONFIG += qt plugin c++11

TARGET = $$qtLibraryTarget($$TARGET)
uri = re.frida.qmlcomponents

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

OTHER_FILES = qmldir

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}

win32 {
    win32-msvc*:contains(QMAKE_TARGET.arch, x86_64): {
        FRIDA_HOST = x64-Release
    } else {
        FRIDA_HOST = Win32-Release
    }
    INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/glib-2.0"
    INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/lib/glib-2.0/include"
    INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/gee-1.0"
    INCLUDEPATH += "$${FRIDA}/build/tmp-windows/$${FRIDA_HOST}/frida-core"
    LIBS += dnsapi.lib psapi.lib shlwapi.lib ws2_32.lib
    LIBS += -L"$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/lib"
    LIBS += intl.lib
    LIBS += ffi.lib
    LIBS += glib-2.0.lib gmodule-2.0.lib gobject-2.0.lib gthread-2.0.lib gio-2.0.lib
    LIBS += gee-1.0.lib
    LIBS += -L"$${FRIDA}/build/tmp-windows/$${FRIDA_HOST}/frida-core"
    LIBS += frida-core.lib
    QMAKE_LFLAGS_DEBUG += /LTCG /NODEFAULTLIB:libcmtd.lib
    QMAKE_LFLAGS_RELEASE += /LTCG /NODEFAULTLIB:libcmt.lib
}

!win32 {
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
    macx {
        FRIDA_BUILD = mac-x86_64
        FRIDA_HOST = mac-x86_64
    }
    linux {
        FRIDA_BUILD = linux-x86_64
        FRIDA_HOST = linux-x86_64
    }
    PKG_CONFIG = PKG_CONFIG_PATH=$${FRIDA}/build/sdk-$${FRIDA_HOST}/lib/pkgconfig:$${FRIDA}/build/frida-$${FRIDA_HOST}/lib/pkgconfig $${FRIDA}/build/toolchain-$${FRIDA_BUILD}/bin/pkg-config --define-variable=frida_sdk_prefix=$${FRIDA}/build/sdk-$${FRIDA_HOST} --static
    PKGCONFIG += frida-core-1.0
}

macx {
    QMAKE_CXXFLAGS = -Wno-deprecated-register
    QMAKE_LFLAGS += -Wl,-exported_symbol,_qt_plugin_query_metadata -Wl,-exported_symbol,_qt_plugin_instance -Wl,-dead_strip -Wl,-no_compact_unwind
}
