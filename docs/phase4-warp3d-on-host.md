# Phase 4: Warp3D on the host

Status: proposal, 2026-10-04

## Decision

Phase 3 showed that the traps were not the cost. The cost is the emulated 68k
code that turns every Warp3D call into OpenGL calls: `drawVertex` and up to
eleven `_gl*` encoders per textured, shaded triangle. Phase 4 moves that
translation to the host, so that a Warp3D call becomes one command:

- Warp3D.library keeps the API, the `W3D_Context` and `W3D_Texture` structures
  that applications read, the window and screen handling (`SetFunction`
  patches, Picasso96 mode requests) and `W3D_GetState`.
- Everything that calls OpenGL today becomes one Warp3D command in the phase 3
  buffer. The host executes it with the same OpenGL calls the 68k code makes
  now.
- The OpenGL output stays the same, including the known bugs, so the reference
  tests must keep matching 0.53. Fixing the bugs and a modern renderer
  (OpenGL 3.3, shaders) are the next phase, where frames are allowed to change.
- agl.library stays as it is. It implements the OpenGL API, so the phase 3
  command buffer is already the right level for it.

## Commands

Warp3D commands use the phase 3 buffer and header format with opcodes from
0x8000 up; the host's `qt_decode` hands them to a Warp3D dispatcher
(`host/w3d.cpp`). The rules are the same as in phase 3: commands that only
pass values are queued, and commands that read or write application memory
later, or return a value, flush the buffer and run at once.

| Group | Warp3D functions | Command contents | Kind |
| --- | --- | --- | --- |
| Drawing | DrawTriangle, DrawTriFan, DrawTriStrip, DrawLine, DrawPoint, DrawLineStrip, DrawLineLoop, the `V` variants | `context->state`, texture handle, the vertices as raw 64-byte `W3D_Vertex` copies | queued |
| Vertex arrays | DrawArray, DrawElements | state, texture handle, array addresses, strides and modes from the context, index address | synchronous (reads application arrays) |
| State | SetState, SetBlendMode, SetAlphaMode, SetFogParams, SetZCompareMode, SetLogicOp, SetColorMask, SetScissor, SetCurrentColor, SetFilter, SetWrapMode, SetTexEnv | the values (the alpha reference and fog structure are copied) | queued |
| Clearing | ClearDrawRegion, ClearZBuffer | colour, window size, fullscreen flag | queued |
| Textures | AllocTexObj, UpdateTexImage, UpdateTexSubImage | handle, format, size, image and palette addresses | synchronous (the host reads the image) |
| | FreeTexObj | handle | queued |
| Depth buffer | ReadZPixel, ReadZSpan, WriteZPixel, WriteZSpan | coordinates, buffer address | synchronous |
| Sync | Flush, WaitIdle, CheckIdle | none | synchronous |

Notes on the table:
- **Texture handles** are the Amiga addresses of the `W3D_Texture` structures;
  the host maps them to OpenGL texture names. It also keeps the texture width
  and height, which `drawVertex` divides by.
- **Vertices** are copied whole with `CopyMem`, big-endian, and the host picks
  the fields it needs. This is the cheapest thing the 68k can do. A triangle
  takes 204 bytes (header, state, texture handle and 3 × 64), so the 256 KB
  buffer holds about 1280 of them.
- **`context->state`** travels with every draw command, as the 68k code reads it
  on every vertex today. Applications that change the field directly keep
  working.

## What the 68k side keeps

- `W3D_CreateContext`/`W3D_DestroyContext`: the structure, the window and
  screen patches, `createContext` on the host, and one command that sets the
  initial OpenGL state (texturing on, smooth shading).
- Texture objects: the `W3D_Texture` structure and its fields stay filled in as
  now. Only the OpenGL part moves.
- Everything without OpenGL: driver queries, `W3D_GetState`, screen mode
  requests, stencil stubs.
- The library ABI: the same entry points, registers and structure layouts.

## Tests

- The reference tests (Warp3D and agl) must match 0.53 as now, in 32-bit and
  64-bit WinUAE.
- `t09_throughput` measures the gain. Expected: several times the phase 3 rate,
  because the 68k work per triangle drops from about eleven encoder calls with
  float conversions to one `CopyMem` of 192 bytes.
- The unit test gets Warp3D cases: each command is encoded on the host with
  known values, decoded against recording OpenGL stubs, and checked against the
  calls the old 68k code would make.

## Steps

1. Protocol and host skeleton: opcode range, the Warp3D dispatcher, texture
   handle map, and the context's initial state command.
2. Drawing and state commands: DrawTriangle, DrawTriFan, DrawTriStrip, lines,
   points and the `V` variants, SetState and the other state setters, and
   ClearDrawRegion. After this step t01, t02, t04, t05, t06, t07 and t09 run on
   the new path.
3. Textures: AllocTexObj, Update(Sub)TexImage, FreeTexObj, SetFilter,
   SetWrapMode, SetTexEnv (t03).
4. Vertex arrays and the depth buffer reads/writes (t08).
5. Measurements, unit tests, documentation; remove the `_gl*` calls from
   Warp3D.library.

## Open questions

- Which applications read or write `W3D_Context` fields directly? The design
  keeps every field filled in as now, so nothing should change for them.

## Next

Phase 5 fixes the known bugs on the fixed-function renderer first, one at a
time, so each fix shows up as its own reference test difference. The move to
OpenGL 3.3 and shaders comes after that (decided 2026-10-04).
