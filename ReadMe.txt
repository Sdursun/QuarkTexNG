QuarkTex is a 3D graphics hardware virualization solution first released in 2003.
It transforms calls to the Warp3D API inside an emulated AmigaOS to native OpenGL calls for a host Windows system.

Developed by Robert Konrad.
Released under the LGPL license.

Building
--------

Everything is built in Docker; Docker and a POSIX shell (Git Bash on Windows)
are all that is needed:

  ./build.sh            builds everything and collects the release in dist/
  ./build.sh amiga      Warp3D.library and agl.library (m68k-amigaos-gcc)
  ./build.sh host       QuarkTex.alib (32-bit MinGW-w64)
  ./build.sh generate   regenerates gl/*.auto.* after editing gl/glFuncs.txt

QuarkTex.alib can also be built with Visual Studio 2022. It has to be a
32-bit DLL and therefore runs only in 32-bit WinUAE:

  cmake -S . -B build/host -A Win32
  cmake --build build/host --config Release

Layout:
  Warp3D.library/   Warp3D 4 API on top of OpenGL (68k)
  agl.library/      StormMESA API on top of OpenGL (68k)
  gl/               68k bridge to the host OpenGL, generated from glFuncs.txt
  amiga/            library skeleton and build files for the 68k side
  QuarkTex.cpp      host DLL: window and OpenGL context inside WinUAE
