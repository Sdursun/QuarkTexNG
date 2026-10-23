/*
 * Commands of QuarkTex's ogles2.library for the host (host/ogles2.cpp),
 * beside the OpenGL, Warp3D and minigl ones. Same format: the header word is
 * (opcode << 16) | word count, arguments are 32-bit big-endian words,
 * addresses are Amiga addresses, GL constants are OpenGL's own (GLES2's
 * tokens already are, docs/phase9-ogles2.md).
 *
 * Shaders, programs and their scalar entries (CreateShader, CompileShader,
 * AttachShader, LinkProgram, UseProgram, ...) go through the host's real
 * OpenGL 3.3 core objects directly -- their IDs are plain GLuint results,
 * no byte order concern, so they are hand-written 68k <-> host wrappers
 * with no new opcode (ogles2.library/glfuncs.c calls the host's existing
 * _gl<Name> encoders, generated from gl/glFuncs.txt, the same ones Warp3D
 * uses). Only entries below need their own opcode: ones whose pointed-to
 * data is multi-byte (float/int/short) and so needs byte order handled, or
 * whose Amiga-side pointer only makes sense resolved against GLES2's own
 * state (a bound buffer).
 *
 * Buffers (stage 2): GL_ARRAY_BUFFER/GL_ELEMENT_ARRAY_BUFFER data is kept as
 * a raw (still big-endian) copy on the host, one per name QT_OGLES2_GEN_
 * BUFFERS made up (host/ogles2.cpp's own small-integer names, not real
 * GL buffer objects): the element type is not known until glVertexAttrib-
 * Pointer or glDrawElements names it, so the byte order conversion happens
 * then, the same way minigl.library's vertex arrays are converted at draw
 * time (host/mgl.cpp) rather than when the application hands them over.
 */
#ifndef QUARKTEX_OGLESCMD_H
#define QUARKTEX_OGLESCMD_H

#define QT_OGLES2_FIRST 0xA000

/* glShaderSource: shader, address of one NUL-terminated GLSL ES string.
 * count > 1 or an explicit length (shaders given as several chunks, or not
 * NUL-terminated) are not supported yet: every known caller, including the
 * ogles2 SDK's own examples, passes one whole, NUL-terminated string. */
#define QT_OGLES2_SHADER_SOURCE 0xA000
#define QT_OGLES2_SHADER_SOURCE_WORDS 3

/* glGenBuffers/glDeleteBuffers: n, address of n big-endian GLuint names.
 * The names are QuarkTex's own (1, 2, 3, ...), not host GL buffer names. */
#define QT_OGLES2_GEN_BUFFERS 0xA001
#define QT_OGLES2_GEN_BUFFERS_WORDS 3
#define QT_OGLES2_DELETE_BUFFERS 0xA002
#define QT_OGLES2_DELETE_BUFFERS_WORDS 3

/* glBindBuffer: target, buffer (0 unbinds, as GLES2 has it). */
#define QT_OGLES2_BIND_BUFFER 0xA003
#define QT_OGLES2_BIND_BUFFER_WORDS 3

/* glBufferData: the buffer bound to target, size, address of size bytes
 * (big-endian, kept as given -- converted at draw time), usage (ignored,
 * nothing here streams). */
#define QT_OGLES2_BUFFER_DATA 0xA004
#define QT_OGLES2_BUFFER_DATA_WORDS 5

/* glVertexAttribPointer, glEnable/DisableVertexAttribArray: 68k-side only
 * (minigl.library's own vertex arrays work the same way, host/mgl.cpp): the
 * pointer is kept, by index, on the 68k side and only reaches the host as
 * part of a QT_OGLES2_DRAW, where the buffer or Amiga memory it names is
 * read and converted. No opcode of their own.
 *
 * glDrawArrays/glDrawElements with QT_OGLES2_MAX_ATTRIBS vertex attributes
 * resolved and converted from the (fake) bound buffers or Amiga memory:
 * mode, first, count, index type (0 for glDrawArrays, else GL_UNSIGNED_
 * BYTE/SHORT), index buffer name (0: indices is an Amiga address; GLES2
 * reads element indices from a bound GL_ELEMENT_ARRAY_BUFFER, so this
 * follows glVertexAttribPointer's own buffer-or-address rule), indices;
 * then per attribute: enabled, size, type, normalized, stride, buffer name
 * (0: pointer is an Amiga address), pointer/offset. */
#define QT_OGLES2_MAX_ATTRIBS 8
#define QT_OGLES2_DRAW 0xA006
#define QT_OGLES2_DRAW_WORDS (7 + 7 * QT_OGLES2_MAX_ATTRIBS)

/*
 * Shaders, programs and uniforms: real OpenGL 2.0 core entries (gl3.h, the
 * same dynamically loaded set ffp.cpp uses), all scalar or GLchar* (plain
 * bytes, no order to convert) -- so every one of them fits a few generic
 * shapes, by subop (host/ogles2.cpp), rather than one opcode each.
 */

/* word0, word1: scalar arguments, raw 32-bit words (an enum/int/uint as
 * given, a float as its bit pattern read with c.f() where the subop takes
 * one); result: the call's return value (0 for void ones). */
#define QT_OGLES2_SIMPLE 0xA010
#define QT_OGLES2_SIMPLE_WORDS 4
enum {
	QT_OGLES2_S_CreateShader, QT_OGLES2_S_DeleteShader, QT_OGLES2_S_CompileShader,
	QT_OGLES2_S_AttachShader, QT_OGLES2_S_DetachShader, QT_OGLES2_S_CreateProgram,
	QT_OGLES2_S_DeleteProgram, QT_OGLES2_S_LinkProgram, QT_OGLES2_S_UseProgram,
	QT_OGLES2_S_ValidateProgram, QT_OGLES2_S_IsShader, QT_OGLES2_S_IsProgram
};

/* program/index, name (an Amiga address, read as a NUL-terminated string):
 * BindAttribLocation (index used), GetAttribLocation, GetUniformLocation
 * (index ignored, result their return value). */
#define QT_OGLES2_NAME 0xA011
#define QT_OGLES2_NAME_WORDS 5
enum { QT_OGLES2_N_BindAttribLocation, QT_OGLES2_N_GetAttribLocation, QT_OGLES2_N_GetUniformLocation };

/* location, v0..v3 (int or float bit patterns, by subop): glUniform1f..4f,
 * 1i..4i. */
#define QT_OGLES2_UNIFORM 0xA012
#define QT_OGLES2_UNIFORM_WORDS 7
enum {
	QT_OGLES2_U_1F, QT_OGLES2_U_2F, QT_OGLES2_U_3F, QT_OGLES2_U_4F,
	QT_OGLES2_U_1I, QT_OGLES2_U_2I, QT_OGLES2_U_3I, QT_OGLES2_U_4I
};

/* glGetShaderiv/glGetProgramiv: id, pname, address of one GLint (swapped
 * explicitly, unlike the generic command buffer's raw pointer forwarding). */
#define QT_OGLES2_GET_IV 0xA013
#define QT_OGLES2_GET_IV_WORDS 5
enum { QT_OGLES2_G_Shader, QT_OGLES2_G_Program };

/* glGetShaderInfoLog/glGetProgramInfoLog: subop (QT_OGLES2_G_Shader/
 * Program, as QT_OGLES2_GET_IV's), id, bufSize, address of one GLsizei (the
 * length actually written, swapped explicitly; 0: not wanted), address of
 * bufSize bytes for the text (plain bytes, no order to convert). */
#define QT_OGLES2_GET_LOG 0xA014
#define QT_OGLES2_GET_LOG_WORDS 6

#endif
