# Reference tests

Small programs that each draw one fixed scene: `tNN_*` through Warp3D.library,
`aNN_*` through agl.library (the StormMESA OpenGL API). They run in WinUAE twice,
once with the QuarkTex 0.53 libraries from Aminet (`orig`) and once with the
current build (`new`). The rendered frames are then compared pixel by pixel.

## Running

```sh
./build.sh            # libraries and host DLLs
./build.sh tests      # test programs
```

```powershell
copy tests\settings.example.psd1 tests\settings.local.psd1   # once; set your paths
pwsh tests/run.ps1
```

The report is written to `build/tests/report/index.html`. Close WinUAE first.
Each run boots WinUAE twice, which takes about a minute.

`pwsh tests/run.ps1 -Variants new -Emulator winuae64.exe` runs only the current
build, in 64-bit WinUAE, and compares it with the frames of the last 0.53 run.
The 0.53 libraries always run in `winuae.exe`, because they need the uaelib
traps of the 32-bit build. They talk to the legacy `QuarkTex.alib`, which
`./build.sh tests` builds from the end of phase 2 (0.53 plus frame capture).

`./build.sh unittest` checks the generated OpenGL encoder/decoder pair on the
build host; it needs no emulator.

Requirements:
- a 32-bit `winuae.exe` with `Amiga Programs\UAEquit`
- an A1200 Kickstart ROM
- a bootable AmigaOS 3.x hard file with RTG (uaegfx). It is mounted read-only.

## How it works

- `run.ps1` copies `winuae.exe` (and `winuae64.exe` if asked for) to
  `build/tests/winuae` and runs it in portable mode (with a `winuae.ini`), so the
  normal installation is not touched. The host libraries
  `quarktex-windows-x86[-64].dll` go next to it, the legacy `QuarkTex.alib` into
  its `alib` directory.
- The test configuration (`winuae/test.uae.in`) boots from `QTBOOT:`
  (`amiga/boot`). That volume assigns the system from the read-only hard file,
  puts `QTTEST:Libs` in front of `LIBS:`, and replaces `User-Startup` with
  `QTTEST:S/run-tests`. This script runs every test and quits WinUAE with
  `UAEquit`.
- Before creating its context, a test writes its name to
  `QTTEST:capture/label.txt`. When `QUARKTEX_CAPTURE_DIR` is set, the host
  library reads that name and saves every frame it presents as
  `<test>_<frame>.bmp`.
- `compare.py` compares the last frame of each test. A pixel counts as
  different when a channel differs by more than 8. A test fails when more than
  0.1 % of its pixels differ.
- Tests listed in `known-differences.txt` are reported as KNOWN instead of FAIL.

## Adding a test

Create `tests/tNN_name.c` with `test_name`, `test_setup`, `test_draw` and
`test_cleanup` (see `t01_triangle.c`). If it needs a Warp3D function that
`include/warp3d_calls.h` does not list yet, add it there; the LVO is in
`build/amiga/Warp3D/functable.h`.

An agl test is `tests/aNN_name.c` with the same four parts (see
`a01_agl_basic.c`). Every agl.library function can be called by name:
`aglcalls.py` generates `build/tests/amiga/agl_calls.h` from
`agl.library/agl_lib.fd`. It passes floats and doubles in fp0-fp7 as StormMESA
does, which the `inline/macros.h` LP macros cannot.

## Known bugs (in 0.53 and the current build; phase 5 fixes them)

Shown by the tests:
- Warp3D: without the z-buffer no depth reaches OpenGL, so fog does nothing
  (t06_fog, first row).
- Warp3D: `W3D_Point.pointsize` is ignored (t02_primitives).
- Warp3D: CHUNKY textures ignore the palette (t03_textures, first quad).
- agl: `glDrawArrays` with stride 0 repeats the first vertex, so the strip in
  a05_agl_queries is not drawn.

Found in the code (kept as they are so that the frames still match 0.53):
- Warp3D: `W3D_SetState(W3D_ZBUFFERUPDATE)` also switches blending (a
  missing `break`), and depth writes cannot be turned off.
- Warp3D: `SetTexEnv` and `SetWrapMode` pass their colours as r, b, g, a.
- Warp3D: `UpdateTexSubImage` uploads `texsource` instead of its image.
- Warp3D: `FreeAllTexObj` frees the wrong list nodes.
- Warp3D: the depth buffer reads and writes use the wrong sizes and addresses.
- agl: the byte swapping of 16/32-bit texture, pixel and display list data
  steps through `void *` byte by byte, so those formats arrive scrambled.
