# uaenative.library probe

Checks that a host DLL can be reached through the UAE Native Interface
(`uaenative.library`, `native_code=true`) in both 32- and 64-bit WinUAE. That is
the route for phase 3. The old uaelib DLL calls (trap 100-103) that QuarkTex 0.53
uses are compiled only into 32-bit WinUAE (`X86_MSVC_ASSEMBLY`).

The probe checks four things:
- WinUAE finds `qtprobe-windows-x86.dll` / `qtprobe-windows-x86-64.dll` in its
  own directory
- a function receives the 68k registers in a `struct uni`
- `uni_resolve()` maps Amiga addresses (a string, and 1 MB of fast RAM) to host
  pointers
- the DLL finds the WinUAE display window (class `AmigaPowah`) itself

## Build and run

```sh
mkdir -p build/spikes
docker run --rm -v "$PWD:/w" -w /w quarktex-host sh -c '
  i686-w64-mingw32-g++   -std=c++11 -O2 -shared -static -s -o build/spikes/qtprobe-windows-x86.dll    spikes/uaenative/qtprobe.cpp
  x86_64-w64-mingw32-g++ -std=c++11 -O2 -shared -static -s -o build/spikes/qtprobe-windows-x86-64.dll spikes/uaenative/qtprobe.cpp'
docker run --rm -v "$PWD:/w" -w /w amigadev/crosstools:m68k-amigaos \
  m68k-amigaos-gcc -m68020-60 -O2 -o build/spikes/probe spikes/uaenative/probe.c
```

```powershell
pwsh spikes/uaenative/run.ps1     # uses tests/settings.local.psd1
```

Result on 2026-10-04 (WinUAE 6.0.3, AmigaOS 3.2, 68040, Z3 fast RAM):

```
== winuae.exe
   qt_probe returned 42 (OK)
   reply: 32-bit host, WinUAE, message 'hello from the Amiga', d1=41, AmigaPowah window found (1024x768)
   qt_sum over 1 MB fast RAM at 402a052c: 07f80000, expected 07f80000 (OK)
   PROBE OK
== winuae64.exe
   qt_probe returned 42 (OK)
   reply: 64-bit host, WinUAE, message 'hello from the Amiga', d1=41, AmigaPowah window found (1024x768)
   qt_sum over 1 MB fast RAM at 402a053c: 07f80000, expected 07f80000 (OK)
   PROBE OK
```

## Interface notes (from WinUAE's `uaenative.cpp`)

- LVOs: open_library -30 (a1 name, d0 min version), close_library -36 (a1),
  get_function -42 (a0 library, a1 name), call_function -48 (a0 function,
  d1-d7/a1-a5 passed on), call_function_async -54, call_function_by_name -60.
- Handles have bit 31 set; anything else is an error code (`0x7000000x`).
- The DLL is searched for as `<name>-windows-<x86|x86-64>.dll`, then `<name>.dll`,
  in the WinUAE data directory and in `plugins/`. Names must not contain a path.
- Optional exports: `uni_init()`, and the variables `uni_resolve`, `uni_version`
  and `uni_uae_version`, which the emulator fills in.
- Native functions are `int32_t f(struct uni *)` (cdecl) and run on the
  emulation thread unless the async variant is used.
