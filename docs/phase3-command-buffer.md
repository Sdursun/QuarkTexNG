# Phase 3: command buffer over uaenative.library

Status: proposal, 2026-10-04

## Decision

Phase 3 changes only the transport between the 68k libraries and the host. The
new transport works in 32-bit and 64-bit WinUAE. What gets rendered stays the
same, so every reference test must keep producing the same frames as now.

- The host is reached through `uaenative.library` instead of the uaelib traps
  100-105. The old traps are compiled only into 32-bit WinUAE. The
  `spikes/uaenative` probe shows the new route works in both builds.
- The 68k side can no longer call `opengl32.dll` directly: in a 64-bit process
  the function pointers do not fit in a 68k register. All OpenGL calls move into
  the host DLL.
- Calls that take only scalar arguments and return nothing are written to a
  command buffer. The buffer is sent to the host in one call when it is flushed.
- All other calls run at once, after the buffer has been flushed. These are
  calls that take pointers or return a value. Their behaviour is exactly what it
  is today.
- Warp3D keeps translating to OpenGL on the 68k side. Moving that logic to the
  host is phase 4.

## Why pointer calls stay synchronous

agl.library converts caller memory in place around each call:

```c
void glVertex3fv(GLfloat *v __asm("a0")) {
	SWAP32(v, 3);
	_glVertex3fv(memoffset + (long) v);
	SWAP32(v, 3);
}
```

If such a call were queued, the host would read the data after it had been
swapped back. Running these calls at once keeps today's semantics. The cost is
one host call each.

Of the 336 functions in `gl/glFuncs.txt`:
- 177 take only scalars and return nothing; they are queued.
- 153 take pointers and 8 return values; they run at once.

The Warp3D drawing path is all scalars: `glBegin`, `glTexCoord4f`, `glColor4f`,
`glVertex3f` and `glEnd`. It is therefore queued completely. A textured, shaded
triangle costs 11 traps today; with the buffer it costs none until the next
flush.

Queueing the fixed-size `*v` variants by copying their data into the buffer is
left for later. Today that would conflict with the in-place swapping above.

## Components

```
Warp3D.library / agl.library (68k, unchanged except memoffset = 0)
        │  _glXxx(...)               generated from gl/glFuncs.txt
        ▼
gl/gl.c (68k)       queue: append to buffer       sync: flush, then call
        │  uaenative.library call_function (a0 = function handle)
        ▼
quarktex-windows-x86[-64].dll (host)
        qt_execute(buffer, length)   decode and call OpenGL, generated
        qt_call_<name>(...)          one entry per synchronous function, generated
        qt_create/move/free/swap/log context management, frame capture
```

### 68k side (`gl/gl.c`)

- `glInit` opens `uaenative.library`, opens the host library `quarktex` and looks
  up the function handles. If that fails, `W3D_CreateContext` returns an error
  instead of crashing. Today a missing DLL ends in a call to address 0.
- A buffer of 256 KB is allocated with `AllocVec(MEMF_ANY)`.
- A queued call appends `opcode (16 bit) | word count (16 bit)` followed by its
  arguments as 32-bit big-endian words. A double takes two words, high word
  first.
- The buffer is flushed when it is full, before every synchronous call, and on
  swap, `glFlush`, `glFinish` and `freeContext`.
- `memoffset` becomes 0. The existing `memoffset + (long) ptr` expressions then
  pass plain Amiga addresses, and the host resolves them with `uni_resolve()`.
  No call sites need to change.

### Host side

- Built from one source as `quarktex-windows-x86.dll` and
  `quarktex-windows-x86-64.dll`. Installed into the WinUAE directory or
  `plugins/`; `QuarkTex.alib` and the `alib` directory go away.
- `qt_execute` reads the buffer, swaps every word to host order, and dispatches
  on the opcode to the OpenGL call. The dispatch table is generated from
  `glFuncs.txt` by the same generator as the 68k side, so the two cannot drift
  apart.
- Synchronous functions receive their arguments in `struct uni`: d1-d7, a1-a5,
  plus a second argument block in the buffer for calls with more than 12 words.
  This also fixes `glMap2d`, which is silently dropped today.
- The OpenGL window is created as a child of the WinUAE display window. The DLL
  finds that window itself (class `AmigaPowah`), because `uaenative.library` does
  not pass it.
- Frame capture (`QUARKTEX_CAPTURE_DIR`) moves over unchanged.
- OpenGL stays the fixed-function 1.1 API in this phase.

## Protocol versioning

The host exports `qt_protocol_version`. The 68k side checks it in `glInit` and
refuses to start on a mismatch. Opcodes are the line numbers in `glFuncs.txt`,
so new functions are appended to the end of that file.

## Tests

- The reference tests run unchanged in `winuae.exe` and `winuae64.exe`:
  `run.ps1 -Emulator winuae64.exe`. Both must match the 0.53 frames as now
  (t06_fog stays a known difference).
- New test `t09_throughput` draws a fixed number of triangles for a fixed time
  and logs triangles per second. It is not compared, only reported. Run it on
  the phase 2 build and the phase 3 build to measure the gain.
- A unit test for the generated encoder/decoder pair runs on the host in Docker.
  It encodes every opcode with known arguments, decodes them against a stub
  OpenGL table, and checks every argument.

## Steps

1. Host DLL skeleton on uaenative: context management, window lookup, capture.
   The 68k bridge uses it for the five host functions; OpenGL still goes the old
   way, so this step can only be tested in 32-bit WinUAE.
2. Generator: opcodes, 68k encoder, host decoder, synchronous entry points, and
   the host-side unit test.
3. Switch `gl/gl.c` to the buffer and set `memoffset` to 0. From here 64-bit
   WinUAE works.
4. `t09_throughput`, reference tests in both emulators, and documentation
   (install instructions change: one DLL per architecture, no `alib`).
5. Remove the uaelib trap code.

## Open questions

- Does any application rely on GL errors appearing at the exact call? Queued
  calls report errors at the next flush. QuarkTex only logs errors at swap, so
  this should not matter.
- Buffer size: a textured, shaded Warp3D triangle takes 180 bytes, so 256 KB
  holds about 1450 triangles. That is one or two flushes per frame for a Quake
  scene. Measure in step 4 and adjust if needed.
