/*
 * Warp3D commands in the command buffer (docs/phase4-warp3d-on-host.md).
 * Shared by Warp3D.library (68k) and the host library (host/w3d.cpp).
 *
 * The header word is (opcode << 16) | word count, as for the OpenGL commands
 * in gl/glgen.cpp; Warp3D opcodes start at QT_W3D_FIRST. Arguments are 32-bit
 * big-endian words, floats as their bit patterns, addresses are Amiga
 * addresses. Enumerations are Warp3D values; the host maps them to OpenGL.
 */
#ifndef QUARKTEX_W3DCMD_H
#define QUARKTEX_W3DCMD_H

#define QT_W3D_FIRST 0x8000

/* Initial OpenGL state of a new context. No arguments. */
#define QT_W3D_INIT_CONTEXT 0x8000

/*
 * Drawing: DRAW_BEGIN, one or more VERTICES, DRAW_END.
 * DRAW_BEGIN: OpenGL primitive, context->state, texture used (0/1), its
 *   OpenGL name, width and height.
 * VERTICES: count, then count W3D_Vertex structures copied as they are
 *   (16 words each, QT_W3D_VERTEX_WORDS).
 */
#define QT_W3D_DRAW_BEGIN 0x8001
#define QT_W3D_VERTICES 0x8002
#define QT_W3D_DRAW_END 0x8003
#define QT_W3D_VERTEX_WORDS 16
#define QT_W3D_MAX_VERTICES 256 /* per VERTICES or DRAW command */

/*
 * The same in one command for up to QT_W3D_MAX_VERTICES vertices: the six
 * DRAW_BEGIN arguments, the count, the vertices.
 */
#define QT_W3D_DRAW 0x800F

/* W3D_SetState: state bit, W3D_ENABLE or W3D_DISABLE. */
#define QT_W3D_SET_STATE 0x8004
/* W3D_SetBlendMode: source and destination factor. */
#define QT_W3D_BLEND_MODE 0x8005
/* W3D_SetAlphaMode: mode, reference value (float). */
#define QT_W3D_ALPHA_MODE 0x8006
/* W3D_SetFogParams: mode, start, end, density, colour r, g, b (floats). */
#define QT_W3D_FOG 0x8007
/* W3D_SetZCompareMode: mode. */
#define QT_W3D_Z_COMPARE 0x8008
/* W3D_SetLogicOp: operation. */
#define QT_W3D_LOGIC_OP 0x8009
/* W3D_SetColorMask: red, green, blue, alpha (booleans). */
#define QT_W3D_COLOR_MASK 0x800A
/* W3D_SetCurrentColor: r, g, b, a (floats). */
#define QT_W3D_CURRENT_COLOR 0x800B
/* W3D_SetScissor: x, y (OpenGL, from the bottom), width, height. */
#define QT_W3D_SCISSOR 0x800C
/* W3D_ClearDrawRegion: colour (ARGB), fullscreen (0/1), width, height. */
#define QT_W3D_CLEAR 0x800D
/* W3D_ClearZBuffer. No arguments. */
#define QT_W3D_CLEAR_Z 0x800E

#endif
