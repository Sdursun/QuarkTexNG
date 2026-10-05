# Phase 7: a QuarkTex minigl.library

Status: planned (2026-10-05). Stage 1, the analysis of the interface, is
done.

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
