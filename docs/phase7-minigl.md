# Phase 7: a QuarkTex minigl.library

Status: in progress (2026-10-05). Stages 1 (analysis), 2 (skeleton) and 5
(the games) are done: RTCW, JK2, OpenLara and Hurrican run on it.

## Why

Games ported for PiStorm3D (RTCW, JK2, OpenLara, Hurrican) use the shared
`minigl.library`. Through MiniGL Classic they already run on QuarkTex's
Warp3D (RTCW at 59 fps, phase 6), but:

- MiniGL Classic transforms, lights and clips on the emulated 68k; a
  minigl.library that passes the OpenGL calls to the host leaves all of that
  to the PC's graphics card;
- some MiniGL Classic paths do not fit QuarkTex's Warp3D (Hurrican's
  vertex arrays, docs/phase6-core-renderer.md).

The MiniGL Readme expects one minigl.library per system (PiStorm3D,
classic Warp3D cards, a Radeon, an emulator passing calls to the host GL);
the same game binary runs on all of them.

## The interface (from the PiStorm3D SDK)

- `minigl.library` has one entry point after the standard four: LVO -30,
  `GetDispatch`, no arguments, returns a `MGLDispatchTable`
  (`libraries/minigl_dispatch.h`, `libraries/minigl_offsets.h`).
- The table: `abiVersion` (3), `structSize`, `backendFlags` (STUB 1,
  CLASSIC 2, PISTORM3D 4), `reserved`, `GLcontext *currentContext`, then
  155 function pointers: the GL functions with the context as first
  argument, MiniGL's own `MGL*`/`mgl*` functions, and entries appended
  since. Plain C calls with the arguments on the stack (gcc).
- New entries are only ever appended; the ABI version stays 3.
- The client side (`libminigl.a`, linked into each game, disassembled):
  `OpenLibrary("minigl.library", 14)`, calls LVO -30, and accepts the table
  only if `abiVersion > 2` and `structSize > 623` (at least 151 entries).
  A client built on a newer header asks for a larger `structSize`. Games
  may check more themselves (JK2: a library version; OpenLara:
  `MGLCreateContextFromWindow`).
- The 164 inline wrappers the games are compiled with only pass the
  context through the table; none dereferences it. Our context structure
  can be our own.
- MiniGL's GL constants are not OpenGL's: `mgl/gl.h` numbers them in one
  `enum` (253 names, a few with OpenGL values such as `GL_TEXTURE0_ARB`,
  and `GL_MAX_TEXTURE_UNITS_ARB = GL_FILL + 1` restarting the count), and
  the buffer bits are 1 (colour) and 2 (depth). Computed with the compiler,
  no two enum names share a value, so one table from MiniGL value to
  OpenGL value translates every enum argument (and query results back);
  it is generated from the header.

## Plan

1. Analysis (done).
2. Skeleton: `minigl.library` with `GetDispatch` and a table generated in
   header order; contexts (`MGLCreateContext`, `FromID`, `FromWindow`,
   screen and window, `MGLSwitchDisplay`, `MGLDeleteContext`, the
   `mgl*Choose*` settings, `mglGetSupportedScreenModes`, the GLUT-like main
   loop for the demos) on a host compatibility context without the 0.53
   model view matrix and with a 24-bit depth buffer; the GL functions
   through the existing command buffer, with the enum translation; agl's
   code where the conversion is already done (byte order, pixel data,
   vertex arrays). First picture: the MiniGL demos (`gears`, `ballonly`).
3. The rest of the table; MiniGL's extensions (packed pixels, colour
   tables / paletted textures, which OpenGL drivers do not have any more).
4. Vertex arrays drawn by the host from Amiga memory (as Warp3D's
   `DRAW_ARRAY`), not vertex by vertex from the 68k.
5. The games: RTCW, JK2, OpenLara, Hurrican, against MiniGL Classic on
   QuarkTex's Warp3D.

## Keeping up with new MiniGL releases

Games never need to be rebuilt for our library. When a game is built on a
newer header with appended entries, it refuses an older library; then the
table is regenerated from the new header and the new functions are added.
The SDK header is the single source for the table order and the enum
values, and a test checks the table against it.

## Building

minigl.library needs MiniGL's SDK headers, which are not part of QuarkTex
(they are under the Hyperion MiniGL Open Source License; QuarkTex is LGPL).
`MINIGL_SDK=<SDK include directory> ./build.sh` mounts them into the build
containers, runs `minigl.library/mglgen.py` and builds
`build/amiga/minigl.library`; without `MINIGL_SDK` the library is skipped.
The SDK's `mgl/context.h`, `mgl/vertexbuffer.h` and `mgl/minigl.h` include
backend headers the SDK does not ship; `minigl.library/sdk-shim/` replaces
them with the public types only (as the RTCW port's minigl-shim does).

## Stage 2 (done)

- `minigl.library/mglgen.py` generates the enum translation (252 values),
  the encoder declarations with plain C types (`qgl.auto.h`) and the
  dispatch table: 52 entries are generated wrappers, 96 hand-written
  (`context.c`, `glfuncs.c`), 5 log "not implemented" (interleaved arrays,
  edge flag and index pointers, blend equation and separate blend
  functions).
- Contexts: a screen of the asked size (`BestModeID`) with a backdrop window,
  or a window, or the application's window; `FromBitMap` returns NULL.
  The host context is a compatibility one without the 0.53 model view
  matrix and with a 24-bit depth buffer (`QT_CONTEXT_PLAIN`).
- Vertex arrays are read on the 68k and sent as immediate mode; float
  parameter arrays go as byte-swapped copies; `glGet*` results are turned
  around and constants translated back. One texture unit, no
  `GL_ARB_multitexture`, no paletted textures yet.
- `library_test` and `ballonly` (shared library clients from MiniGL
  Classic's archive) run: the Boing ball, 1 ms per frame by its own count.

## First game: RTCW

RTCW (rtcw-sp, the PiStorm3D port) runs on QuarkTex's minigl.library:
escape1 draws as through MiniGL Classic and Warp3D, 58 fps by the game's
counter (59 through Warp3D; both are bound by the emulation here, measured
in stage 4). The trace found one error per frame: the RTCW port defines
values of its own for GL names MiniGL lacks (`GL_CLIP_PLANE0 = 0x7A07`,
stencil, normal arrays) and MiniGL ignores them. The enum translation now
marks a value that is neither MiniGL's nor an OpenGL constant as unknown,
and the generated wrappers skip such a call (logged once per entry); since
then the game runs without OpenGL errors.

The trace (`QUARKTEX_TRACE_FRAME`) also checks OpenGL's error after every
OpenGL command of the traced frame outside glBegin/glEnd and logs the
command.

## Stage 5: the games (done)

All with `tests/run-app.ps1`, winuae64.exe with JIT, no OpenGL errors in the
host log, frame rates by each game's own counter:

| Game | Result | Through MiniGL Classic and Warp3D |
| --- | --- | --- |
| RTCW (rtcw-sp) | escape1 draws, 58 fps | the same picture, 59 fps |
| JK2 (jk2sp) | intro, crawl, cutscene, Kejim in third person, 90 fps | not tried |
| OpenLara MiniGL 1.7 | intro video, the Caves cutscene and Lara in the level, 49 fps | the menu; its 3D objects dark |
| Hurrican (classic build) | cracktro with logo and stars, menu, the demo level with all backgrounds | logo, intro pictures and backgrounds missing (phase 6) |

One fix came out of it: in fullscreen the host window was the whole emulator
display, but JK2 got its 640 x 480 screen in a 1024 x 768 display mode, which
shows the screen at its top left; the picture was drawn at the bottom left of
a 1024 x 768 window. The host window now has the screen's size at the top
left.

Hurrican's missing parts through MiniGL Classic (its vertex arrays, phase 6)
do not occur here: this library passes the application's arrays on as they
are (stage 4).

## Stage 4: vertex arrays on the host (done)

Measured first: the host logs a profile every 300 frames with
`QUARKTEX_PROFILE` (`tests/run-app.ps1 -Profile`): frame rate, and per frame
the time the host spent executing commands (with buffers and bytes), in
SwapBuffers, and elsewhere, which is the 68k. RTCW's 58 fps were its own
frame cap; with `+set com_maxfps 0` on escape1:

| | fps | executing | commands per frame | 68k per frame |
| --- | --- | --- | --- | --- |
| MiniGL Classic and Warp3D | 157 | 0.79 ms | 147 KB | 5.4 ms |
| this library, arrays as immediate mode from the 68k | 111 | 1.08 ms | 1.37 MB | 7.7 ms |
| this library, arrays read by the host | 188 | 0.81 ms | 23 KB | 4.3 ms |

The host was never the bottleneck: the 68k was, converting every element of
the arrays into glColor/glTexCoord/glVertex commands. glDrawArrays and
glDrawElements now send one command, `QT_MGL_DRAW` (`gl/mglcmd.h`), with the
mode, the indices and each enabled array's size, type, stride and address;
the host (`host/mgl.cpp`) reads the elements the call reaches from Amiga
memory, converts them from big-endian and draws them with OpenGL's own vertex
arrays. The call is synchronous, as the application may change its arrays as
soon as it returns. glArrayElement, called between glBegin and glEnd, still
goes through the 68k.
