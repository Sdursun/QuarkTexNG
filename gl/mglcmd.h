/*
 * Commands of QuarkTex's minigl.library for the host (host/mgl.cpp), beside
 * the OpenGL commands and the Warp3D ones (gl/w3dcmd.h). Same format: the
 * header word is (opcode << 16) | word count, arguments are 32-bit
 * big-endian words, addresses Amiga addresses, constants OpenGL's.
 */
#ifndef QUARKTEX_MGLCMD_H
#define QUARKTEX_MGLCMD_H

#define QT_MGL_FIRST 0x9000

/*
 * glDrawArrays/glDrawElements with the vertex arrays, read by the host from
 * Amiga memory (big-endian), synchronous: mode, first, count, index type
 * (0 for glDrawArrays, else GL_UNSIGNED_BYTE/SHORT/INT), index address; then
 * for the vertex, colour and texture coordinate arrays each: enabled, size,
 * type, stride (in bytes, not 0), address.
 */
#define QT_MGL_DRAW 0x9000
#define QT_MGL_DRAW_WORDS 21
#define QT_MGL_MAX_VERTICES 1048576

#endif
