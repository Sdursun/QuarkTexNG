# Phase 10: a QuarkTex ogles3.library

Status: every entry of the table written (2026-11-14). OpenGL ES 2.0, 3.0
and 3.1 contexts run, OpenGL ES 3.1 complete: all four demos of the SDK
draw correctly, among them the compute shader ones (`boingswarm-ogles3`,
`juggler-ogles3`). Compressed textures are ETC1 and ES 3.0's ETC2/EAC; no
program binary formats are offered; GL_KHR_debug gives the application
only its own messages (see "The last entries").

## Why

`ogles3.library` (github.com/ladislawv/ogles3, by R-Typer and Ladislawv) is
the successor of the PiStorm `ogles2.library`: OpenGL ES 3.1 on a Raspberry
Pi 4's V3D GPU, through PiStorm and Emu68. As with ogles2 (phase 9),
QuarkTex NG gives programs built against its SDK a library of the same name
that draws with the host's OpenGL. ogles3 is a library of its own, beside
ogles2: programs written for ogles2 keep using that one.

The SDK used here is ogles3 1.0-beta2 (`ogles3-1.0-beta2-sdk.zip` from the
repository's `release/dist/`); the library is still a beta, and its
interface may change before its release.

## The interface (ogles3 SDK, `include/`)

- The same shape as ogles2's: LVO -30 returns the dispatch table, LVO -36
  is called by `OGLES3Close()` before `CloseLibrary` (the SDK's
  `client/ogles3_open.c` and `client/lvo.s`). `OGLES3Open()` takes the table
  if `abiVersion` is `OGLES3_DISPATCH_ABI_VERSION` (1) and `structSize` at
  least its own.
- `struct OGLES3Dispatch` has 380 entries: ogles2's 164 first, in the same
  order and with the same signatures, then OpenGL ES 3.0's and 3.1's and
  their extensions'.
- The context tags and error codes have ogles2's values (`<libraries/
  ogles3.h>` also keeps the `OGLES2_` names). A context is OpenGL ES 3.1
  unless `OGLES3_CCT_PISTORM_ES_VERSION` asks for 20; 30 also gives 3.1,
  the highest the library has, as the PiStorm library 1.0 does (its
  `ogles3.doc` and its source; the comment in `<libraries/ogles3.h>` still
  says 20 is the default). On a host without OpenGL 4.3 it is 3.0, and only
  31 is refused. `GL_VERSION` reads "OpenGL ES 3.1 ogles3.library 1.0", as
  the PiStorm one does (2026-11-15, with release 1.0's SDK, whose interface
  is beta2's).

## The library

`ogles3.library` is built from `ogles2.library`'s sources (`amiga/Makefile`,
`OGLES3_*`): `context.c` and `glfuncs.c` compiled again with `QT_OGLES3`
and the ogles3 SDK's headers (`ogles2.h` maps the context and dispatch
table names), with `ogles3.library/lib.c` for the library's own entry
points and the files of `ogles3.library/` for the entries ES 3.x adds.
`ogles2.library/oglesgen.py --ogles3` generates its table from
`ogles3_dispatch.h`; the entries neither library implements yet are
logging stubs.

Building needs the SDK: `OGLES3_SDK=<sdk>/ogles3-sdk-1.0-beta2 ./build.sh
amiga`.

## Stage 1 (done)

- The library, its table (380 entries: the 6 context calls, ogles2's 152,
  222 stubs) and `OGLES3_CCT_PISTORM_ES_VERSION`: 20 (the default) gives an
  ES 2.0 context, 30 an ES 3.0 one (so far only `glGetString` differs), 31
  `OGLES3_CCE_UNSUPPORTED`, anything else `OGLES3_CCE_BAD_TAGS`.
- Verified: `o02_ogles2_api` built against the ogles3 SDK (its include
  changed to `<proto/ogles3.h>`, as the SDK's documentation says for
  ogles2 sources, linked with `-logles3`) passes all its checks with
  ogles3.library in the test WinUAE.

## Stage 2: OpenGL ES 3.0 (done, but transform feedback and queries)

Protocol 12 (`gl/oglescmd.h`): libraries and host DLLs of protocol 11 do
not work with it.

- **GLSL ES 3.00** sources (`#version 300 es`) are compiled as GLSL 3.30
  core with only the `#version` line changed: `in`/`out`, `layout
  (location)`, uniform blocks, `texture()` and precision statements are
  the same there (`host/ogles2.cpp`, `coreSource`).
- **Attributes:** 16 (`QT_OGLES2_MAX_ATTRIBS`, ogles2.library's too), each
  with a divisor (`glVertexAttribDivisor`) and an integer flag
  (`glVertexAttribIPointer`: converted to 32-bit ints and given to
  `glVertexAttribIPointer` on the host, so the shader gets them as they
  were). A per-instance attribute's converted range is its instances',
  not the indices'. `QT_OGLES2_DRAW` carries the instance count:
  `glDrawArraysInstanced`, `glDrawElementsInstanced` and their EXT forms.
- **Vertex array objects** are 68k-side (`ogles3.library/es3.c`), as the
  attributes are: `glBindVertexArray` copies the live attribute state and
  element buffer out to the object left and the new one's in.
- **Uniform buffers:** `glBindBufferBase/Range(GL_UNIFORM_BUFFER)` names a
  fake buffer to the host (`QT_OGLES2_BIND_BUFFER_RANGE`). std140 data is
  all 32-bit words, so the host keeps the buffer's words swapped in a real
  buffer, with the converted-copy machinery of the vertex attributes
  (only what changed is converted again), and binds it before each draw.
  `glCopyBufferSubData` copies between fake buffers; `glMapBufferRange`
  is a 68k-side copy as `glMapBufferOES` is (array and element buffers).
- **Textures:** `glTexImage3D`/`glTexSubImage3D`/`glCopyTexSubImage3D`;
  `glTexStorage2D/3D` as `glTexImage*` per level without data (OpenGL 4.2
  has `glTexStorage`; the host asks for 3.3), the levels cut off with
  `GL_TEXTURE_MAX_LEVEL`, sized formats given a matching format and type
  (integer, depth, depth-stencil). The ES 3.0 pixel types are swapped:
  `GL_HALF_FLOAT`, `GL_FLOAT`, `GL_(UNSIGNED_)BYTE/SHORT/INT` and the
  packed 32-bit ones.
- **Samplers** and queries are real host objects (`QT_OGLES2_GEN`'s
  kinds); `glBindSampler`, `glSamplerParameter*`.
- **Framebuffers:** `glDrawBuffers`, `glReadBuffer`, `glBlitFramebuffer`,
  `glRenderbufferStorageMultisample`, `glFramebufferTextureLayer`,
  `glClearBuffer*`; `glInvalidate(Sub)Framebuffer` are hints and do
  nothing. ES 3.0 contexts offer `GL_EXT_color_buffer_float` and
  `_half_float`, which desktop OpenGL 3.3 has: boing2026-ogles3 uses float
  targets for its pool only with them.
- **The rest:** `glUniform*ui(v)`, `glUniformMatrix{2x3,...}fv` and
  transposed matrices, `glGetUniformuiv`, `glGetFragDataLocation`,
  `glGetUniformBlockIndex`, `glUniformBlockBinding`, `glGetStringi`,
  `glGetInteger64v`, `glVertexAttribI4*`. Sync objects are emulated: the
  host runs each command buffer through before the 68k goes on, so
  `glClientWaitSync` is `glFinish`.
- Still stubs: transform feedback, queries (`glBeginQuery`...),
  `glGetActiveUniformBlockiv/Name`, `glGetUniformIndices`,
  `glGetActiveUniformsiv`, `glGetIntegeri_v`, `glGetInternalformativ`,
  program binaries, compressed 3D textures, the ES 3.1 entries.
- `QUARKTEX_CHECK_ERRORS` (`tests/run-app.ps1 -CheckErrors`) logs an
  OpenGL error with the command that left it.

Verification (the test WinUAE of `tests/run-app.ps1`):
- `tests/e01_ogles3_api.c`, 29 checks read back from the framebuffer:
  queries, a GLSL ES 3.00 program, two vertex array objects with their own
  index buffers, integer attributes, a uint uniform, instancing with a
  divisor, a uniform block changed by `glBufferSubData`, `glMapBufferRange`
  and `glCopyBufferSubData`, `glTexStorage2D` through a sampler object, a
  3D texture, an RGBA16F target cleared with `glClearBufferfv` and
  blitted. All pass.
- The SDK's demos (`ogles3-1.0-beta2-demos.zip`, run unchanged):
  `boingocean-ogles3` (HDR sea, about 390 fps at 640 x 480) and
  `boing2026-ogles3` (the pool: waves, caustics and refraction in float
  targets, about 210 fps) draw correctly.
- ogles2.library at protocol 12: `o02_ogles2_api` passes (its
  `GL_MAX_VERTEX_ATTRIBS` check is now "at least 8"), the ogles2 SDK's
  `boingocean`/`boing2026` and Quake III's `timedemo four` (84.3 fps) as
  before; `m01_minigl` and the Warp3D/agl reference suite unchanged.

Seen on the way: the frame capture of the tests (`captureFrame`), and the
presenting of a frame into Amiga memory, read from whatever framebuffer is
bound for reading; a program's framebuffer 0 is the host window's own, not
the offscreen context's framebuffer object. Intel's driver lets that
work, and the capture of a frame whose bound read buffer is `GL_NONE`
leaves a `GL_INVALID_OPERATION` (logged at the swap). To look into.

## The host on OpenGL 4.3 (2026-11-10)

OpenGL ES 3.1 needs desktop OpenGL 4.3 (compute shaders, storage buffers,
images, indirect draws), so the host's core contexts ask for 4.3 now
(`host/gl3.cpp`), and 3.3 only if the driver has no 4.3; then the log says
so, and everything but ES 3.1 still runs. The 4.3 entries are a list of
their own (`QT_GL43_FUNCTIONS`, `gl3::has43()`). `QuarkTexNGCheck.exe`
(`tools/check`) says PASS for 4.3. The test machine's Iris Xe has 4.6;
Warp3D, agl, ogles2 and ogles3 give the same results in a 4.3 context.

## Stage 2b: the rest of OpenGL ES 3.0 (done)

- **Transform feedback:** `glBindBufferBase/Range(GL_TRANSFORM_FEEDBACK_
  BUFFER)` names a fake buffer to the host, as for uniform buffers. At
  `glBeginTransformFeedback` the host binds a host buffer per fake buffer,
  filled with its words swapped (so what is not written stays as it was);
  at `glEndTransformFeedback` it copies the bound ranges back into the
  fake buffers, every 32-bit word swapped (the outputs are all 32-bit),
  recorded as changes, so the attribute copies follow and `glMapBuffer-
  Range` reads the result. `glTransformFeedbackVaryings`, `glGetTransform-
  FeedbackVarying`, pausing and the transform feedback objects (OpenGL 4.0)
  too.
- **Queries:** real host objects (`glBeginQuery`/`glEndQuery`,
  `glGetQueryiv`, `glGetQueryObjectuiv`, the EXT forms).
- **Uniform block queries:** `glGetActiveUniformBlockiv` (the active
  uniform indices as many as the block has), `glGetActiveUniformBlockName`,
  `glGetUniformIndices` and `glGetActiveUniformsiv` (their name and index
  arrays read from Amiga memory).
- `glGetIntegeri_v` (and the 64-bit and boolean forms) answers the uniform
  and transform feedback buffer bindings with the fake buffers' names and
  ranges, `glGetBufferParameteri64v`, `glGetSamplerParameter*`,
  `glGetVertexAttribI*`, `glGetInternalformativ` (4 samples), no program
  binary formats. `glMapBufferRange` maps any buffer target.
- ogles3.library's table takes every `ogles2_GL<Name>` its own files
  define (`oglesgen.py`): 273 entries written, 101 stubs (ES 3.1's and the
  rest of the extensions).

Verification: `tests/e02_ogles3_queries.c` (19 checks: varyings captured
and read back with `glMapBufferRange`, an occlusion query of a drawn and a
culled quad, the block queries against std140 offsets, the indexed
binding queries) and `e01` pass with no OpenGL error
(`QUARKTEX_CHECK_ERRORS`); the demos, `o02` and the reference suite as
before.

## Stage 3a: OpenGL ES 3.1 compute (done)

- **ES 3.1 contexts:** `OGLES3_CCT_PISTORM_ES_VERSION` 31 opens one when
  the host's core contexts are OpenGL 4.3 (`QT_OGLES2_C_Has43`), else
  `OGLES3_CCE_UNSUPPORTED`. `glGetString` answers "OpenGL ES 3.1" and
  "OpenGL ES GLSL ES 3.10"; `GL_MAJOR_VERSION`/`GL_MINOR_VERSION` and the
  generic buffer bindings are answered on the 68k (fake names).
- **GLSL ES 3.10** (`#version 310 es`) is compiled as GLSL 4.30 core, only
  the `#version` line changed: compute shaders, `std430` buffers, images,
  atomic counters and `binding` layouts are the same there.
- **Shader storage and atomic counter buffers** are fake buffers bound by
  `glBindBufferBase/Range` as uniform buffers are: before each dispatch
  and draw the host binds their word copy (32-bit words swapped; std430
  and counters have no other kind). After a dispatch or draw those
  buffers are marked written by the GPU; the word copy is then newer than
  the bytes, and is read back (every word swapped, recorded as a change)
  before anything else reads or writes the bytes: `glMapBufferRange`,
  `glBufferSubData`, `glCopyBufferSubData`, an index buffer, an attribute
  copy, transform feedback. So a compute shader can write vertices, and
  the 68k can read what it computed. `glBufferData` with data drops the
  mark.
- `glDispatchCompute`; `glDispatchComputeIndirect`, `glDrawArraysIndirect`
  and `glDrawElementsIndirect` read their command on the 68k from the
  buffer bound to `GL_DISPATCH_INDIRECT_BUFFER`/`GL_DRAW_INDIRECT_BUFFER`
  (read back first if the GPU wrote it) and run it as a direct one
  (`baseVertex` is not offered). `glBindImageTexture`, `glMemoryBarrier`;
  `glMemoryBarrierByRegion` is the whole barrier (OpenGL 4.5 has the
  other).
- `glMapBufferRange` maps the four ES 3.1 buffer targets too (12 slots).

Verification: `tests/e03_ogles31_compute.c` (10 checks: the ES 3.1 context
and its version, a compute shader filling a storage buffer and counting
with an atomic counter, both read back with `glMapBufferRange`, an indirect
dispatch, `imageStore` read through a framebuffer, vertices a compute
shader wrote drawn by `glDrawArraysIndirect`) passes with no OpenGL error.
The SDK's ES 3.1 demos run unchanged: `boingswarm-ogles3` (16384 boids
simulated by compute shaders, about 85 fps at 640 x 480) and
`juggler-ogles3` (the 1987 Juggler ray traced by a compute shader, about
184 fps). `e01` (its ES 3.1 context check now expects one to open), `e02`,
`o02` and the reference suite as before.

## Stage 3b: the rest of OpenGL ES 3.1 (done)

- **Separate vertex attribute formats and bindings** (`glVertexAttrib-
  Format`, `glVertexAttribIFormat`, `glVertexAttribBinding`, `glBind-
  VertexBuffer`, `glVertexBindingDivisor`): 68k-side state, as the
  attributes are (`ogles2.library/ogles2.h`). An attribute keeps its
  format, relative offset and the binding it reads; 16 bindings keep
  buffer, offset, stride and divisor; vertex array objects keep both. What
  `QT_OGLES2_DRAW` sends is unchanged: each attribute's buffer, pointer,
  stride and divisor, made of the two whenever one changes
  (`ogles2_bindingChanged`). `glVertexAttribPointer`, `glVertexAttrib-
  IPointer` and `glVertexAttribDivisor` are what ES 3.1 defines them as
  (they set binding `index`), in ogles2.library as well. A binding with
  stride 0 is read as tightly packed (the host's rule for `glVertexAttrib-
  Pointer`), not as every vertex reading one element. The binding queries
  (`GL_VERTEX_ATTRIB_BINDING`, `_RELATIVE_OFFSET`, `glGetIntegeri_v`'s
  `GL_VERTEX_BINDING_*`, `GL_MAX_VERTEX_ATTRIB_BINDINGS` 16,
  `GL_MAX_VERTEX_ATTRIB_STRIDE` 2048) are answered on the 68k.
- **Separable programs and pipelines:** pipelines are real host objects
  (OpenGL 4.1, `QT_OGLES2_K_ProgramPipeline`); `glProgramParameteri`
  passes `GL_PROGRAM_SEPARABLE` on; `glCreateShaderProgramv` is made of
  the existing calls as ES 3.1 defines it (the shader's compiler messages
  are not added to the program's log). A GLSL ES 3.x vertex shader now
  redeclares `gl_PerVertex` when the host compiles it (`coreSource`):
  desktop OpenGL wants that in a separable program, NVIDIA's compiler
  refuses one without it ("ARB_separate_shader_objects requires built-in
  block gl_PerVertex to be redeclared"), and it does no harm in others;
  GLSL ES 3.00 gets it through `GL_ARB_separate_shader_objects`, only on a
  4.3 host.
- A new context has no program in use, as OpenGL ES starts
  (`context.c`): the host's core contexts came with the Warp3D emulation's
  program current (`host/ffp.cpp`), which `GL_CURRENT_PROGRAM` answered
  and which shadowed a bound pipeline (programs come before pipelines).
- **`glProgramUniform*`:** `QT_OGLES2_PROGRAM_UNIFORM_V`, `QT_OGLES2_
  UNIFORM_V`'s subops with the program; the host makes the program
  current for the call and puts the previous one back (`glUseProgram(0)`
  leaves a bound pipeline in charge again). The scalar forms send their
  values as an array of one.
- **The program interface queries** (`glGetProgramInterfaceiv`,
  `glGetProgramResourceIndex`, `...Name`, `...iv`, `...Location`):
  `QT_OGLES2_PROGRAM_RESOURCE`, straight to OpenGL 4.3's, the props and
  results swapped.
- **Multisample textures:** `glTexStorage2DMultisample`, `glSampleMaski`,
  `glGetMultisamplefv`; `sampler2DMS` and `texelFetch` compile as they are.
  `glGetTexLevelParameter{i,f}v`, `glFramebufferParameteri`,
  `glGetFramebufferParameteriv`.
- No new protocol: the opcodes and subops are new ones, so a library of
  stage 3b with an older protocol 12 DLL only misses what is new.
- The table (`oglesgen.py` finds the entries by their definitions, so no
  macros make them): 340 entries written, 34 stubs.

Verification (the test WinUAE, this time on the RTX 5070 PC, NVIDIA
621.14): `tests/e04_ogles31_rest.c`, 48 checks: a pipeline of two
`glCreateShaderProgramv` programs drawing with `glProgramUniform4f`'s
colour, `glActiveShaderProgram` and `glUniform4f` into it, the pipeline
queries and validation; `glProgramUniform*` (vector, uint, matrix) on a
program that is not current, read back with `glGetUniform*`; the
interface queries of uniforms, inputs, outputs and a storage block;
interleaved position and colour through one binding at an offset, a
per-instance binding drawn with two instances, a binding switched to
another buffer, two vertex array objects keeping theirs, the binding
queries; a 4-sample texture cleared, drawn into with every sample masked
and unmasked, read with `texelFetch`, its level parameters and a sample
position; `glGetTexLevelParameterfv` and `glFramebufferParameteri`. All
pass, as do `e01`-`e03` and `o02`, with no OpenGL error
(`QUARKTEX_CHECK_ERRORS`). The four ogles3 demos and ogles2's SDK
examples (`boing2026`, `boingocean`, `cube`) draw as before. The
Warp3D/agl reference suite was not run (OneDrive was not running on that
PC, and its test files were in the cloud only); stage 3b changes nothing
of theirs but the host's list of OpenGL 4.3 functions.

## The last entries (done, 2026-11-14)

The 34 stubs left after stage 3b, written (`ogles3.library/es3more.c`,
and in `glfuncs.c` the ones ogles2.library has too, whose 6 stubs go with
them): both tables are now without stubs.

- **Compressed textures** (`QT_OGLES2_COMPRESSED_TEX`, 2D and 3D, image
  and sub-image): the blocks are byte streams and go to the host's
  `glCompressedTex*` unchanged. Offered: OpenGL ES 3.0's ten ETC2/EAC
  formats (which OpenGL 4.3 has, `GL_ARB_ES3_compatibility`) and ETC1
  (`GL_OES_compressed_ETC1_RGB8_texture`, in both libraries' extension
  lists), taken as ETC2 RGB8, of which it is a subset.
  `GL_NUM_COMPRESSED_TEXTURE_FORMATS` is 11 with a 4.3 host, 0 with a 3.3
  one. The data pointer is read as an Amiga address, as the other texture
  uploads read theirs.
- **Program and shader binaries:** no formats. `glGetProgramBinary(OES)`
  gives a length of 0 and `GL_INVALID_OPERATION`, `glProgramBinary(OES)`
  and `glShaderBinary` `GL_INVALID_ENUM`, `GL_PROGRAM_BINARY_LENGTH` is 0:
  programs that cache binaries fall back to their sources. (Errors the
  68k side raises are kept per context and come before the host's in
  `glGetError`.)
- **GL_KHR_debug** (not in `GL_EXTENSIONS`): the callback gets the
  application's own messages, `glDebugMessageInsertKHR`'s and the debug
  group pushes and pops, with `GL_DEBUG_OUTPUT` on; the host driver's are
  not passed on (a host thread cannot call 68k code), so the message log
  is empty. Group depth and overflow/underflow are kept; object labels are
  not (they read back empty). The `GL_*DEBUG*` limits are answered.
- **glFramebufferTexture3DOES:** `glFramebufferTextureLayer` of the 3D
  texture.
- **glMultiDraw{Arrays,Elements,ElementsBaseVertex}EXT:** one draw each.
  The base vertex draws (`EXT`/`OES`: plain, range, instanced) are
  `QT_OGLES2_DRAW` with its base vertex; `GL_EXT_draw_elements_base_vertex`
  and `GL_EXT_multi_draw_arrays` are in ogles3's extension list.
- **glClipControlEXT:** the host's `glClipControl` where the driver has
  OpenGL 4.5's (loaded as optional, `QT_GL_OPTIONAL_FUNCTIONS`),
  `GL_INVALID_OPERATION` where not; not offered as an extension.
- **glPrimitiveBoundingBox{OES,EXT}:** a tessellation hint; nothing to do.

Verification (test WinUAE, RTX 5070): `tests/e05_ogles3_last.c`, 36
checks: ETC2 RGB8 and ETC1 2D textures, a sub-image block, RGBA8_ETC2_EAC
in a 2D array texture through `glCompressedTexImage3D` and a sub-image,
the format list; the binary queries and errors; a debug callback with an
inserted message, a group push/pop, the depth and an underflow; a layer
of a 3D texture as a framebuffer attachment; the multi and base vertex
draws. All pass, as do `e01`-`e04`, `o01`, `o02` (its compressed format
count now 11) with no OpenGL error, the host unit test, and the
Warp3D/agl reference suite (10 PASS, 19 KNOWN).
