# Phase 4: Warp3D on the host

Status: implemented, 2026-10-05. Warp3D drawing, state, textures and vertex
arrays run on the host, and the reference tests still match 0.53 in 32-bit and
64-bit WinUAE. With the JIT compiler on, Warp3D takes about a third of the time
it took in 0.53 (see [Results](#results)).

## Decision

Phase 3 showed that the traps were not the cost. The cost was the emulated 68k
code that turned every Warp3D call into OpenGL calls: `drawVertex` and up to
eleven `_gl*` encoders per textured, shaded triangle. Phase 4 moves that
translation to the host, so that a Warp3D call becomes one command:

- Warp3D.library keeps the API, the `W3D_Context` and `W3D_Texture` structures
  that applications read, the window and screen handling (`SetFunction`
  patches, Picasso96 mode requests) and `W3D_GetState`.
- Drawing, state, textures and vertex arrays are Warp3D commands in the phase 3
  buffer. `host/w3d.cpp` executes them with the OpenGL calls the 68k code made.
- The OpenGL output stays the same, including the known bugs, so the reference
  tests still match 0.53. Fixing the bugs and a modern renderer are later
  phases.
- agl.library is unchanged. It implements the OpenGL API, so the phase 3
  command buffer is already the right level for it.

## Commands

Warp3D commands (`gl/w3dcmd.h`) use the phase 3 buffer and header format with
opcodes from 0x8000. `qt_decode` hands them to `qt_w3d_decode` in
`host/w3d.cpp`. As in phase 3, commands that only pass values are queued.
Commands that read or write application memory, or return a value, flush the
buffer and run at once.

| Group | Warp3D functions | Command | Kind |
| --- | --- | --- | --- |
| Drawing | DrawTriangle, DrawTriFan, DrawTriStrip, DrawLine, DrawPoint, DrawLineStrip, DrawLineLoop, the `V` variants | `DRAW`: primitive, `context->state`, texture, then the vertices as raw 64-byte `W3D_Vertex` copies. More than 256 vertices: `DRAW_BEGIN`, `VERTICES`…, `DRAW_END` | queued |
| Vertex arrays | DrawArray, DrawElements | `DRAW_ARRAY`: the context's array pointers, strides and modes, index type and pointer | synchronous |
| State | SetState, SetBlendMode, SetAlphaMode, SetFogParams, SetZCompareMode, SetLogicOp, SetColorMask, SetCurrentColor, SetScissor | the values; the alpha reference and fog structure are copied | queued |
| Clearing | ClearDrawRegion, ClearZBuffer | colour, fullscreen flag, window size | queued |
| Textures | AllocTexObj | `TEX_ALLOC`: format, size, image; returns the OpenGL name | synchronous |
| | UpdateTexImage, UpdateTexSubImage | `TEX_UPDATE`: name, format, rectangle, image | synchronous |
| | FreeTexObj, FreeAllTexObj, SetFilter, SetTexEnv, SetWrapMode | name and values | queued |
| Context | CreateContext | `INIT_CONTEXT`: texturing on, smooth shading | queued |

Notes on the table:
- **Textures:** the host returns the OpenGL name from `TEX_ALLOC`, and
  Warp3D.library keeps it in its `Texture` structure as before. The design had
  planned a map on the host instead; the name avoids it. `DRAW` carries the name
  and the texture size, which `drawVertex` divides by.
- **Vertices:** they are copied whole and big-endian, with `movem.l`
  (`qt_copy_vertices`, 6 instructions per vertex). The host picks the fields it
  needs. A triangle is one `DRAW` command of 224 bytes.
- **`context->state`:** it travels with every draw command, because the 68k code
  read it on every vertex. Applications that change the field directly keep
  working.

### Kept on the OpenGL command path

These functions still call OpenGL from the 68k side through the phase 3 buffer:
- `W3D_ReadZPixel`/`ReadZSpan` and `W3D_WriteZPixel`/`WriteZSpan`;
- `glFinish` from `W3D_Flush`, `W3D_FlushFrame`, `W3D_WaitIdle` and
  `W3D_CheckIdle`;
- `glFrontFace`.

They are rare and synchronous, and they already work in 64-bit WinUAE. The depth
buffer functions are broken in 0.53: they read 4-byte floats into 8-byte
`W3D_Double`s, and `WriteZPixel` passes the address of its pointer. Moving them
would only rewrite those bugs; phase 5 fixes them where they are.

## 0.53 behaviour kept on the host

- `W3D_SetState(W3D_ZBUFFERUPDATE)` also switches blending: Context.c has no
  `break` after that case.
- The fog colour's alpha is 0.
- `SetTexEnv` and `SetWrapMode` pass their colours as r, b, g, a.
- `UpdateTexSubImage` uploads `texsource`, not its image argument.
- `UNPACK_SWAP_BYTES` stays as the last texture allocation left it.
- Without the z-buffer no depth reaches OpenGL (`glVertex2f`), so fog does
  nothing.

Removed: the `malloc` that `SetTexEnv` leaked on every call.

## Results

The test machine was WinUAE 6.0.3, AmigaOS 3.2 and a 68040 at `cpu_speed=max`.
`t09_throughput` was built with 400000 triangles per frame, 1.2 million in all.
Warp3D's share is the run time of t09 minus the run time of the same loop
without `W3D_DrawTriangle` (`-DNO_DRAW`). The host time comes from
`QUARKTEX_PROFILE=1`.

| Build | Emulator | JIT off | JIT on | Host time, JIT on |
| --- | --- | --- | --- | --- |
| 0.53 | winuae.exe | 3.23 s | 1.34 s | – |
| Phase 3 (OpenGL command buffer) | winuae.exe | about 2.8 s | – | – |
| Phase 4 | winuae.exe | 1.92 s (−40 %) | 0.39 s (3.4× faster) | 0.31 s |
| Phase 4 | winuae64.exe | 1.83 s (−44 %) | 0.44 s (3.1× faster) | 0.34 s |

- **Without the JIT,** emulating the 68k code still dominates: the library
  call, reserving the buffer and copying the vertices.
- **With the JIT,** 80 % of what is left is on the host. Every triangle is drawn
  with `glBegin`/`glEnd` and one OpenGL call per vertex attribute.
  Batching the vertices into arrays, or the OpenGL 3.3 renderer, is the next
  speed-up.

Warp3D.library shrank from 87 KB (phase 3) to 35 KB.

## Tests

- The reference tests match 0.53 in both emulators. The only differences are
  the known ones: t06_fog and a05_agl_queries.
- `./build.sh unittest` covers the Warp3D commands: context setup, `DRAW`, the
  `ZBUFFERUPDATE` fall-through, window clearing, texture allocation, the r, b, g,
  a colour order, and `DRAW_ARRAY` with indices.
- `tests/run.ps1 -Jit` runs the tests with the JIT compiler.

## Next

Phase 5 fixes the known bugs on the fixed-function renderer first, one at a
time, so each fix shows up as its own reference test difference. The move to
OpenGL 3.3 and shaders comes after that (decided 2026-10-04).
