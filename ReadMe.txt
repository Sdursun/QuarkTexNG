QuartexNG 1.0 beta
==================

QuartexNG brings hardware 3D to AmigaOS in WinUAE: Warp3D, StormMESA (agl)
and MiniGL programs inside the emulated Amiga draw with the host's OpenGL on
Windows.

It is the continuation of QuarkTex, the 3D graphics virtualization solution
first released in 2003, developed by Robert Konrad.
Released under the LGPL license.

Installing
----------

- Copy quartexng-windows-x86.dll and quartexng-windows-x86-64.dll into the
  WinUAE directory (or its plugins directory). Older quarktex-windows-*.dll
  files are no longer used and can be deleted.
- Copy Warp3D.library, agl.library (for StormMESA) and minigl.library (for
  MiniGL) to LIBS: in AmigaOS.
- Enable native code in WinUAE (native_code=true in the configuration).
- Required: set WinUAE's FPU to 80-bit precision. Settings, CPU and FPU,
  FPU: "Host (80-bit)" (fpu_msvc_long_double=true in the configuration).
  With the default 64 bits games compute wrong values that a real 68k FPU
  does not: in JK2 the first person weapon never shows, in RTCW it vanishes
  during play. minigl.library tells so in a requester, all three libraries
  in QuartexNGLog.txt.

QuartexNG works in 32-bit (winuae.exe) and 64-bit (winuae64.exe) WinUAE; each
loads the DLL for its own architecture.

Building
--------

Everything is built in Docker; Docker and a POSIX shell (Git Bash on Windows)
are all that is needed:

  ./build.sh            builds everything and collects the release in dist/
  ./build.sh amiga      Warp3D.library and agl.library, and minigl.library
                        when MINIGL_SDK names MiniGL's SDK headers
                        (see docs/phase7-minigl.md) (m68k-amigaos-gcc)
  ./build.sh host       quartexng-windows-x86.dll and -x86-64.dll (MinGW-w64)
  ./build.sh tests      the reference tests, see tests/README.md
  ./build.sh generate   regenerates gl/*.auto.* after editing gl/glFuncs.txt

The product name and version are set in one place, PRODUCT in amiga/Makefile.

The host libraries can also be built with Visual Studio 2022:

  cmake -S . -B build/host-x64 -A x64      (or -A Win32 for the x86 DLL)
  cmake --build build/host-x64 --config Release

Layout:
  Warp3D.library/   Warp3D 4 API on top of OpenGL (68k)
  agl.library/      StormMESA API on top of OpenGL (68k)
  minigl.library/   MiniGL's shared library interface on top of OpenGL (68k)
  gl/               68k bridge to the host, generated from glFuncs.txt
  amiga/            library skeleton and build files for the 68k side
  host/             host library: window and OpenGL context inside WinUAE
  tests/            reference tests run in WinUAE
  docs/             design notes
