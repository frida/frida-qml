# Modify this if your locally compiled Frida isn't next to this directory
FRIDA = $$absolute_path("../frida")

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
    processlistmodel.cpp

HEADERS += \
    plugin.h \
    device.h \
    process.h \
    maincontext.h \
    frida.h \
    script.h \
    devicelistmodel.h \
    processlistmodel.h

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

QMAKE_CXXFLAGS = -Wno-deprecated-register

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
    QMAKE_LFLAGS += -Wl,-dead_strip -Wl,-no_compact_unwind
}
