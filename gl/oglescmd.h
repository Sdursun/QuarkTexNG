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

/*
 * Textures and framebuffers (stage 2, cube/boingocean): real host OpenGL
 * objects, the same as shaders and programs (above). glTexImage2D's pixel
 * data is converted as glTexSubImage2D's is (QT_OGLES2_TEX_SUB_IMAGE_2D).
 */

/* glGenTextures/glDeleteTextures, glGenFramebuffers/glDeleteFramebuffers,
 * glGenRenderbuffers/glDeleteRenderbuffers: kind (QT_OGLES2_K_*), n, address
 * of n big-endian GLuint names (real host ones here, unlike QT_OGLES2_GEN_
 * BUFFERS's fake ones -- textures and framebuffers do not need converting
 * at draw time the way buffer data does). */
#define QT_OGLES2_GEN 0xA015
#define QT_OGLES2_GEN_WORDS 4
#define QT_OGLES2_DELETE 0xA016
#define QT_OGLES2_DELETE_WORDS 4
enum { QT_OGLES2_K_Texture, QT_OGLES2_K_Framebuffer, QT_OGLES2_K_Renderbuffer };

/* glBindTexture, glActiveTexture, glGenerateMipmap, glTexParameteri,
 * glBindFramebuffer, glBindRenderbuffer, glFramebufferTexture2D,
 * glFramebufferRenderbuffer, glCheckFramebufferStatus (result: its return
 * value): subop, w0..w4 (an enum/int/uint as given; unused trailing words
 * ignored). */
#define QT_OGLES2_CALL 0xA017
#define QT_OGLES2_CALL_WORDS 7
enum {
	QT_OGLES2_C_BindTexture, QT_OGLES2_C_ActiveTexture, QT_OGLES2_C_GenerateMipmap, QT_OGLES2_C_TexParameteri,
	QT_OGLES2_C_BindFramebuffer, QT_OGLES2_C_BindRenderbuffer, QT_OGLES2_C_FramebufferTexture2D,
	QT_OGLES2_C_FramebufferRenderbuffer, QT_OGLES2_C_CheckFramebufferStatus, QT_OGLES2_C_RenderbufferStorage
};

/* glTexImage2D: target, level, internalformat, width, height, border,
 * format, type, address of the pixel data (0: NULL, as for a render
 * target's own storage). */
#define QT_OGLES2_TEX_IMAGE_2D 0xA018
#define QT_OGLES2_TEX_IMAGE_2D_WORDS 10

/* glUniformMatrix4fv: location, count, transpose (ignored: always sent
 * column-major, as OpenGL's own uniforms are, so a transposed matrix is
 * swapped on the 68k side instead, gl/gl.c's LoadMatrixf/MultMatrixf
 * already do for the fixed-function matrix stack), address of 16 * count
 * big-endian floats. */
#define QT_OGLES2_UNIFORM_MATRIX4FV 0xA019
#define QT_OGLES2_UNIFORM_MATRIX4FV_WORDS 5

/*
 * The rest of GLES2 (stage 3). Entries GL 1.1 already has with no pointer
 * argument (glBlendFunc, glStencilOp, glPixelStorei, glCopyTexImage2D, ...)
 * use the existing _gl<Name> encoders, as stage 1's do; vertex attribute
 * state queries and glMapBufferOES are answered on the 68k side.
 *
 * QT_OGLES2_CALL's later subops: glBlendColor and glSampleCoverage take
 * float words (bit patterns, read with c.f()); glVertexAttrib4f is index,
 * x, y, z, w (glVertexAttrib1f..4fv all become it, GLES2's own defaults of
 * 0, 0, 0, 1 filled in on the 68k side); glIsBuffer asks about QuarkTex's
 * fake buffers (QT_OGLES2_GEN_BUFFERS).
 *
 * QT_OGLES2_DRAW's "first" is the base vertex of an indexed draw (glDraw-
 * ElementsBaseVertexOES; 0 from glDrawElements), added to every index.
 */
enum {
	QT_OGLES2_C_BlendColor = QT_OGLES2_C_RenderbufferStorage + 1, QT_OGLES2_C_BlendEquation,
	QT_OGLES2_C_BlendEquationSeparate, QT_OGLES2_C_BlendFuncSeparate, QT_OGLES2_C_StencilFuncSeparate,
	QT_OGLES2_C_StencilMaskSeparate, QT_OGLES2_C_StencilOpSeparate, QT_OGLES2_C_SampleCoverage,
	QT_OGLES2_C_IsFramebuffer, QT_OGLES2_C_IsRenderbuffer, QT_OGLES2_C_IsBuffer, QT_OGLES2_C_VertexAttrib4f,
	QT_OGLES2_C_ProvokingVertex
};

/* glUniform{1,2,3,4}{f,i}v (QT_OGLES2_UNIFORM's subops) and glUniformMatrix
 * {2,3,4}fv (below; transpose is GL_FALSE in GLES2, so not sent): subop,
 * location, count, address of count vectors or matrices of big-endian
 * floats or ints. */
#define QT_OGLES2_UNIFORM_V 0xA01A
#define QT_OGLES2_UNIFORM_V_WORDS 5
enum { QT_OGLES2_U_M2 = QT_OGLES2_U_4I + 1, QT_OGLES2_U_M3, QT_OGLES2_U_M4 };

/* glBufferSubData: target (its bound fake buffer), offset, size, address of
 * size bytes, kept big-endian as glBufferData's are. QT_OGLES2_READ_BUFFER
 * is the reverse, for glMapBufferOES: the bound buffer's bytes offset to
 * offset + size copied to the address. */
#define QT_OGLES2_BUFFER_SUB_DATA 0xA01B
#define QT_OGLES2_BUFFER_SUB_DATA_WORDS 5
#define QT_OGLES2_READ_BUFFER 0xA01C
#define QT_OGLES2_READ_BUFFER_WORDS 5

/* glGetIntegerv/glGetFloatv/glGetBooleanv: kind, pname, address the host
 * writes the value(s) to, as many as pname has (big-endian ints or floats,
 * GLboolean bytes). GLES2-only pnames the host's desktop core profile does
 * not have are answered from their desktop equivalents. */
#define QT_OGLES2_GET 0xA01D
#define QT_OGLES2_GET_WORDS 4
enum { QT_OGLES2_GET_Integer, QT_OGLES2_GET_Float, QT_OGLES2_GET_Boolean };

/* glGetTexParameter{i,f}v, glGetRenderbufferParameteriv, glGetFramebuffer-
 * AttachmentParameteriv, glGetBufferParameteriv (a fake buffer's), glGet-
 * Uniform{f,i}v: subop, w0..w2 (their scalar arguments in order: target,
 * pname; target, attachment, pname; program, location), address of the
 * result(s), big-endian. A uniform's value count comes from its type. */
#define QT_OGLES2_GET_PARAM 0xA01E
#define QT_OGLES2_GET_PARAM_WORDS 6
enum {
	QT_OGLES2_P_TexParameteriv, QT_OGLES2_P_TexParameterfv, QT_OGLES2_P_RenderbufferParameteriv,
	QT_OGLES2_P_FramebufferAttachmentParameteriv, QT_OGLES2_P_BufferParameteriv, QT_OGLES2_P_Uniformfv,
	QT_OGLES2_P_Uniformiv
};

/* glGetActiveAttrib/glGetActiveUniform: subop, program, index, bufSize,
 * addresses of the length, size and type (big-endian, each 0 if not
 * wanted) and of bufSize bytes for the name. */
#define QT_OGLES2_GET_ACTIVE 0xA01F
#define QT_OGLES2_GET_ACTIVE_WORDS 9
enum { QT_OGLES2_A_Attrib, QT_OGLES2_A_Uniform };

/* glGetAttachedShaders: program, maxCount, address of the count (0 if not
 * wanted), address of maxCount GLuints. */
#define QT_OGLES2_GET_ATTACHED_SHADERS 0xA020
#define QT_OGLES2_GET_ATTACHED_SHADERS_WORDS 5

/* glGetShaderSource: QT_OGLES2_GET_LOG with this subop. */
enum { QT_OGLES2_G_ShaderSource = QT_OGLES2_G_Program + 1 };

/* glTexSubImage2D: target, level, xoffset, yoffset, width, height, format,
 * type, address of the pixel data. GL_UNSIGNED_BYTE pixels are forwarded
 * as given; the 16-bit types (GL_UNSIGNED_SHORT_5_6_5, 4_4_4_4, 5_5_5_1,
 * GL_UNSIGNED_SHORT) and 32-bit ones (GL_UNSIGNED_INT, _24_8) are swapped.
 * GL_ALPHA/LUMINANCE/LUMINANCE_ALPHA are
 * (gone from desktop core profiles) stored as GL_RED/GL_RG with a swizzle. */
#define QT_OGLES2_TEX_SUB_IMAGE_2D 0xA021
#define QT_OGLES2_TEX_SUB_IMAGE_2D_WORDS 10

/* glReadPixels: x, y, width, height, format, type, address the host reads
 * into (16-bit packed types swapped afterwards). */
#define QT_OGLES2_READ_PIXELS 0xA022
#define QT_OGLES2_READ_PIXELS_WORDS 8

/* glDrawBuffersEXT: n, address of n big-endian GLenums. */
#define QT_OGLES2_DRAW_BUFFERS 0xA023
#define QT_OGLES2_DRAW_BUFFERS_WORDS 3

#endif
