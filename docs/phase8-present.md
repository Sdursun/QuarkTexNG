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

## Cost

Reading the picture back and writing it costs RTCW at 640 x 480, uncapped,
about 2.7 ms a frame: 188 fps before, about 118 now. Capped at its usual 60
fps there is no difference to see.
