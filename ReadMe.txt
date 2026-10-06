QuarkTex NG 1.0 beta
====================

QuarkTex NG brings hardware 3D to AmigaOS in WinUAE: Warp3D, StormMESA (agl)
and MiniGL programs inside the emulated Amiga draw with the host's OpenGL on
Windows.

It is an independent continuation of QuarkTex, the 3D graphics
virtualization solution Robert Konrad first released in 2003.

License and source
------------------

QuarkTex (C) 2003-2012 Robert Konrad.
QuarkTex NG, changes and additions since 2026 (C) Serkan DURSUN.
Both are released under the GNU Lesser General Public License version 3
(License.txt), which builds on the GNU General Public License version 3
(COPYING). QuarkTex NG is a modified version of QuarkTex and is not made or
endorsed by its original author.

Source code: in Source/ of the Aminet archive, and at
https://github.com/Sdursun/QuarkTexNG
The WinUAE settings, performance figures and third-party notices are in
README.md, next to this file and on GitHub.

Installing
----------

- Copy quarktexng-windows-x86.dll and quarktexng-windows-x86-64.dll into the
  WinUAE directory (or its plugins directory). Older quarktex-windows-*.dll
  files are no longer used and can be deleted.
- Copy Warp3D.library, agl.library (for StormMESA) and minigl.library (for
  MiniGL) to LIBS: in AmigaOS.
- Required: Settings, Host, Miscellaneous: tick "Allow native code"
  (native_code=true in the configuration).
- Required: Settings, Hardware, CPU and FPU: FPU "CPU internal" with the
  precision "Host (80-bit)" (fpu_msvc_long_double=true). With the default
  64 bits games compute wrong values that a real 68k FPU does not: in JK2
  the first person weapon never shows, in RTCW it vanishes during play.
  minigl.library tells so in a requester, all three libraries in
  QuarkTexNGLog.txt.
- Recommended on the same page: a 68040 or 68060 at "Fastest possible",
  JIT on with the largest cache size (16 MB, cachesize=16384) and the JIT's
  "FPU support" on (compfpu=true). The games run on the emulated 68k; with
  the JIT FPU off, JK2 fell from 90 to about 15 fps.
- For fullscreen: an RTG board (UAE Zorro III). README.md on GitHub has
  all settings.

QuarkTex NG works in 32-bit (winuae.exe) and 64-bit (winuae64.exe) WinUAE; each
loads the DLL for its own architecture.

Building
--------

Everything is built in Docker; Docker and a POSIX shell (Git Bash on Windows)
are all that is needed:

  ./build.sh            builds everything and collects the release in dist/
  ./build.sh amiga      Warp3D.library and agl.library, and minigl.library
                        when MINIGL_SDK names MiniGL's SDK headers
                        (see docs/phase7-minigl.md) (m68k-amigaos-gcc)
  ./build.sh host       quarktexng-windows-x86.dll and -x86-64.dll (MinGW-w64)
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
