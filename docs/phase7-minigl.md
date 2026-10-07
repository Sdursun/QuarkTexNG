# Phase 7: a QuarkTex minigl.library

Status: in progress. Stages 1 (analysis), 2 (skeleton) and 5 (the games) are
done: RTCW, JK2, OpenLara and Hurrican run on it. Stage 6 (2026-10-07) moved
the interface to MiniGL's 29 SDK (PiStorm3D 29.1).

## Stage 6: MiniGL's 29 SDK (done)

PiStorm3D's public 29.1 release changed MiniGL's interface from the 27 SDK
this library was built against (phase 7 stages 1-5): GL tokens carry OpenGL's
own values now instead of being auto-numbered, dispatch ABI 3 -> 5, 52 new
table entries (lighting, display lists as stubs, GLU quadrics and
`gluBuild2DMipmaps`, a GLUT subset), and `MGLResizeContext` returns whether it
succeeded. `MINIGL_SDK` now points at `SDK/minigl-shared-library/include` of
the PiStorm3D 29.1 archive (`SDK/README.txt` there); it is not part of
QuarkTex (Hyperion MiniGL Open Source License), same as before.

**A program built on the older (27) SDK is not supported.** This was tried
(a per-task table, filled from the version `OpenLibrary` asks for) and
dropped: that version never reaches the library. A real `OpenLibrary` call
was checked disassembled (`m68k-amigaos-objdump`) -- the custom `Open()`
vector is called with only A6 = library base; D0 holds nothing related to the
version, confirmed by two calls (asking 14 and 29) logging unrelated values
(12 and 24). Nothing else at `OpenLibrary` or `GetDispatchTable` time
distinguishes the two generations either (both are the same LVO calls, no
extra registers). The 29 SDK's own release notes describe the same break:
"I used a game for minigl.library V27... the game needs to get recompiled...
no source-code changes needed". `minigl.library/mglgen.py` now reads only the
29 SDK's `#define`s (real OpenGL values; MiniGL's private ones, 0x7000-0x7FFF,
still marked and skipped as unknown when an application passes one the host
does not take, as before).

Added, besides the enum change: `glLightfv`/`glMaterialfv`/`glLightModelfv`
and their `glGet*` (float vectors only -- `glLighti`/`glMateriali` are not in
the 29 table), `glNormalPointer` and `glNormal3fv` (vertex arrays and
`glDrawArrays`/`glDrawElements` gained a normal array, `QT_MGL_DRAW_WORDS` 26
-> 31; `glInterleavedArrays`' formats with a normal component fill it too),
`glColorMaterial`/`glTexEnvfv` (stubs: the host does not shade, so lighting
calls reach OpenGL but have no visible effect without a shader -- unchanged
from before, now just not silently dropped), display lists and
`glAreTexturesResident`-adjacent entries MiniGL does not actually implement
either (stubs, logged once), a GLU subset (quadrics drawn in immediate mode,
`gluBuild2DMipmaps` as `GL_GENERATE_MIPMAP` plus `glTexImage2D`) and the GLUT
subset `mgl/glut.h` declares (one window, a main loop sharing the context's
event handling with `MGLMainLoop`, game mode as a fullscreen context, the
solid shapes, `glutGet(GLUT_ELAPSED_TIME)` from `timer.device`). MiniGL 29's
thirteen NULL-vector entries (passed a NULL pointer, ignored rather than
dereferenced) are matched. Library version 27.0 -> 29.1 (`amiga/Makefile`;
the number is informational only, `MGLDispatchTable.structSize` is what a
client checks, per the SDK's own note on this).

Verified against `m01_minigl` (built with the 29 SDK's `MINIGL_VERSION`) in
WinUAE: ABI 5, 840-byte table, all 13 checks PASS; the reference test suite
(`tests/run.ps1`) is unaffected (0 FAIL, same PASS/KNOWN counts as before).

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

## Stage 3: the rest of the table (done)

No stub is left in the dispatch table.

- **GL_ARB_multitexture**, two units. glActiveTextureARB and
  glMultiTexCoord2fARB are host commands (`QT_MGL_ACTIVE_TEXTURE`,
  `QT_MGL_MULTI_TEX_COORD`), executed with OpenGL 1.3's functions; texture
  coordinate arrays are kept per unit and `QT_MGL_DRAW` carries both. The
  extension string names it as MiniGL does, `GL_MGL_ARB_multitexture`; Quake 3
  engine games look for `GL_ARB_multitexture` with strstr, find it, and draw
  lightmaps in the same pass (RTCW 208 and JK2 188 buffers per frame before,
  172 and 116 after).
- **Paletted textures** (`GL_EXT_color_table`, `GL_EXT_shared_texture_palette`)
  as MiniGL has them: one shared palette, set with glColorTable (RGB or RGBA,
  up to 256 entries), applied when a `GL_COLOR_INDEX` image is loaded. The
  68k expands the indices into RGBA for glTexImage2D and glTexSubImage2D,
  with the unpack alignment; changing the palette later does not change
  loaded textures, as in MiniGL. `GL_SHARED_TEXTURE_PALETTE_EXT` is only
  kept for glIsEnabled; the host driver has no such state.
- **glBlendEquation** and **glBlendFuncSeparate**: host commands with the
  OpenGL 1.2 and 1.4 functions (MiniGL's SDK defines the equations with their
  OpenGL values).
- **glInterleavedArrays**: OpenGL 1.1's 14 formats set up the vertex, colour
  and texture coordinate arrays; normals are skipped, as MiniGL has no normal
  array.
- **glEdgeFlagPointer**, **glIndexPointer**: accepted and ignored. Edge flags
  only matter to polygons drawn as lines or points and colour indices only in
  colour index mode, which MiniGL does not have.

MiniGL's packed pixel types are translated where OpenGL has them
(`MGL_UNSIGNED_SHORT_5_6_5`, `MGL_UNSIGNED_SHORT_4_4_4_4`); `GL_MGL_packed_pixels`
is not named in the extension string, since `MGL_UBYTE_ARGB` has no OpenGL
counterpart.

`tests/m01_minigl.c` checks all of it through the shared library interface,
with MiniGL's numbers for the constants, reading its pixels back: palette
entries with padded index rows, interleaved arrays, a subtracting blend
equation, two modulated units fed from arrays of each unit. It is built by
`./build.sh tests` when `MINIGL_SDK` is set and run with `tests/run-app.ps1`;
it prints PASS or FAIL per check (into `app.log`). The library of stage 4
fails all of them.
