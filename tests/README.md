# Reference tests

Small Warp3D programs that each draw one fixed scene. They run in WinUAE twice,
once with the QuarkTex 0.53 libraries from Aminet (`orig`) and once with the
current build (`new`). The rendered frames are then compared pixel by pixel.

## Running

```sh
./build.sh            # libraries and QuarkTex.alib
./build.sh tests      # test programs
```

```powershell
copy tests\settings.example.psd1 tests\settings.local.psd1   # once; set your paths
pwsh tests/run.ps1
```

The report is written to `build/tests/report/index.html`. Close WinUAE first.
Each run boots WinUAE twice, which takes about a minute.

Requirements:
- a 32-bit `winuae.exe` with `Amiga Programs\UAEquit`
- an A1200 Kickstart ROM
- a bootable AmigaOS 3.x hard file with RTG (uaegfx). It is mounted read-only.

## How it works

- `run.ps1` copies `winuae.exe` to `build/tests/winuae` and runs it in portable
  mode (with a `winuae.ini`), so the normal installation is not touched. The
  capture-enabled `QuarkTex.alib` goes into its `alib` directory.
- The test configuration (`winuae/test.uae.in`) boots from `QTBOOT:`
  (`amiga/boot`). That volume assigns the system from the read-only hard file,
  puts `QTTEST:Libs` in front of `LIBS:`, and replaces `User-Startup` with
  `QTTEST:S/run-tests`. This script runs every test and quits WinUAE with
  `UAEquit`.
- Before creating its context, a test writes its name to
  `QTTEST:capture/label.txt`. When `QUARKTEX_CAPTURE_DIR` is set,
  `QuarkTex.alib` reads that name and saves every frame it presents as
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
