# Phase 6: Warp3D on OpenGL 3.3 core

Status: in progress (2026-10-05). Warp3D draws on an OpenGL 3.3 core
profile context, pixel for pixel as the fixed-function renderer of phase 5;
the stencil buffer, the chroma test and several contexts at once work.

## Contexts

`createContext` (gl/gl.c) takes flags; the host gets them in d5
(`qt_create_context`, protocol version 7).

| Library | Context | Drawing |
| --- | --- | --- |
| Warp3D.library | OpenGL 3.3 core (`QT_CONTEXT_CORE`) | host/w3d.cpp through the emulation in host/ffp.cpp |
| agl.library | compatibility, as before | OpenGL 1.1 calls passed on |

agl.library is an OpenGL 1.1 implementation whose calls go to the host one
for one, so it needs the fixed-function pipeline of the driver. Warp3D does
not: the host decides how to draw it. A core context also checks that: any
OpenGL 1.1 call left in the Warp3D path is an error the driver reports,
and `qt_swap_buffers` logs errors.

The OpenGL 3.3 functions are loaded by host/gl3.cpp (declared in gl3.h, as
Visual Studio has no glext.h). The context is made with
`wglCreateContextAttribsARB`.

## The emulation (host/ffp.cpp)

w3d.cpp is unchanged in what it does: it still makes the OpenGL 1.1 calls
of phase 5, through `QT_GL(name)`, which now means `ffp::name`. The unit test
still points `QT_GL` at recording stubs, so its expectations stay as they
were. ffp implements those calls with their OpenGL 1.1 meaning:

- Immediate mode (`Begin`, `Vertex*`, `Color*`, `TexCoord*`, `End`, `Recti`)
  with the current colour and texture coordinate, into a stream vertex buffer.
- One shader with the parts of the fixed-function pipeline Warp3D uses:
  smooth or flat shading (`flat` with the last vertex as provoking vertex, as
  in OpenGL 1.1), the texture environments for an RGBA texture (replace,
  modulate, decal, blend), projective texture coordinates, fog (linear, exp,
  exp2, with the absolute eye z as fog coordinate), the alpha test. The 0.53
  model view matrix becomes a uniform, built the way `glScalef` and
  `glTranslatef` built it.
- Textures: the OpenGL 1.1 pixel formats without a core equivalent become red
  or red/green textures with a swizzle that gives the RGBA values OpenGL 1.1
  stored (`GL_ALPHA` 0, 0, 0, A; `GL_LUMINANCE` L, L, L, 1; ...). A texture
  is used only if it is complete, as in OpenGL 1.1 (an image, no mipmap
  filter).
- `GL_CLAMP`: the shader clamps the coordinate to 0..1 and the texture wraps
  `GL_CLAMP_TO_BORDER`, so linear filtering at the edge mixes in the border
  colour as OpenGL 1.1 does. With `GL_NEAREST` the clamp stops at the centre
  of the edge texel. (`GL_CLAMP_TO_EDGE` alone changed t11_texcolors.)
- Separate triangles, lines and points are batched while the state stays the
  same: one buffer upload and one draw call instead of one per Warp3D
  primitive. Every state change, every OpenGL command from the command
  buffer (`qt_w3d_sync`), the buffer swap and frame capture draw the batch
  first.

## The z-buffer commands

ZBuffer.c used `glRasterPos`, `glDrawPixels` and `glPushAttrib`, which the
core profile does not have. Two Warp3D commands replace them (gl/w3dcmd.h):

- `QT_W3D_READ_Z`: the host reads the depth buffer and writes `2 * depth - 1`
  into the application's `W3D_Double` array.
- `QT_W3D_WRITE_Z`: the host writes `(z + 1) / 2` with one point per pixel
  and `gl_FragDepth`, colour writes off, depth test `GL_ALWAYS`, and restores
  that state; with a mask only the masked pixels.

Both are synchronous. Warp3D.library no longer converts or allocates.

## Stencil buffer (roadmap item 5)

0.53 had no stencil buffer: `W3D_AllocStencilBuffer` and every other stencil
call returned an error, `W3D_Query` said "not supported". The host's context
always had 8 stencil bits, so Warp3D now uses them:

- `W3D_AllocStencilBuffer`/`FreeStencilBuffer` only mark the buffer as used;
  the other calls return `W3D_NOSTENCILBUFFER` without it, as in Warp3D.
- `W3D_SetState(W3D_STENCILBUFFER)` switches the stencil test,
  `W3D_SetStencilFunc`/`SetStencilOp`/`SetWriteMask` and
  `W3D_ClearStencilBuffer` become the commands `STENCIL_FUNC`, `STENCIL_OP`,
  `STENCIL_MASK`, `STENCIL_CLEAR`; the host maps the Warp3D values to
  OpenGL.
- `W3D_ReadStencilPixel`/`Span` (`READ_STENCIL`) read with `glReadPixels`.
  `W3D_WriteStencilPixel`/`Span` and `W3D_FillStencilBuffer` (8, 16 or 32
  bits per value; `WRITE_STENCIL`) draw one point per pixel with the
  stencil test passing always and `GL_REPLACE` by the value, colour writes
  and the depth test off, then restore the state (`ffp::StencilPoints`).
  Values are taken modulo 256 and the write mask applies, as they would with
  `glDrawPixels`.

Test: t15_stencil (new); against the snapshot before (`stencil0`) only it
changed, and it reads back 1; 1 1 2 2; 0 as drawn.

## Chroma test (roadmap item 5)

0.53 did not support it (`W3D_SetChromaTestBounds` returned
`W3D_UNSUPPORTED`, `W3D_SetState(W3D_CHROMATEST)` `W3D_UNSUPPORTEDSTATE`).
OpenGL never had one; the shader does it now:

- `W3D_SetChromaTestBounds` sends the texture's bounds and mode (`CHROMA`);
  `ffp::ChromaBounds` keeps them per texture, `W3D_CHROMATEST` switches the
  test for all textures (`ffp::ChromaTest`).
- The bounds are ARGB like the other Warp3D colours; red, green and blue are
  compared, alpha is ignored, both bounds included. The filtered texel is
  compared in 8 bits per channel, before the texture environment.
  `W3D_CHROMATEST_INCLUSIVE` keeps texels within the bounds,
  `W3D_CHROMATEST_EXCLUSIVE` rejects them, `W3D_CHROMATEST_NONE` keeps all.

Test: t16_chroma (new); against the snapshot before (`chroma0`) only it
changed.

## Several contexts (roadmap item 5)

0.53 and the host up to here had one context: one host window, OpenGL
context and emulation state, and on the Amiga side one window, position and
size in globals of each library. A second `W3D_CreateContext` replaced the
first on the 0.53 host (both drew into one OpenGL context with one state);
on the phase 6 host it failed, because the window class was registered
again while the first window still used it. Warp3D.library also chained its
ClipBlit patch onto itself for every context.

Now (protocol version 8):

- `qt_create_context` returns an id; `qt_execute` (d2), `qt_move_window`
  (d5), `qt_swap_buffers` and `qt_free_context` (d1) take it. The host keeps
  a window, an OpenGL context, an `ffp::Context`, a capture label and a
  profile per id and makes the OpenGL context and the emulation state
  current when the id changes (`activate`). The window class is registered
  while there is a window.
- gl/gl.c sends the buffer to the selected context; `selectContext` flushes
  when it switches, `createContext` selects the new context.
- Warp3D.library allocates a `QtContext` around each `W3D_Context` (host id,
  window, position, size, fullscreen, texture list); every command selects
  its context (`w3d_command(context, ...)`). The ClipBlit patch is installed
  with the first windowed context and removed with the last; it presents the
  context whose window the ClipBlit draws into, else the newest one, as 0.53
  presented its only context on any ClipBlit. The fullscreen patches are
  counted the same way. (0.53 also never reset its fullscreen flag, so after
  one fullscreen context every later one was taken as fullscreen.)
- agl.library keeps its host context and window in `amigamesa_context.gl_ctx`;
  `AmigaMesaMakeCurrent` selects it. `AmigaMesaDestroyContext` now also frees
  the context structure, which 0.53 left allocated.
- Frame capture: a context created while one with the same label exists is
  captured as `<label>-2` (and so on). compare.py reports a test listed in
  known-differences.txt as KNOWN also when the reference has no frame for it.

Test: t17_contexts (new): two windows, two Warp3D contexts drawing in turns
with different blending. Against the snapshot before (`multi0`, where the
second context could not be created) all other tests are unchanged.

## W3D_Query

0.53 answered `W3D_FULLY_SUPPORTED` to every query but the maximum texture
width and height (`W3D_QueryDriver` even to those), so applications turned
on what QuarkTex does not do. `W3D_Query` and `W3D_QueryDriver` now share
one table (Hardware.c `support`) of what the renderer does:

- not supported: specular highlights (the vertices' specular colour is not
  used), line and polygon stippling, antialiasing, dithering, volume
  textures, backface culling, and unknown queries;
- partially: `W3D_Q_INTERPOLATED` (drawn as exp2 fog);
- 2048 for the maximum texture sizes, also the perspective ones (0.53
  answered 3 there);
- fully supported: the rest.

Test: t18_query (new); against the snapshot before (`query0`) only it
changed. (Mipmapping was answered "not supported" here until the mipmaps
below; since then it is supported.)

## Mipmaps

Warp3D makes the mipmaps of a texture that the application does not supply.
QuarkTex made none, so a mipmap filter (`W3D_SetFilter(..., W3D_*_MIP_*)`)
left the OpenGL texture incomplete and drawing went on without it. The
emulation now makes them with `glGenerateMipmap` when such a texture is
drawn and its image changed since (`ffp.cpp texturing`). It also sets
`GL_TEXTURE_MAX_LEVEL` to the last level: with the default of 1000 the Intel
driver took the generated chain as incomplete, and the textures came out
black. `W3D_Q_MIPMAPPING` and `W3D_Q_MMFILTER` are now supported.

Test: t20_mipmap (new): 1-texel stripes drawn at their size and shrunk 4 and
8 times; the shrunk quads are an even purple. Against the snapshot before
(`mip0`, white quads) only t20 and the two squares of t18 changed.

## Real software (tests/run-app.ps1)

`tests/run-app.ps1` runs an Amiga program from a directory mounted as a
volume in the test WinUAE (68060, JIT, 512 MB), with the current libraries
and extra ones (such as a minigl.library) in LIBS:, and saves every n-th
frame (`QUARKTEX_CAPTURE_EVERY`), its output and free memory before and
after. Give it a copy: programs write their settings and logs. WinUAE's
directory volumes do not find files whose full Windows path is longer than
185 characters, so keep that copy's path short.

Tried with MiniGL Classic 27.0 (a minigl.library on Warp3D):

- Return to Castle Wolfenstein SP (rtcw-sp 1.0, the PiStorm3D port), started
  with `+exec amiga.cfg +set s_initsound 0 +map escape1`, winuae64.exe:
  the level loads and draws as on PiStorm3D: textured walls with lightmaps
  (blended in a second pass, as multitexture is off), models, HUD; the
  game's counter shows 59 fps at 640x480. Its texture mode is
  `GL_LINEAR_MIPMAP_LINEAR`, so without the mipmaps above nothing would be
  textured.

- OpenLara MiniGL 1.7 (Tomb Raider): the intro video with subtitles and the
  main menu draw. The 3D objects on the menu ring (the passport) look dark
  and untextured; not explained yet.
- Hurrican (classic MiniGL build), run from a drawer of a volume (it refuses
  the root): all textures load and it plays its demo level; HUD, dialogue
  boxes and part of the level tiles draw. Missing: the cracktro's logo and
  stars, the intro pictures, the level backgrounds. The trace
  (`-TraceFrame`) shows why: for those draws MiniGL Classic hands Warp3D a
  vertex array whose positions (`MGLVertex.bx..bw`) are not filled in (NaN);
  the screen coordinates are in the vertices' `W3D_Vertex` part. The
  `W3D_Context` layout is the same as in MiniGL's Warp3D.h (checked field by
  field), and QuarkTex 0.53 shows the same picture, so this is not a phase 6
  regression but a mismatch between this MiniGL path and QuarkTex's
  Warp3D. Open: whether Wazp3D or a real Warp3D driver reads those arrays
  differently.
- Found on the way, fixed since: in a window `W3D_ClearDrawRegion` drew a rectangle in
  the current state (as 0.53 did), so with blending on it did not clear,
  and divided the colour channels by 256. It is a real clear now, as in
  Warp3D (test t21_clear; against `clear0` only it changed). Hurrican looks
  the same with it: its missing parts come from the vertex arrays above.

## Leaks

An audit of the allocations in both libraries and the host found:

- `W3D_DestroyContext` left the textures the application had not freed
  allocated (the `W3D_Texture` and driver structures). It now calls
  `W3D_FreeAllTexObj` first. Ten create/destroy cycles with three textures
  left each lost at least 4 KB of Amiga memory before, less after.
- The host kept the palettes of CHUNKY textures in one map by OpenGL
  texture name for all contexts, and never removed the entries of destroyed
  contexts. With several contexts, whose texture names may be the same, a
  texture without a palette could take another context's one. The palette
  is now kept with the texture in the context's `ffp` data
  (`ffp::TexturePalette`) and goes with it. (The Intel driver used here
  gives every context different texture names, so the test cannot show the
  mix-up; it guards against it.)
- `AmigaMesaDestroyContext` did not free the context structure (fixed with
  the several contexts, above).

The rest is paired up; the 1.9 MB `drawmem` buffer of each Warp3D context
is not used by QuarkTex but freed with the context.

Test: t19_leaks (new): the cycles above, and a CHUNKY texture without a
palette next to a second context's texture with one. Against the snapshot
before (`leaks0`) only the memory square changed, from red to green.

## Results

All reference tests against the phase 5 snapshot (`run.ps1 -Against fix8`,
strict), on Intel Iris Xe Graphics, OpenGL 3.3.0 - Build 32.0.101.7088,
winuae.exe and winuae64.exe: 19 of 20 identical. t03_textures differs on
purpose: `W3D_I8` is drawn now. OpenGL 1.1 does not take `GL_INTENSITY` as a
pixel format, so that texture stayed empty and the quad white.

t09_throughput, time the host spends executing the command buffers
(`QUARKTEX_PROFILE=1`, winuae.exe with JIT, 60000 triangles):

| Renderer | Host time | Triangles/s (68k timer) |
| --- | --- | --- |
| phase 5, fixed function | 27-29 ms | 2.6-2.8 million |
| phase 6, core, without batching | (not measured with JIT) | 146000 without JIT, against 218000 |
| phase 6, core, batched | 10 ms | 5.9-6.3 million |

Without JIT the 68k side dominates and both renderers give about 210000
triangles/s.

## Next

- Roadmap item 4 also names texture format conversion in the shader
  (CLUT, R5G6B5, ...). Today CHUNKY textures are converted to RGBA on the
  CPU and the 16-bit formats are uploaded with packed types; both work, so
  this is an optimisation, not a fix.
- Roadmap item 5 is done.
