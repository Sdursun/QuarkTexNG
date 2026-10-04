/*
 * Warp3D commands in the command buffer (docs/phase4-warp3d-on-host.md).
 * Shared by Warp3D.library (68k) and the host library (host/w3d.cpp).
 *
 * The header word is (opcode << 16) | word count, as for the OpenGL commands
 * in gl/glgen.cpp; Warp3D opcodes start at QT_W3D_FIRST. Arguments are 32-bit
 * big-endian words, addresses are Amiga addresses.
 */
#ifndef QUARKTEX_W3DCMD_H
#define QUARKTEX_W3DCMD_H

#define QT_W3D_FIRST 0x8000

/* Initial OpenGL state of a new context. No arguments. */
#define QT_W3D_INIT_CONTEXT 0x8000

#endif
