# Phase 5: bug fixes

Status: done (2026-10-05). All ten known bugs are fixed.

The known bugs of QuarkTex 0.53 are fixed one at a time on the fixed-function
OpenGL renderer. Moving to OpenGL 3.3 followed in phase 6
(`docs/phase6-core-renderer.md`).

## How each fix is checked

1. A reference test shows the bug. Where none did, a new test comes first and
   records the 0.53 behaviour.
2. The frames of the build before the fix are kept as a snapshot:
   `pwsh tests/run.ps1 -Save <name>`.
3. After the fix, `pwsh tests/run.ps1 -Variants new -Against <name>` must show
   exactly the expected test as changed, and all others unchanged. This
   comparison is strict: a single changed pixel counts (`compare.py --strict`).
   The 0.53 comparison allows 0.1 % of the pixels to differ, which hid the
   point sizes of fix 5.
4. The new frame is checked by eye. The test then goes into
   `tests/known-differences.txt` as a difference from 0.53.

## Fixes

| # | Bug | Test | Status |
| --- | --- | --- | --- |
| 1 | `W3D_SetState(W3D_ZBUFFERUPDATE)` switched blending (missing `break`); depth writes could not be turned off | t10_zupdate (new) | fixed |
| 2 | Without the z-buffer no depth reaches OpenGL, so fog does nothing. Now the depth is sent when the z-buffer or fog is on | t06_fog, first row | fixed |
| 3 | `SetTexEnv` and `SetWrapMode` passed their colours as r, b, g, a | t11_texcolors (new) | fixed |
| 4 | `UpdateTexSubImage` uploaded `texsource` instead of its image and ignored `srcbpr`; `FreeAllTexObj` freed the wrong list nodes (and NULL) | t12_texupdate (new) | fixed |
| 5 | `W3D_Point.pointsize` and line widths were ignored | t02_primitives (0.065 % of its pixels, below the 0.1 % threshold of the 0.53 comparison) | fixed |
| 6 | CHUNKY textures ignored the palette (`W3D_ATO_PALETTE`, and the palette argument of the updates); the host now converts them to RGBA | t03_textures, first quad | fixed |
| 7 | Depth buffer reads and writes used the wrong sizes, addresses, y direction and type; they also have to convert between Warp3D z and the (z + 1) / 2 in the depth buffer, and mask colour writes | t14_zbuffer_io (new) | fixed |
| 8 | agl: 16/32-bit texture, pixel and display list data was byte-swapped in place one byte at a time (`void *` steps), one element per pixel and without row padding. Pixel data now goes to the host unchanged and the host's `PACK`/`UNPACK_SWAP_BYTES` (the inverse of the application's) turn it around; `SWAPn` steps by its element size | a06_agl_pixels (new) | fixed |
| 9 | agl: `glDrawArrays` with stride 0 repeated the first vertex; the pointer functions now use the packed element size | a05_agl_queries | fixed |
| 10 | `UpdateTexImage`/`UpdateTexSubImage` left `UNPACK_SWAP_BYTES` as the last allocation set it, so 16-bit textures were updated with the wrong byte order after an 8-bit one was allocated | t13_texswap (new) | fixed |

## Test infrastructure changes

- `run.ps1 -Save`/`-Against` (snapshots) and `compare.py --no-known`
  and `--strict`. All fixes were checked again strictly between the saved
  snapshots; each changed only its own test.
- The tests write their capture label, read it back, and retry. Once a test's
  label write failed (the run before fix 1), and its frames went out under the
  previous test's name. The project folder is in OneDrive, which may lock the
  file for a moment; that is a guess, not confirmed. Retries are logged as
  "label written on attempt N".
