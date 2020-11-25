TEMPLATE = lib
CONFIG += qt plugin qmltypes
QT += quick

QML_IMPORT_NAME = Frida
QML_IMPORT_MAJOR_VERSION = 1

!isEmpty(_QMAKE_CONF_) {
    DESTDIR = $$shadowed($$dirname(_QMAKE_CONF_))/qml/$$QML_IMPORT_NAME
} else {
    DESTDIR = $$OUT_PWD/qml/$$QML_IMPORT_NAME
}
TARGET = frida-qml
TARGETPATH = Frida
QMAKE_MOC_OPTIONS += -Muri=$$TARGETPATH

SOURCES = \
    src/plugin.cpp \
    src/device.cpp \
    src/application.cpp \
    src/process.cpp \
    src/maincontext.cpp \
    src/frida.cpp \
    src/spawnoptions.cpp \
    src/script.cpp \
    src/devicelistmodel.cpp \
    src/applicationlistmodel.cpp \
    src/processlistmodel.cpp \
    src/iconprovider.cpp

HEADERS = \
    src/plugin.h \
    src/device.h \
    src/application.h \
    src/process.h \
    src/maincontext.h \
    src/frida.h \
    src/spawnoptions.h \
    src/script.h \
    src/devicelistmodel.h \
    src/applicationlistmodel.h \
    src/processlistmodel.h \
    src/iconprovider.h

PLUGINFILES = \
    qmldir

OTHER_FILES += $$PLUGINFILES

INCLUDEPATH += $$PWD/src

!isEmpty(FRIDA_CORE_DEVKIT) {
    INCLUDEPATH += "$${FRIDA_CORE_DEVKIT}"

    win32 {
        LIBS_PRIVATE += "$${FRIDA_CORE_DEVKIT}/frida-core.lib"
        QMAKE_LFLAGS += /IGNORE:4099
    }
    unix {
        LIBS_PRIVATE += "-L$${FRIDA_CORE_DEVKIT}" -lfrida-core
    }

    macx {
        LIBS_PRIVATE += -Wl,-framework,AppKit -lbsm -lresolv
    }
    linux {
        LIBS_PRIVATE += -ldl -lresolv -lrt -pthread
    }

    install_path = $$[QT_INSTALL_QML]/$$QML_IMPORT_NAME
} else {
    FRIDA = $$absolute_path("$$PWD/../")

    win32 {
        win32-msvc*:contains(QMAKE_TARGET.arch, x86_64): {
            FRIDA_HOST = x64-Release
        } else {
            FRIDA_HOST = Win32-Release
        }
    }
    macx {
        FRIDA_BUILD = macos-x86_64
        FRIDA_HOST = macos-x86_64
    }
    linux {
        FRIDA_BUILD = linux-x86_64
        FRIDA_HOST = linux-x86_64
    }

    win32 {
        FRIDA_SDK_LIBS = \
            libffi.a \
            libz.a \
            libglib-2.0.a libgmodule-2.0.a libgobject-2.0.a libgthread-2.0.a libgio-2.0.a \
            libgioschannel.a \
            libgee-0.8.a \
            libjson-glib-1.0.a \
            libpsl.a \
            libxml2.a \
            libsoup-2.4.a \
            libcapstone.a

        INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/glib-2.0"
        INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/lib/glib-2.0/include"
        INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/gee-0.8"
        INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/json-glib-1.0"
        INCLUDEPATH += "$${FRIDA}/build/tmp-windows/$${FRIDA_HOST}/frida-core/api"

        LIBS_PRIVATE += \
            advapi32.lib \
            crypt32.lib \
            dnsapi.lib \
            gdi32.lib \
            iphlpapi.lib \
            ole32.lib \
            psapi.lib \
            secur32.lib \
            shell32.lib \
            shlwapi.lib \
            user32.lib \
            winmm.lib \
            ws2_32.lib
        LIBS_PRIVATE += -L"$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/lib" $${FRIDA_SDK_LIBS}
        LIBS_PRIVATE += -L"$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/lib/gio/modules"
        LIBS_PRIVATE += -L"$${FRIDA}/build/tmp-windows/$${FRIDA_HOST}/frida-core" frida-core.lib
    }

    unix {
        QT_CONFIG -= no-pkg-config
        CONFIG += link_pkgconfig
        PKG_CONFIG = PKG_CONFIG_PATH=$${FRIDA}/build/sdk-$${FRIDA_HOST}/lib/pkgconfig:$${FRIDA}/build/frida-$${FRIDA_HOST}/lib/pkgconfig $${FRIDA}/build/toolchain-$${FRIDA_BUILD}/bin/pkg-config --define-variable=frida_sdk_prefix=$${FRIDA}/build/sdk-$${FRIDA_HOST} --static
        PKGCONFIG += frida-core-1.0
    }

    win32:install_path = $${FRIDA}/build/frida-windows/$${FRIDA_HOST}/lib/qt5/qml/$$QML_IMPORT_NAME
    unix:install_path = $${FRIDA}/build/frida-$${FRIDA_HOST}/lib/qt5/qml/$$QML_IMPORT_NAME
}

!static {
    win32 {
        QMAKE_LFLAGS += /NODEFAULTLIB:libcmt.lib
    }

    macx {
        QMAKE_LFLAGS += -Wl,-exported_symbol,_qt_plugin_query_metadata -Wl,-exported_symbol,_qt_plugin_instance -Wl,-dead_strip
    }

    linux {
        QMAKE_LFLAGS += -Wl,--version-script -Wl,$${PWD}/frida-qml.version -Wl,--gc-sections -Wl,-z,noexecstack
    }
}

target.path = $$install_path

pluginfiles_install.files = $$PLUGINFILES $$OUT_PWD/plugins.qmltypes
pluginfiles_install.path = $$install_path
pluginfiles_install.CONFIG += no_check_exist

pluginfiles_copy.files = $$PLUGINFILES $$OUT_PWD/plugins.qmltypes
pluginfiles_copy.path = $$DESTDIR

QMLTYPES_INSTALL_DIR = $$install_path

INSTALLS += target pluginfiles_install
COPIES += pluginfiles_copy

static {
    frida_qml_resources.files = qmldir
    frida_qml_resources.prefix = /qt-project.org/imports/Frida
    RESOURCES += frida_qml_resources
}
