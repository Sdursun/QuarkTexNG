# Several programs at once

## The problem

An Amiga shared library is loaded once, and every program that opens it
shares its data. Until now each QuarkTex library kept these in that shared
data:

- one command buffer and one "selected host context" (`gl/gl.c`);
- some API state each library tracks on the 68k side.

Two programs drawing at the same time therefore broke each other:

- A program can lose the CPU between `qt_reserve` and `qt_flush`. The other
  program's commands then landed in the same buffer.
- Commands went to whichever context was selected last.

Running the ogles2 SDK's `triangle`, `cube` and `boingocean` together showed
each program's triangles drawn in another program's window, and Boing Ocean
went black.

## The design

**Per task in `gl/gl.c`.** Each task gets a slot of its own, holding:

- its command buffer, allocated on its first command;
- its selected host context;
- two pointers for the library's own use (`qt_taskData`).

`qt_reserve`, `qt_flush`, `selectContext` and the rest work on the calling
task's slot. A task touches only its own slot, so no lock is needed except
`Forbid` while the slot table changes. The host executes a whole buffer
within one 68k instruction, so one host call is never interrupted by
another.

The slot lookup is a single pointer comparison when the same task calls
again. When a task frees its last context, its slot (buffer and data) goes
too. If all 32 slots are taken, the slots of tasks that have ended are taken
back (`qt_alive`). A new task's selected context starts as the one any task
selected last. This keeps working a program that draws from another task
than the one that created its context.

The host side was already per context: every `qt_execute` makes its own
context current. The protocol version is now 11, because ogles2's buffer
commands changed (below).

**Per context or per task in the libraries:**

| Library | State | Now kept |
|---|---|---|
| Warp3D | already in `W3D_Context` | context list and the ClipBlit patch counters change under `Forbid` |
| agl | vertex array pointers and formats (`VertexArrays.c`) | per task, `qt_taskData()[1]`; enable state is the host context's |
| minigl | vertex arrays, client texture unit, palette, unpack alignment | per context, `QtMglContext.client` |
| minigl | GLUT state, including `glutInit`'s clock | per task, `qt_taskData()[1]` |
| minigl | the current context | per program: see below |
| ogles2 | attributes, buffer bindings, current attribute values, mapped buffers | per context, `struct OGLES2Context` |
| ogles2 | the current context | per task, `qt_taskData()[0]` |

**MiniGL's current context.** MiniGL 29's applications read and write their
current context themselves, through the variable the dispatch table's
`currentContext` points to (`MGLD_CTX` in `<libraries/minigl_dispatch.h>`).
With one table, every program shared that variable. `MGLGetDispatchTable`
now gives each calling task its own copy of the table, pointing to a
variable of its own (16 copies; ended tasks' copies are reused; when all 16
are taken, the shared table is the fallback).

**ogles2's buffers on the host** are no longer bound there. The 68k side
keeps the bindings per context and names the buffer in `QT_OGLES2_BUFFER_
DATA`, `BUFFER_SUB_DATA`, `READ_BUFFER` and `GET_PARAM`. `QT_OGLES2_BIND_
BUFFER` is gone. The application's GLSL ES sources, kept for
`glGetShaderSource`, are keyed by host context as well as shader name.

## Verification

In WinUAE, with the new libraries loaded from RAM: by path (`preload`, a
scratch tool), seven programs were started at once:

- `o02_ogles2_api`, twice;
- `a06_agl_pixels`;
- `t14_zbuffer_io` and `t15_stencil` (Warp3D);
- `m01_minigl`;
- `a03_agl_texture`.

Every value they read back matched a run on its own:

- o02's 37 checks passed in both copies;
- m01's checks passed;
- glReadPixels, the Z buffer and stencil reads were identical.

`triangle`, `cube` and `boingocean` running together each draw their own
picture. The reference suite (`tests/run.ps1`) is unchanged.

## Limits

- Windows that overlap: with presentation into Amiga display memory
  (phase 8), each program's frame is written over its whole window
  rectangle, also over windows in front of it. This is unchanged and not
  part of this work.
- `MGLSetWindowMode`/pixel depth hints (minigl `context.c`) are still
  shared. They are set just before a context is made, so a clash needs two
  programs starting in the same instant.
