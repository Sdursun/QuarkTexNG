QuarkTex is a 3D graphics hardware virualization solution first released in 2003.
It transforms calls to the Warp3D API inside an emulated AmigaOS to native OpenGL calls for a host Windows system.

Developed by Robert Konrad.
Released under the LGPL license.

Installing
----------

- Copy quarktex-windows-x86.dll and quarktex-windows-x86-64.dll into the
  WinUAE directory (or its plugins directory).
- Copy Warp3D.library (and agl.library for StormMESA) to LIBS: in AmigaOS.
- Enable native code in WinUAE (native_code=true in the configuration).

QuarkTex works in 32-bit (winuae.exe) and 64-bit (winuae64.exe) WinUAE; each
loads the DLL for its own architecture.

Building
--------

Everything is built in Docker; Docker and a POSIX shell (Git Bash on Windows)
are all that is needed:

  ./build.sh            builds everything and collects the release in dist/
  ./build.sh amiga      Warp3D.library and agl.library (m68k-amigaos-gcc)
  ./build.sh host       quarktex-windows-x86.dll and -x86-64.dll (MinGW-w64)
  ./build.sh tests      the reference tests, see tests/README.md
  ./build.sh generate   regenerates gl/*.auto.* after editing gl/glFuncs.txt

The host libraries can also be built with Visual Studio 2022:

  cmake -S . -B build/host-x64 -A x64      (or -A Win32 for the x86 DLL)
  cmake --build build/host-x64 --config Release

Layout:
  Warp3D.library/   Warp3D 4 API on top of OpenGL (68k)
  agl.library/      StormMESA API on top of OpenGL (68k)
  gl/               68k bridge to the host, generated from glFuncs.txt
  amiga/            library skeleton and build files for the 68k side
  host/             host library: window and OpenGL context inside WinUAE
  tests/            reference tests run in WinUAE
  docs/             design notes
