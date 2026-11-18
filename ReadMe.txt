QuarkTex NG 1.2 beta 1
======================

QuarkTex NG brings hardware 3D to AmigaOS in WinUAE: Warp3D, StormMESA
(agl), MiniGL, OpenGL ES 2.0 (ogles2) and OpenGL ES 3.0/3.1 (ogles3)
programs inside the emulated Amiga draw with the host's OpenGL on Windows.

ogles2.library and ogles3.library implement the interfaces of the PiStorm
libraries and SDKs by R-Typer and Ladislawv (github.com/ladislawv/ogles2,
github.com/ladislawv/ogles2pi3, github.com/ladislawv/ogles3).

New in 1.2 beta 1 (README.md has the details): BlitzQuake draws again
(Warp3D's z-buffer clear), ogles2.library 1.4, anisotropic filtering and
S3TC textures and multisampling (SetEnv OGLES2/MSAA 4) in ogles2 and
ogles3, no false FPU warning.

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
The notices of the parts made by others, among them the mingw-w64 runtime
in the host DLLs, are in ThirdParty.txt.

Source code: in Source/ of the Aminet archive, and at
https://github.com/Sdursun/QuarkTexNG
The WinUAE settings, performance figures and third-party notices are in
README.md, next to this file and on GitHub.

Installing
----------

- First run QuarkTexNGCheck.exe on the PC: it shows the graphics card,
  driver and OpenGL versions and says "QuarkTex NG support = PASS" if the
  driver can run QuarkTex NG (OpenGL 4.3 core profile), or FAIL and why.
  Double-clicked it opens a window; in a command prompt it prints the
  report.
- Copy quarktexng-windows-x86.dll and quarktexng-windows-x86-64.dll into the
  WinUAE directory (or its plugins directory). Older quarktex-windows-*.dll
  files are no longer used and can be deleted.
- Copy Warp3D.library, agl.library (for StormMESA), minigl.library (for
  MiniGL), ogles2.library (for OpenGL ES 2.0) and ogles3.library (for
  OpenGL ES 3.0/3.1) to LIBS: in AmigaOS. The libraries and DLLs of one
  release belong together.
- Required: Settings, Host, Miscellaneous: tick "Allow native code"
  (native_code=true in the configuration).
- Required: Settings, Hardware, CPU and FPU: FPU "CPU internal" with the
  precision "Host (80-bit)" (fpu_msvc_long_double=true). With the default
  64 bits games compute wrong values that a real 68k FPU does not: in JK2
  the first person weapon never shows, in RTCW it vanishes during play.
  The libraries write a warning into QuarkTexNGLog.txt.
- Recommended on the same page: a 68040 or 68060 at "Fastest possible",
  JIT on with the largest cache size (16 MB, cachesize=16384) and the JIT's
  "FPU support" on (compfpu=true). The games run on the emulated 68k; with
  the JIT FPU off, JK2 fell from 90 to about 15 fps.
- For fullscreen: an RTG board (UAE Zorro III). README.md on GitHub has
  all settings.

QuarkTex NG works in 32-bit (winuae.exe) and 64-bit (winuae64.exe)
WinUAE; each loads the DLL for its own architecture.

Building
--------

Everything is built in Docker; Docker and a POSIX shell (Git Bash on Windows)
are all that is needed:

  ./build.sh            builds everything and collects the release in dist/
  ./build.sh amiga      Warp3D.library and agl.library, minigl.library
                        when MINIGL_SDK names MiniGL's SDK headers
                        (see docs/phase7-minigl.md), ogles2.library
                        when OGLES2_SDK names the ogles2 SDK
                        (see docs/phase9-ogles2.md) and ogles3.library
                        when OGLES3_SDK names the ogles3 SDK
                        (see docs/phase10-ogles3.md) (m68k-amigaos-gcc)
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
  ogles2.library/   the OpenGL ES 2.0 interface of the ogles2 SDK by R-Typer
                    and Ladislawv, on OpenGL (68k)
  ogles3.library/   OpenGL ES 3.0/3.1 of the ogles3 SDK by R-Typer and
                    Ladislawv, built with ogles2.library's sources (68k)
  gl/               68k bridge to the host, generated from glFuncs.txt
  amiga/            library skeleton and build files for the 68k side
  host/             host library: window and OpenGL context inside WinUAE
  tests/            reference tests run in WinUAE
  docs/             design notes
