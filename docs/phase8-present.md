# Phase 8: frames into Amiga display memory

## The problem

Until now every context drew into a host window of its own, a child of the
emulator's window, placed over the Amiga window or screen. That only works
while WinUAE shows the Amiga display one to one in a window:

- In fullscreen WinUAE presents with Direct3D 11 in exclusive mode, and the
  child window is not shown at all: the game ran (frames were drawn, the log
  was clean) but the screen showed the empty Amiga window.
- In full-window mode, or whenever WinUAE scales the display (window size,
  filters, aspect correction), the child window kept the unscaled Amiga
  coordinates: the picture sat small at the top left.

`tests/run-app.ps1 -Fullscreen true|fullwindow` runs WinUAE in those modes;
screenshots of the real screen during the run showed both cases with RTCW.

## The design

A context created with `QT_CONTEXT_OFFSCREEN` (gl/gl.h) draws into a
framebuffer object (colour RGBA8, depth 24 and stencil 8) instead of a window;
its host window stays hidden. At each swap the 68k side names a `QtTarget`:
a bitmap in Amiga memory (address, bytes per row, Picasso96 RGBFormat, size)
and the rectangle in it. The host reads the picture back (`glReadPixels`,
BGRA), turns it upright, converts it into the bitmap's format and writes it
there (`host/present.h`, `host/present.cpp`). The emulator then shows it like
any other Amiga graphics: in a window, full window or fullscreen, scaled and
filtered with the rest of the display; screenshots and recordings of WinUAE
contain it, and Amiga windows and menus over it are no longer hidden.

The formats written are Picasso96's RGBFB_R8G8B8 to RGBFB_B5G5R5PC: 24- and
32-bit in every byte order, 16- and 15-bit big- and little-endian. Planar and
8-bit (CLUT) displays keep the host window, which shows in windowed WinUAE
only, as before.

An offscreen context's framebuffer object has no front or back buffer, so
`glDrawBuffer` and `glReadBuffer` with `GL_FRONT`, `GL_BACK` and the like
name its colour buffer (`host/gldecode.cpp`); without that, RTCW's
`glDrawBuffer(GL_BACK)` every frame was an OpenGL error. The frame capture
reads the framebuffer object as well.

`qt_swap_buffers` takes the target in a1 (0: none); `QT_PROTOCOL_VERSION`
is 9, so libraries and host DLLs of different versions refuse each other
cleanly (phase 7's minigl commands did not raise it either).

## minigl.library

On a Picasso96 bitmap of one of those formats every context is offscreen
(`minigl.library/context.c`): the window's inner area in the screen's
bitmap, the whole screen in fullscreen. `MGLSwitchDisplay` locks the bitmap
(`p96LockBitMap`, which gives its address, bytes per row and format) and the
window's layer, and has the host write the frame. A window that moves or
changes size gives the picture its new size. `mglCreateContextFromBitMap`,
which returned NULL, works now for such a bitmap: frames go into it at
`MGLSwitchDisplay`.

## Cost, and reading back without waiting

Measured on RTCW at 640 x 480, uncapped (`-Profile` logs the parts): the
first, synchronous version cost 2.7 ms a frame, 188 fps before phase 8 and
118 after. Of those 2.7 ms, 1.3 went into waiting for the GPU to finish the
frame (a `glFinish` before `glReadPixels` took them), 1.0 into the transfer
and 0.7 into converting and writing.

- **Asynchronous read back.** At the swap `glReadPixels` goes into one of
  two pixel buffers with a fence after it, and the swap returns. The picture
  is written as soon as the fence has passed: checked without waiting at
  each of the next frame's command buffers (the GPU is done within a
  millisecond or two, while the 68k prepares the next frame), at the next
  swap at the latest. An application that waits for input after a frame
  would never send those buffers, so `finishFrame` (host
  `qt_finish_frame`) writes the pending picture at once: minigl's main loop
  calls it before it waits, and `glFinish` does as well.
- **One conversion loop per format**, 32-bit formats as one load, shift or
  byte swap and store: 0.7 ms became 0.25.

Now the swap takes 0.08 ms and writing 0.25: RTCW runs at 170 fps, bound by
the 68k.

## Checks

- `tests/host` checks the conversion into all twelve formats, clipping at
  the bitmap's edge and the upright rows, and that `glDrawBuffer`/
  `glReadBuffer` look at the framebuffer binding.
- `m01_minigl` reads back, after `glFinish`, what the Amiga display shows
  (`p96ReadPixel` on its window): the same points as in the picture, in a
  window and in fullscreen.
- Screenshots of the real screen during RTCW, JK2, OpenLara and Hurrican
  in exclusive fullscreen show the games.
- A minigl.library of protocol 8 with this host gets no context (and the
  other way round): the libraries and DLLs go together.

## Screen modes

RTCW started without arguments (`r_mode 6`, `r_fullscreen 1`: a 1024 x 768
screen) showed only the top left of its menu. minigl.library picked the
display mode with graphics.library's `BestModeID`, which gave the 1024 x 768
screen a 640 x 480 RTG mode, with the size in the Nominal tags (they only
give the aspect ratio) and with the Desired tags as well, although a
1024 x 768 16-bit mode was there. It now asks Picasso96
(`p96BestModeIDTags`) and keeps `BestModeID` for systems without it: RTCW
gets 1024 x 768, JK2 640 x 480 (it had a 1024 x 768 mode for its 640 x 480
screen), m01 320 x 240. The host log names the screen and its mode
(`minigl.library: screen 1024x768 in mode 0x50051100 (1024x768)`).

`tests/run-app.ps1 -UaeOptions` adds WinUAE configuration lines, for
example `gfx_width_fullscreen=800` as in a user's configuration.

## The emulated FPU's precision

JK2's first person weapon never showed and RTCW's vanished during play,
while both show on a real Amiga. The traced frame (`-TraceFrame`) showed the
weapon's parts drawn with a model view matrix of NaNs and infinities, and a
HUD element with NaN coordinates, computed by the game without any GL query.
WinUAE emulates the 68k FPU with 64-bit doubles by default; the games count
on the 80 bits of a real 68881/68040/68060. With CPU and FPU, FPU: "Host
(80-bit)" (`fpu_msvc_long_double=true`) the weapon shows and no matrix of the
traced frame has a NaN.

`gl/gl.c` checks at library initialisation whether 1 + 2^-60 differs from 1
(only with the 64-bit mantissa of extended precision; in WinUAE 0 by default,
1 with "Host (80-bit)", with the JIT FPU as well, and with Softfloat). If
not, all libraries log a warning, and minigl.library shows a requester
before its first screen opens. The test configuration sets
`fpu_msvc_long_double=true`; the reference tests are unchanged by it.

## agl.library and Warp3D.library

The presenting moved to `gl/gl.c` (`presentable`, `presentInto`), shared by
the three libraries. agl and Warp3D write each frame before the call
returns (`wait`): their applications may stop drawing after any frame
(event-driven StormMESA programs), and no later command would write it.

- **agl.library:** a context on a Picasso96 window is offscreen;
  `AmigaMesaSwapBuffers` writes the frame into the window.
- **Warp3D.library, windowed:** the frame goes into the window when the
  application ClipBlits into it (as before, ClipBlit is patched) or calls
  `W3D_FlushFrame`.
- **Warp3D.library, fullscreen** (`W3D_CC_MODEID`): the frame goes into the
  draw region's bitmap at its y offset when the application shows it.
  `ChangeScreenBuffer` and `ScrollVPort` are patched to write the frame
  first and then call the original function (through a trampoline that
  puts the library base in a6); changing the draw region and
  `W3D_FlushFrame` write it too. A dirty flag, set by the drawing and
  clearing commands, keeps a frame from being written twice. The 0.53
  patches that turned `RectFill`, `EraseRect` and the display functions
  off stay for the host window path of planar screens only.

Found on the way:

- Since phase 6 the fullscreen branch of `W3D_CreateContext` tested `qt`
  instead of the fullscreen flag, so fullscreen contexts got no host
  context and the call failed.
- `W3D_AllocTexObj` left `mipmaps[0-15]` as malloc left them and wrote
  `mipmaps[16]`, past the array. The texture is cleared now.
- The `W3D_Context` fields that describe the draw region (drawregion,
  width, height, bprow, depth, format, yoffset, scissor, maximum texture
  sizes) stayed 0; they are filled from the bitmap now, also on
  `W3D_SetDrawRegion`.
- Lines go through pixel centres, as points do. At whole coordinates the
  row a line lit was the driver's choice, and the window and the
  framebuffer object of the same driver chose differently;
  `t02_primitives` is a known difference to 0.53 for it.

Checks: every reference test prints what the Amiga display shows at five
points of its window after the last frame (`report_shown` in
`tests/window.c`); all 27 match their captured frame. `w01_w3d_fullscreen`
opens a double-buffered 16-bit screen, draws three frames into alternate
buffers and flips them, and reads both buffers back: the context describes
the screen and each buffer holds its frame.

Open: MiniGL Classic (the Warp3D-based minigl.library) in fullscreen
creates its context and a texture, destroys the context and then crashes
the system (AN_MemCorrupt). QuartexNG's own minigl.library runs those
games; MiniGL Classic on QuartexNG's Warp3D is still to be looked into.
