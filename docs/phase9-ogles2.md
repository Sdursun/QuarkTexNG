# Phase 9: a QuarkTex ogles2.library

Status: stage 4 done (2026-10-25). All of GLES2 is implemented except
compressed textures, shader and program binaries and glClipControlEXT,
which stay logging stubs, and every surface of `aglCreateContext2`. The
SDK's four examples run unmodified in WinUAE (`triangle`, `cube`,
`boingocean` and `boing2026`), and so does the PiStorm port of Quake III
Arena with ioquake3's GLES2 renderer.

## Why

`ogles2.library` (PiStorm3D family, by R-Typer and Ladislawv) gives
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
after host/ogles2.cpp rewrites the GLSL ES source as GLSL 3.30 core
(stage 3).

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
  is its own opcode: one whole, NUL-terminated GLSL ES string. (Since
  stage 4 the library joins several chunks, or chunks with an explicit
  length, into one first.)
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

## Stage 2b: textures and framebuffers (done)

- Textures, framebuffers and renderbuffers are real host OpenGL objects,
  as shaders and programs are: `QT_OGLES2_GEN`/`QT_OGLES2_DELETE` (by kind)
  create and delete them. Their names come back as big-endian `GLuint`s.
  The scalar calls (`glBindTexture`, `glActiveTexture`, `glGenerateMipmap`,
  `glTexParameteri`, `glBindFramebuffer`, `glFramebufferTexture2D`,
  `glCheckFramebufferStatus`, renderbuffer storage and attachment) share one
  opcode, `QT_OGLES2_CALL`, and are selected by subop.
- `glTexImage2D` forwards `GL_UNSIGNED_BYTE` pixel data as given, because
  byte data has no byte order. A NULL pointer allocates a render target.
  Every other type is logged as missing for now.
- `glUniformMatrix4fv` swaps the floats on the host. `transpose` is not
  supported; GLES2 requires it to be `GL_FALSE`.

Verified live in WinUAE with two SDK examples, built unmodified:
`cube.c` (a textured, mipmapped, rotating cube with an MVP matrix uniform)
and `boingocean.c` (a render-to-texture reflection effect, 660 fps at
640x480). The reference suite (`tests/run.ps1`) still shows no FAIL.

## Stage 3: the rest of GLES2 (done)

The library now implements 152 of the 164 dispatch entries itself (plus
the six agl* context calls). Six remain logging stubs on purpose:
`glCompressedTexImage2D` and `glCompressedTexSubImage2D` (no compressed
formats are offered, and `GL_NUM_COMPRESSED_TEXTURE_FORMATS` is 0),
`glShaderBinary`, `glProgramBinaryOES` and `glGetProgramBinaryOES` (no
binary formats), and `glClipControlEXT` (OpenGL 4.5).

- **Shaders are translated.** The host compiles each GLSL ES 1.00 shader as
  GLSL 3.30 core. A shader with no `#version` line is GLSL 1.10, and only
  some drivers accept that in a core profile; Intel does, others need not.
  `attribute`, `varying`, `texture2D` and the other names GLSL 3.30 core
  dropped become `#define`s. `gl_FragColor` and `gl_FragData` become
  declared outputs. `#version` and `GL_OES_*` `#extension` lines are blanked,
  so the compiler's line numbers still match the application's source.
  `GL_ES` and the other `GL_` macros cannot be defined (the names are
  reserved), so a shader's `#ifdef GL_ES` parts drop out. In practice these
  are precision statements, which GLSL 3.30 ignores anyway.
  `glGetShaderSource` and `GL_SHADER_SOURCE_LENGTH` still return the
  application's own source.
- **Pixel formats.** `GL_ALPHA`, `GL_LUMINANCE` and `GL_LUMINANCE_ALPHA`
  are stored as `GL_RED`/`GL_RG` with a texture swizzle; core profiles do
  not have them. Pixel data in 16-bit types (`GL_UNSIGNED_SHORT_5_6_5`,
  `4_4_4_4`, `5_5_5_1`, and `GL_UNSIGNED_SHORT` depth) and 32-bit types
  (`GL_UNSIGNED_INT`, `GL_UNSIGNED_INT_24_8`) is byte-swapped by
  `glTexImage2D`, `glTexSubImage2D` and `glReadPixels`, honouring the pack
  and unpack alignment. `GL_RGB565` renderbuffers become `GL_RGB8`: desktop
  OpenGL only has `GL_RGB565` from 4.1 on.
- **Queries.** `glGetIntegerv`/`Floatv`/`Booleanv` write as many values as
  the pname has. GLES2-only pnames such as `GL_MAX_VARYING_VECTORS` are
  answered from their desktop equivalents. The buffer bindings and
  `GL_MAX_VERTEX_ATTRIBS` (8) are answered on the 68k side. Vertex attribute
  queries and `glGetShaderPrecisionFormat` are answered there too.
  `glGetUniform*v` finds the uniform's type through its active uniforms to
  know how many values to write.
- **Buffers.** `glBufferSubData` writes into a fake buffer.
  `glMapBufferOES` copies the buffer into 68k memory, and `glUnmapBufferOES`
  writes the whole copy back. `glDrawElementsBaseVertexOES` sends the base
  vertex in `QT_OGLES2_DRAW`'s otherwise unused `first` word.
- **Contexts** enable `GL_PROGRAM_POINT_SIZE`, because a GLES2 vertex shader
  always sets `gl_PointSize` itself.
- **Extensions offered:** `GL_OES_texture_npot`, `GL_OES_element_index_uint`,
  `GL_OES_depth_texture`, `GL_OES_standard_derivatives`, `GL_OES_depth24`,
  `GL_OES_packed_depth_stencil`, `GL_OES_rgb8_rgba8`, `GL_OES_mapbuffer`,
  `GL_OES_draw_elements_base_vertex` and `GL_EXT_discard_framebuffer`.

Verification:
- `tests/o02_ogles2_api.c` reads back the result of each group of new
  entries: queries, uniform arrays and matrices, the program queries,
  buffers and mapping, luminance and RGB565 textures, `glTexSubImage2D`,
  constant attributes, `glBlendColor` and the base vertex draw. All 37
  checks pass in WinUAE.
- `boing2026` (shadow maps in depth textures, MSAA framebuffers, bloom,
  derivatives) draws correctly at about 1300 fps at 640x480 on this
  project's test machine (Intel Iris Xe).
- `boingocean` now runs at about 1800 fps, up from 660.

**Several programs at once:** this was a limit of stage 3 and of every
QuarkTex library; fixed afterwards, see docs/multitasking.md.

## Stage 4: every context surface, and Quake III (done)

The PiStorm port of Quake III Arena (`Quake3-GLES2`, ioquake3's renderergl2
on ogles2.library) did not start in WinUAE, for three reasons:

- **The version.** It opens `ogles2.library` and wants 1.1 or newer; ours
  said 1.0. Its own drawer carries the PiStorm library in `libs/` and uses
  that one unless `LIBS:` has 1.3 or newer. The library implements the 1.3
  SDK's interface, so it is now 1.3 (`OGLES2_REVISION` in amiga/Makefile).
- **The surfaces.** `aglCreateContext2` only took `OGLES2_CCT_WINDOW`, and
  the game passes none (windowed) or `OGLES2_CCT_MODEID` (fullscreen):
  `OGLES2_CCE_BAD_TAGS`. All of the SDK's are there now
  (`ogles2.library/context.c`):
  - none of `WINDOW`, `BITMAP`, `MODEID`: a window of the library's own on
    the default public screen, 640 x 480 or `OGLES2_CCT_PISTORM_WIDTH`/
    `_HEIGHT`, without a close gadget or IDCMP;
  - `OGLES2_CCT_MODEID` (or `_CONTEXT_FOR_MODEID`): a 24-bit screen in that
    mode, of its size or the PiStorm size tags, with a backdrop window for
    input, as minigl.library's fullscreen contexts;
  - `OGLES2_CCT_BITMAP`: the program's bitmap, written at each
    `aglSwapBuffers` (only a Picasso96 bitmap the host can write:
    `OGLES2_CCE_NO_DISPLAY` otherwise); `aglSetBitmap` moves to another
    one, after writing the open frame into the old;
  - `OGLES2_CCT_PISTORM_GET_WINDOW` answers with the window that gets
    input; `aglSetParams2` takes a new or resized program window, answers
    the size and window tags and ignores `OGLES2_CCT_VSYNC`.
  `aglDestroyContext` closes the window and screen the library opened.
- **`glShaderSource` with lengths.** ioquake3 passes each shader with an
  explicit length.

Verification, in the user's WinUAE with the library loaded from RAM:, not
installed in `LIBS:`:
- Quake III windowed: the menu and a game in the library's window; all 95
  GLSL shaders compile; the renderer restart at start-up (a context
  destroyed and made again) works.
- `timedemo four` at 640 x 480: 79.0 fps windowed, 83.6 fps fullscreen
  (the README of the PiStorm build gives 80.4 on an A1200 with a Pi 4).
  The fullscreen screen shows the game; both quit cleanly (`+set nextdemo
  quit`).
- `o02_ogles2_api` passes; `boingocean` runs in a window and with its
  `FULLSCREEN` switch (a `MODEID` context).

Breaking the game with Ctrl-C in the middle of play reset the Amiga, with no
alert recorded; a normal quit does not. Not looked into yet.

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
