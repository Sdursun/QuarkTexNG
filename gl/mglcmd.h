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
 * for the vertex, colour, and texture units 0 and 1 coordinate arrays each:
 * enabled, size, type, stride (in bytes, not 0), address.
 */
#define QT_MGL_DRAW 0x9000
#define QT_MGL_DRAW_WORDS 26
#define QT_MGL_MAX_VERTICES 1048576

/* GL_ARB_multitexture, which OpenGL 1.1's command set lacks; units are
 * GL_TEXTURE0 and up, QT_MGL_TEXTURE_UNITS of them. glActiveTexture: unit.
 * glMultiTexCoord2f: unit, s, t. */
#define QT_MGL_ACTIVE_TEXTURE 0x9001
#define QT_MGL_ACTIVE_TEXTURE_WORDS 2
#define QT_MGL_MULTI_TEX_COORD 0x9002
#define QT_MGL_MULTI_TEX_COORD_WORDS 4
#define QT_MGL_TEXTURE_UNITS 2

#endif
