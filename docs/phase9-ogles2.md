# Phase 9: a QuarkTex ogles2.library

Status: stage 2 done (2026-10-23): a GLSL ES shader compiles, a program
links, a vertex buffer uploads, and a real triangle draws and animates in
WinUAE, through the SDK's own `triangle` example unmodified.

## Why

`ogles2.library` (PiStorm3D family, by the ogles2 SDK's maintainers) gives
AmigaOS 3.x programs OpenGL ES 2.0, drawn by a Raspberry Pi 4's V3D GPU
reached through PiStorm and Emu68 (`device: v3d_init failed (-5: no
devicetree.resource (not running on Emu68?))` is the library's own message
when that hardware is not present, as in WinUAE). Like Warp3D and MiniGL,
this is a library whose only backend is a specific piece of real hardware;
QuarkTex NG replaces it with one that draws through the host's OpenGL, the
same way it does for the other three APIs.

GLES2 fits this host well: its tokens are already OpenGL's own values (no
MiniGL-style translation table needed) and it has no fixed-function
pipeline at all, so none of minigl/agl's `ffp.cpp`-style emulation is
needed either -- a GLES2 program supplies its own shaders, which the host's
OpenGL 3.3 core context (the same `QT_CONTEXT_CORE` Warp3D uses) compiles
directly, given `GL_ARB_ES2_compatibility` (checked by a desktop driver
supporting GLSL ES source, not yet verified here; stage 2).

## The interface (from the ogles2 SDK, `SDK/include`)

- `ogles2.library` has one public entry point after the standard four: LVO
  -30, `OGLES2GetDispatchTable`, no arguments, returns a
  `const struct OGLES2Dispatch *`. Confirmed by disassembling the SDK's
  `libogles2.a` (`ogles2_open.o`/`lvo.o`): `OGLES2Open()` opens the library,
  calls LVO -30, and only accepts the table if `abiVersion ==
  OGLES2_DISPATCH_ABI_VERSION` (1) and `structSize >=` the size its own
  header's `struct OGLES2Dispatch` has (164 entries, `OGLES2_DISPATCH_ENTRIES`).
- **A second, undocumented entry point: LVO -36.** `OGLES2Close()` (also in
  `libogles2.a`, `ogles2_open.o`) unconditionally calls it before
  `CloseLibrary` if the library is open, through a separate LVO stub
  (`_OGLES2PrivateLogFlushLVO` in `lvo.o`) that is not declared in any SDK
  header -- the name suggests the real library keeps a debug log and flushes
  it there. **Omitting this entry crashes the machine**: the call jumps into
  whatever happens to follow a library's function table in memory, taken
  as code (found by disassembling, since nothing in the SDK's own headers
  or docs mentions it; see "Debugging a reboot" below). `ogles2.fd` now has
  it as a second, no-op entry, `OGLES2PrivateLogFlush`.
- The table: `abiVersion`, `structSize`, `backendFlags`, `reserved`, then
  six `agl*` context functions (`aglCreateContext2`, `aglDestroyContext`,
  `aglMakeCurrent`, `aglSwapBuffers`, `aglSetParams2`, `aglSetBitmap`, each
  with the context as an explicit first argument -- unlike MiniGL's table,
  which the client's own inline wrappers also expose as implicit-context
  calls), then 158 `GL*` entries matching `<GLES2/gl2.h>` plus a few
  `OES`/`EXT` ones (`glMapBufferOES`, `glDiscardFramebufferEXT`, ...).
- The context type is `struct OGLES2Context`, forward-declared opaque in
  `<libraries/ogles2_dispatch.h>`; our library gives that same tag a real
  body (`ogles2.library/ogles2.h`), the same way MiniGL's own headers give
  `GLcontext`/`GLcontext_t` one.
- Calling convention: plain m68k-gcc stack-passed arguments throughout,
  confirmed by disassembly -- `_aglCreateContext2`/`_aglMakeCurrent` etc. in
  `libogles2.a` are tail calls (`unlk a5; jmp a1@`) that reuse the stack
  frame the *caller's* caller already set up, so the dispatch functions we
  write need no register-parameter annotations, just ordinary C signatures
  matching the header's parameter list exactly.

## Plan

1. Analysis and a minimal context (done): `ogles2.fd` (two entries),
   `ogles2.library/ogles2.h` (the real `struct OGLES2Context`, `GLdouble`/
   `GLclampd` typedefs GLES2 lacks but `gl/gldeclarations.auto.h` needs),
   `oglesgen.py` (reads `ogles2_dispatch.h`'s struct text like
   `minigl.library/mglgen.py` does MiniGL's, but with no enum table: GLES2's
   tokens need no translation). `aglCreateContext2` supports
   `OGLES2_CCT_WINDOW`; the context is `QT_CONTEXT_CORE` (Warp3D's flag),
   presented through `presentInto` exactly as Warp3D/minigl are. Twenty
   entries without a pointer argument are hand-written
   (`ogles2.library/glfuncs.c`), forwarding to the existing `_gl<Name>`
   encoders (`Clear`, `ClearColor`, `Viewport`, `Enable`, ... -- already in
   `gl/glFuncs.txt` from Warp3D's set); the other 138 are stubs, logged once
   if called (`ogles2_missing`).
2. Shaders, programs, buffers, framebuffers, textures: everything that
   takes a pointer. This needs the same kind of hand-written marshalling
   minigl.library's vertex arrays have (`host/mgl.cpp`), since Amiga memory
   is big-endian and the host reads it natively; `glShaderSource`'s array
   of C string pointers in particular has no equivalent anywhere yet. Most
   of `gl/glFuncs.txt`'s *existing* generic pointer-argument machinery
   cannot be reused as-is for this: it forwards a resolved host pointer
   straight to the real GL call with no byte-order handling, which is only
   safe for byte-granular data or where the driver itself interprets the
   data (checked against `gl/glgen.cpp`; `AreTexturesResident` is the one
   existing case of this, untested at scale).
3. `triangle`/`cube`/`boingocean` from the SDK's `SDK/examples` as the
   milestones, matching MiniGL's "first picture" tradition.

## Building

`ogles2.library` needs the ogles2 SDK's **root** directory (`SDK/`, holding
both `include/` and `lib/` -- `lib/` is only needed for `tests/o*.c`, which
link the SDK's own import library, `libogles2.a`, to use `ogles2.library`
exactly as a real program would); the SDK is not part of QuarkTex (its
license is not established here -- check before distributing).
`OGLES2_SDK=<SDK directory> ./build.sh` mounts it into the build containers
and runs `ogles2.library/oglesgen.py`; without it the library is skipped,
same as `MINIGL_SDK`.

## Stage 1 (done)

- `ogles2gen.py`: 164 entries, 6 context calls (hand-written), 20 further
  hand-written (no pointer argument), 138 stubs.
- `o01_ogles2` (`tests/o01_ogles2.c`), built against the real SDK's
  `<proto/ogles2.h>` and linked with `-logles2`: opens the library, creates
  a window context, clears to a colour for `FRAMES` (3) frames and swaps
  each time, reads the colour back through Picasso96 at five points.
  Verified live in WinUAE (not through `tests/run-app.ps1` yet, which needs
  `OGLES2_SDK` wiring of its own): **PASS, 0 failures**, the window shows
  the right colour, `glGetString(GL_RENDERER)` prints `QuarkTex NG ...
  ogles2.library (host OpenGL)`.
- Checking only after the first frame's swap failed every point by the
  window's own default fill colour (170, 170, 170): Intuition repaints a
  freshly opened window once, which a single swap loses the race against.
  `m01_minigl` already drew three frames before checking for the same
  reason; `o01_ogles2` now does too (`FRAMES` from `tests/window.h`).

## Stage 2 (done)

Shaders, programs, uniforms, buffers, vertex attributes and drawing
(gl/oglescmd.h, `host/ogles2.cpp`, `ogles2.library/glfuncs.c`):

- **Shaders and programs are real host OpenGL 2.0 objects** -- the same
  dynamically loaded set `host/ffp.cpp` already uses
  (`host/gl3.h`/`gl3.cpp`: `opengl32.dll` only exports OpenGL 1.1 statically,
  so `glCreateShader` and everything past it is fetched through
  `wglGetProcAddress` at runtime, not linked). Every entry here is scalar or
  takes `GLchar*` text (plain bytes, no order to convert), so they share a
  handful of generic opcodes by subop (`QT_OGLES2_SIMPLE`/`NAME`/`UNIFORM`/
  `GET_IV`/`GET_LOG`) instead of one each: `glCreateShader`/`DeleteShader`/
  `CompileShader`/`AttachShader`/`DetachShader`/`CreateProgram`/
  `DeleteProgram`/`LinkProgram`/`UseProgram`/`ValidateProgram`/`IsShader`/
  `IsProgram` (`SIMPLE`); `glBindAttribLocation`/`GetAttribLocation`/
  `GetUniformLocation` (`NAME`, a program/index plus one Amiga string
  address); `glUniform1f`..`4f`/`1i`..`4i` (`UNIFORM`); `glGetShaderiv`/
  `GetProgramiv` and `glGetShaderInfoLog`/`GetProgramInfoLog` (`GET_IV`/
  `GET_LOG` -- the host swaps the result into the Amiga address it is given
  itself, unlike the generic command buffer's raw pointer forwarding, see
  gl/glgen.cpp, which would leave a host-native value there). `glShaderSource`
  is its own opcode: one whole, NUL-terminated GLSL ES string (every known
  caller passes exactly that; several chunks or an explicit length are
  logged as missing).
- **Buffers are QuarkTex's own**, not real GL buffer objects: `glGenBuffers`
  hands out small integers and `glBufferData` keeps the bytes as given (still
  big-endian) in a `std::map<name, vector<uint8_t>>` on the host. A vertex
  attribute's element type is not known until `glVertexAttribPointer` names
  it, nor whether the application changes it again before the next draw, so
  converting to host order happens at `QT_OGLES2_DRAW` time, per attribute --
  the same reason minigl.library's vertex arrays convert at draw time rather
  than when the application hands them over (`host/mgl.cpp`).
- **`glVertexAttribPointer`/`glEnable`/`DisableVertexAttribArray` are 68k-side
  only** (`ogles2.library/glfuncs.c`): no opcode of their own, just local
  state (up to `QT_OGLES2_MAX_ATTRIBS`, 8); `glDrawArrays`/`glDrawElements`
  send all of it with the draw, the same shape minigl.library's own
  `QT_MGL_DRAW` uses for its arrays.
- **At draw time**, the host converts each enabled attribute's elements (from
  a fake buffer's bytes or straight from Amiga memory) into a flat host
  `float` array, **uploads it into a real, temporary host buffer object**
  (`glGenBuffers`/`BufferData`, deleted again right after the draw) and
  calls `glVertexAttribPointer` with an offset into *that*, not a bare
  client-side pointer. Plain client-side arrays (no `GL_ARRAY_BUFFER` bound)
  reliably hung this project's test machine's Intel Iris Xe driver's
  `glDrawArrays` outright under a core profile -- confirmed by bisecting with
  `logString()` calls (below) down to exactly that one call, with a correctly
  bound VAO (`ffp.cpp` already creates and binds one for every
  `QT_CONTEXT_CORE` context, Warp3D's included) and a provably correct
  pointer and size. Uploading into a real buffer object first is the
  ordinary modern-GL way regardless, and does not hang.
- Index buffers (`glDrawElements`) follow the same buffer-or-Amiga-address
  rule as vertex attributes; `GL_UNSIGNED_BYTE`/`SHORT` indices are read and
  widened to `GLuint` for the real `glDrawElements` call (GLES2 has no
  `GL_UNSIGNED_INT` index type itself, but the host's desktop GL does, and
  widening avoids a second, byte/short-sized temporary index buffer).

Verified with the SDK's own `SDK/examples/triangle.c`, built unmodified
against the real import library (`-logles2`) and copied to a live WinUAE: a
GLSL ES vertex and fragment shader compile and link, a two-attribute
(position, colour) interleaved vertex buffer uploads, and a smoothly
rotating, correctly gradient-shaded triangle draws every frame. The
Warp3D/agl reference suite (`tests/run.ps1`) is unaffected (0 FAIL).

## Debugging a reboot (the LVO -36 bug, 2026-10-23)

`aglSwapBuffers()` and everything before it worked on the first real run
(confirmed one step at a time with `logString()` calls checked against the
host's `QuarkTexNGLog.txt`, which survives even a full Amiga reset since it
is a Windows-side file -- more useful here than anything that could be
recovered from the Amiga side, since the whole machine restarted, not just
the task). `aglDestroyContext()` worked too. The machine reset with no
Guru and no alert code (`amiga_list alert` read `0xffffffff`, "no alert
since reset") right after, isolating it to `CloseWindow()` or
`OGLES2Close()` -- both otherwise ordinary calls nothing else in this
codebase does anything unusual with. Disassembling `libogles2.a`'s
`OGLES2Close()` (`m68k-amigaos-ar x`, `m68k-amigaos-objdump -d -b amiga`)
found the `jsr a6@(-36)` nothing in the SDK's headers names or documents.
