# frida-qml

Frida QML plugin for Qt >= 5.15.

## Binaries

Grab one from Frida's [releases][], and extract it in your Qt installation's
“qml” directory.

## Building

### From inside Frida's build system:

E.g. on macOS/x86_64:

    $ make core-macos-thin
    $ cd build/tmp_thin-macos-x86_64/
    $ mkdir frida-qml
    $ cd frida-qml
    $ qmake ../../../frida-qml
    $ make -j16
    $ make install

### Standalone

First, download a frida-core devkit from Frida's [releases][], and extract it to
e.g. `/opt/frida-core-devkit/`. Then:

    $ mkdir build-frida-qml
    $ cd build-frida-qml
    $ qmake FRIDA_CORE_DEVKIT=/opt/frida-core-devkit ../frida-qml
    $ make -j16
    $ make install


[releases]: https://github.com/frida/frida/releases
