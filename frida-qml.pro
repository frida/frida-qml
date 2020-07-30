TARGET = frida
TARGETPATH = Frida

CONFIG += qmltypes install_qmltypes
QML_IMPORT_NAME = Frida
QML_IMPORT_MAJOR_VERSION = 1

SOURCES = \
    src/plugin.cpp \
    src/device.cpp \
    src/process.cpp \
    src/maincontext.cpp \
    src/frida.cpp \
    src/script.cpp \
    src/devicelistmodel.cpp \
    src/processlistmodel.cpp \
    src/iconprovider.cpp

HEADERS = \
    src/plugin.h \
    src/device.h \
    src/process.h \
    src/maincontext.h \
    src/frida.h \
    src/script.h \
    src/devicelistmodel.h \
    src/processlistmodel.h \
    src/iconprovider.h

QT += quick

load(qml_plugin)

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
            libsoup-2.4.a

        INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/glib-2.0"
        INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/lib/glib-2.0/include"
        INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/gee-0.8"
        INCLUDEPATH += "$${FRIDA}/build/sdk-windows/$${FRIDA_HOST}/include/json-glib-1.0"
        INCLUDEPATH += "$${FRIDA}/build/tmp-windows/$${FRIDA_HOST}/frida-core/api"

        LIBS_PRIVATE += crypt32.lib dnsapi.lib iphlpapi.lib ole32.lib psapi.lib secur32.lib shlwapi.lib winmm.lib ws2_32.lib
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
}

!static {
    win32 {
        QMAKE_LFLAGS += /NODEFAULTLIB:libcmt.lib
    }

    macx {
        QMAKE_CXXFLAGS = -stdlib=libc++ -Wno-deprecated-register
        QMAKE_LFLAGS += -Wl,-exported_symbol,_qt_plugin_query_metadata -Wl,-exported_symbol,_qt_plugin_instance -Wl,-dead_strip
    }

    linux {
        QMAKE_LFLAGS += -Wl,--version-script -Wl,$${PWD}/frida-qml.version -Wl,--gc-sections -Wl,-z,noexecstack
    }
}
