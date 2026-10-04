# Phase 5: bug fixes

Status: in progress, started 2026-10-05.

The known bugs of QuarkTex 0.53 are fixed one at a time on the fixed-function
OpenGL renderer. Moving to OpenGL 3.3 comes after this phase.

## How each fix is checked

1. A reference test shows the bug. Where none did, a new test comes first and
   records the 0.53 behaviour.
2. The frames of the build before the fix are kept as a snapshot:
   `pwsh tests/run.ps1 -Save <name>`.
3. After the fix, `pwsh tests/run.ps1 -Variants new -Against <name>` must show
   exactly the expected test as changed, and all others unchanged.
4. The new frame is checked by eye. The test then goes into
   `tests/known-differences.txt` as a difference from 0.53.

## Fixes

| # | Bug | Test | Status |
| --- | --- | --- | --- |
| 1 | `W3D_SetState(W3D_ZBUFFERUPDATE)` switched blending (missing `break`); depth writes could not be turned off | t10_zupdate (new) | fixed |
| 2 | Without the z-buffer no depth reaches OpenGL, so fog does nothing. Now the depth is sent when the z-buffer or fog is on | t06_fog, first row | fixed |
| 3 | `SetTexEnv` and `SetWrapMode` passed their colours as r, b, g, a | t11_texcolors (new) | fixed |
| 4 | `UpdateTexSubImage` uploads `texsource` instead of its image; `FreeAllTexObj` frees the wrong list nodes | new test | open |
| 5 | `W3D_Point.pointsize` and line widths are ignored | t02_primitives | open |
| 6 | CHUNKY textures ignore the palette | t03_textures, first quad | open |
| 7 | Depth buffer reads and writes use the wrong sizes and addresses | new test | open |
| 8 | agl: 16/32-bit texture, pixel and display list data is byte-swapped one byte at a time (`void *` steps) | new test | open |
| 9 | agl: `glDrawArrays` with stride 0 repeats the first vertex | a05_agl_queries | open |

## Test infrastructure changes

- `run.ps1 -Save`/`-Against` (snapshots) and `compare.py --no-known`.
- The tests write their capture label, read it back, and retry. Once a test's
  label write failed (the run before fix 1), and its frames went out under the
  previous test's name. The project folder is in OneDrive, which may lock the
  file for a moment; that is a guess, not confirmed. Retries are logged as
  "label written on attempt N".
