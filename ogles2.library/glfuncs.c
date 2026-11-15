#include "ogles2.h"
#include "gldeclarations.auto.h"
#include <exec/memory.h>
#include <proto/exec.h>

/*
 * The GL entries the host's existing OpenGL 3.3 core command buffer already
 * carries verbatim (docs/phase9-ogles2.md, stage 1): no pointers, so no
 * byte order or marshalling to handle, and GLES2's tokens are already
 * OpenGL's own (unlike MiniGL), so no enum translation either. Every entry
 * here just forwards to the existing _gl<Name> encoder, dropping the
 * context argument (one context is selected at a time, like Warp3D/agl).
 *
 * The rest of the GLES2 API -- shaders, programs, buffers, framebuffers,
 * textures, anything that takes a pointer -- is stage 2: it needs the same
 * kind of hand-written marshalling minigl.library's vertex arrays have
 * (host/mgl.cpp), since the pointed-to data is big-endian in Amiga memory
 * and the host reads it natively.
 */

void ogles2_GLClear(struct OGLES2Context *ctx, GLbitfield mask) { _glClear(mask); }
void ogles2_GLClearColor(struct OGLES2Context *ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	_glClearColor(red, green, blue, alpha);
}
void ogles2_GLClearDepthf(struct OGLES2Context *ctx, GLfloat d) { _glClearDepth(d); }
void ogles2_GLClearStencil(struct OGLES2Context *ctx, GLint s) { _glClearStencil(s); }
void ogles2_GLViewport(struct OGLES2Context *ctx, GLint x, GLint y, GLsizei width, GLsizei height) {
	_glViewport(x, y, width, height);
}
void ogles2_GLFlush(struct OGLES2Context *ctx) { _glFlush(); }
void ogles2_GLFinish(struct OGLES2Context *ctx) { _glFinish(); }
GLenum ogles2_GLGetError(struct OGLES2Context *ctx) {
	if (ctx && ctx->error) {
		GLenum e = ctx->error;
		ctx->error = 0;
		return e;
	}
	return _glGetError();
}
void ogles2_error(struct OGLES2Context *ctx, GLenum error) {
	if (ctx && !ctx->error) ctx->error = error;
}
void ogles2_GLEnable(struct OGLES2Context *ctx, GLenum cap) { _glEnable(cap); }
void ogles2_GLDisable(struct OGLES2Context *ctx, GLenum cap) { _glDisable(cap); }
GLboolean ogles2_GLIsEnabled(struct OGLES2Context *ctx, GLenum cap) { return _glIsEnabled(cap); }
void ogles2_GLColorMask(struct OGLES2Context *ctx, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	_glColorMask(red, green, blue, alpha);
}
void ogles2_GLDepthMask(struct OGLES2Context *ctx, GLboolean flag) { _glDepthMask(flag); }
void ogles2_GLDepthFunc(struct OGLES2Context *ctx, GLenum func) { _glDepthFunc(func); }
void ogles2_GLCullFace(struct OGLES2Context *ctx, GLenum mode) { _glCullFace(mode); }
void ogles2_GLFrontFace(struct OGLES2Context *ctx, GLenum mode) { _glFrontFace(mode); }
void ogles2_GLHint(struct OGLES2Context *ctx, GLenum target, GLenum mode) { _glHint(target, mode); }
void ogles2_GLLineWidth(struct OGLES2Context *ctx, GLfloat width) { _glLineWidth(width); }
void ogles2_GLScissor(struct OGLES2Context *ctx, GLint x, GLint y, GLsizei width, GLsizei height) {
	_glScissor(x, y, width, height);
}

static const char vendor[] = "QuarkTex NG";
#ifdef QT_OGLES3
static const char renderer[] = QT_PRODUCT " ogles3.library (host OpenGL)";
#else
static const char renderer[] = QT_PRODUCT " ogles2.library (host OpenGL)";
#endif
/* As the PiStorm libraries: the OpenGL ES version, then the library's
 * name and version.revision ("OpenGL ES 3.1 ogles3.library 1.0"; amiga/
 * Makefile gives QT_LIB_VERSION and QT_LIB_REVISION). */
#define QT_STR_(x) #x
#define QT_STR(x) QT_STR_(x)
#ifdef QT_OGLES3
#define QT_LIBRARY_TEXT " ogles3.library " QT_STR(QT_LIB_VERSION) "." QT_STR(QT_LIB_REVISION)
#else
#define QT_LIBRARY_TEXT " ogles2.library " QT_STR(QT_LIB_VERSION) "." QT_STR(QT_LIB_REVISION)
#endif
static const char version[] = "OpenGL ES 2.0" QT_LIBRARY_TEXT;
static const char shadingVersion[] = "OpenGL ES GLSL ES 1.00";
static const char version3[] = "OpenGL ES 3.0" QT_LIBRARY_TEXT;
static const char shadingVersion3[] = "OpenGL ES GLSL ES 3.00";
static const char version31[] = "OpenGL ES 3.1" QT_LIBRARY_TEXT;
static const char shadingVersion31[] = "OpenGL ES GLSL ES 3.10";
/* Only what the host's OpenGL 3.3 core context really does as GLES2 would
 * (host/ogles2.cpp): non-power-of-two textures, 32-bit indices, depth
 * textures, the shaders' derivative functions (core in the GLSL 3.30 the
 * host compiles them as), the renderbuffer formats it shares,
 * glMapBufferOES and the base vertex draw; anisotropic filtering and the
 * S3TC (DXT1/3/5) compressed formats, which every desktop driver has (the
 * host passes GL_TEXTURE_MAX_ANISOTROPY_EXT and the DXT formats on as they
 * are). */
static const char extensions[] = "GL_OES_texture_npot GL_OES_element_index_uint GL_OES_depth_texture "
	"GL_OES_standard_derivatives GL_OES_depth24 "
	"GL_OES_packed_depth_stencil GL_OES_rgb8_rgba8 GL_OES_mapbuffer GL_OES_draw_elements_base_vertex "
	"GL_EXT_discard_framebuffer GL_OES_compressed_ETC1_RGB8_texture GL_EXT_texture_filter_anisotropic "
	"GL_EXT_texture_compression_s3tc GL_EXT_texture_compression_dxt1";
/* OpenGL ES 3.0 contexts (ogles3.library) add what desktop OpenGL 3.3 has
 * of the ES 3.0 extensions: float and half float colour buffers. */
static const char extensions3[] = "GL_OES_texture_npot GL_OES_element_index_uint GL_OES_depth_texture "
	"GL_OES_standard_derivatives GL_OES_depth24 "
	"GL_OES_packed_depth_stencil GL_OES_rgb8_rgba8 GL_OES_mapbuffer GL_OES_draw_elements_base_vertex "
	"GL_EXT_discard_framebuffer GL_EXT_color_buffer_float GL_EXT_color_buffer_half_float "
	"GL_OES_compressed_ETC1_RGB8_texture GL_EXT_draw_elements_base_vertex GL_EXT_multi_draw_arrays "
	"GL_EXT_texture_filter_anisotropic GL_EXT_texture_compression_s3tc GL_EXT_texture_compression_dxt1";

const GLubyte *ogles2_GLGetString(struct OGLES2Context *ctx, GLenum name) {
	switch (name) {
	case GL_VENDOR: return (const GLubyte *) vendor;
	case GL_RENDERER: return (const GLubyte *) renderer;
	case GL_VERSION:
		return (const GLubyte *) (ctx && ctx->esVersion >= 31 ? version31 : ctx && ctx->esVersion >= 30 ? version3 : version);
	case GL_SHADING_LANGUAGE_VERSION:
		return (const GLubyte *) (ctx && ctx->esVersion >= 31 ? shadingVersion31
			: ctx && ctx->esVersion >= 30 ? shadingVersion3 : shadingVersion);
	case GL_EXTENSIONS: return (const GLubyte *) (ctx && ctx->esVersion >= 30 ? extensions3 : extensions);
	}
	return (const GLubyte *) "";
}

/*
 * Stage 2 (docs/phase9-ogles2.md): shaders, programs, uniforms, buffers and
 * drawing, through QuarkTex's own opcodes (gl/oglescmd.h) -- real OpenGL 2.0
 * entries on the host (host/ogles2.cpp, host/gl3.h), reached generically
 * (QT_OGLES2_SIMPLE/NAME/UNIFORM/GET_IV/GET_LOG) since they take only
 * scalars or GLchar* text (no byte order to handle). Vertex attribute
 * pointers and buffers do need it; see gl/oglescmd.h and host/ogles2.cpp.
 */

/* The word for a float argument: on the 68k the wire format (big-endian) is
 * already its native one, so this only needs to alias the bits, as gl/gl.c's
 * own qt_f2l does (static there, so not usable here). */
#define floatWord ogles2_floatWord
#define simple ogles2_simple
#define nameCall ogles2_nameCall
#define uniform ogles2_uniform
#define genObjects ogles2_genObjects
#define deleteObjects ogles2_deleteObjects
#define call ogles2_call
#define bufferCopy ogles2_bufferCopy
#define getParam ogles2_getParam
#define boundBuffer ogles2_boundBuffer

ULONG floatWord(GLfloat f) {
	union { GLfloat f; ULONG l; } u;
	u.f = f;
	return u.l;
}

LONG simple(ULONG subop, ULONG w0, ULONG w1) {
	ULONG *w = qt_reserve(QT_OGLES2_SIMPLE_WORDS);
	w[0] = ((ULONG) QT_OGLES2_SIMPLE << 16) | QT_OGLES2_SIMPLE_WORDS;
	w[1] = subop;
	w[2] = w0;
	w[3] = w1;
	return (LONG) qt_flush();
}

GLuint ogles2_GLCreateShader(struct OGLES2Context *ctx, GLenum type) { return (GLuint) simple(QT_OGLES2_S_CreateShader, type, 0); }
void ogles2_GLDeleteShader(struct OGLES2Context *ctx, GLuint shader) { simple(QT_OGLES2_S_DeleteShader, shader, 0); }
void ogles2_GLCompileShader(struct OGLES2Context *ctx, GLuint shader) { simple(QT_OGLES2_S_CompileShader, shader, 0); }
void ogles2_GLAttachShader(struct OGLES2Context *ctx, GLuint program, GLuint shader) {
	simple(QT_OGLES2_S_AttachShader, program, shader);
}
void ogles2_GLDetachShader(struct OGLES2Context *ctx, GLuint program, GLuint shader) {
	simple(QT_OGLES2_S_DetachShader, program, shader);
}
GLuint ogles2_GLCreateProgram(struct OGLES2Context *ctx) { return (GLuint) simple(QT_OGLES2_S_CreateProgram, 0, 0); }
void ogles2_GLDeleteProgram(struct OGLES2Context *ctx, GLuint program) { simple(QT_OGLES2_S_DeleteProgram, program, 0); }
void ogles2_GLLinkProgram(struct OGLES2Context *ctx, GLuint program) { simple(QT_OGLES2_S_LinkProgram, program, 0); }
void ogles2_GLUseProgram(struct OGLES2Context *ctx, GLuint program) { simple(QT_OGLES2_S_UseProgram, program, 0); }
void ogles2_GLValidateProgram(struct OGLES2Context *ctx, GLuint program) { simple(QT_OGLES2_S_ValidateProgram, program, 0); }
GLboolean ogles2_GLIsShader(struct OGLES2Context *ctx, GLuint shader) { return (GLboolean) simple(QT_OGLES2_S_IsShader, shader, 0); }
GLboolean ogles2_GLIsProgram(struct OGLES2Context *ctx, GLuint program) { return (GLboolean) simple(QT_OGLES2_S_IsProgram, program, 0); }

/* A chunk's length: given, or up to its NUL. */
static ULONG chunkLength(const GLchar *text, const GLint *length, GLsizei i) {
	ULONG n = 0;
	if (length && length[i] >= 0) return (ULONG) length[i];
	while (text[n]) ++n;
	return n;
}

/* The host takes one NUL-terminated text (QT_OGLES2_SHADER_SOURCE): several
 * chunks, or chunks with an explicit length (ioquake3's renderer), are
 * joined into one first. */
void ogles2_GLShaderSource(struct OGLES2Context *ctx, GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length) {
	ULONG *w;
	char *joined = NULL;
	const char *text;
	GLsizei i;
	if (count < 1 || !string) return;
	if (count == 1 && !length && string[0]) text = string[0];
	else {
		ULONG size = 1, at = 0;
		for (i = 0; i < count; ++i) if (string[i]) size += chunkLength(string[i], length, i);
		if (!(joined = AllocVec(size, MEMF_ANY))) return;
		for (i = 0; i < count; ++i) {
			ULONG n;
			if (!string[i]) continue;
			n = chunkLength(string[i], length, i);
			CopyMem((APTR) string[i], joined + at, n);
			at += n;
		}
		joined[at] = 0;
		text = joined;
	}
	w = qt_reserve(QT_OGLES2_SHADER_SOURCE_WORDS);
	w[0] = ((ULONG) QT_OGLES2_SHADER_SOURCE << 16) | QT_OGLES2_SHADER_SOURCE_WORDS;
	w[1] = shader;
	w[2] = (ULONG) text;
	qt_flush();
	if (joined) FreeVec(joined);
}

LONG nameCall(ULONG subop, ULONG program, ULONG index, const char *text) {
	ULONG *w = qt_reserve(QT_OGLES2_NAME_WORDS);
	w[0] = ((ULONG) QT_OGLES2_NAME << 16) | QT_OGLES2_NAME_WORDS;
	w[1] = subop;
	w[2] = program;
	w[3] = index;
	w[4] = (ULONG) text;
	return (LONG) qt_flush();
}

void ogles2_GLBindAttribLocation(struct OGLES2Context *ctx, GLuint program, GLuint index, const GLchar *name) {
	nameCall(QT_OGLES2_N_BindAttribLocation, program, index, name);
}
GLint ogles2_GLGetAttribLocation(struct OGLES2Context *ctx, GLuint program, const GLchar *name) {
	return (GLint) nameCall(QT_OGLES2_N_GetAttribLocation, program, 0, name);
}
GLint ogles2_GLGetUniformLocation(struct OGLES2Context *ctx, GLuint program, const GLchar *name) {
	return (GLint) nameCall(QT_OGLES2_N_GetUniformLocation, program, 0, name);
}

void uniform(ULONG subop, GLint location, ULONG w0, ULONG w1, ULONG w2, ULONG w3) {
	ULONG *w = qt_reserve(QT_OGLES2_UNIFORM_WORDS);
	w[0] = ((ULONG) QT_OGLES2_UNIFORM << 16) | QT_OGLES2_UNIFORM_WORDS;
	w[1] = subop;
	w[2] = (ULONG) location;
	w[3] = w0;
	w[4] = w1;
	w[5] = w2;
	w[6] = w3;
	qt_flush();
}

void ogles2_GLUniform1f(struct OGLES2Context *ctx, GLint location, GLfloat v0) {
	uniform(QT_OGLES2_U_1F, location, floatWord(v0), 0, 0, 0);
}
void ogles2_GLUniform2f(struct OGLES2Context *ctx, GLint location, GLfloat v0, GLfloat v1) {
	uniform(QT_OGLES2_U_2F, location, floatWord(v0), floatWord(v1), 0, 0);
}
void ogles2_GLUniform3f(struct OGLES2Context *ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
	uniform(QT_OGLES2_U_3F, location, floatWord(v0), floatWord(v1), floatWord(v2), 0);
}
void ogles2_GLUniform4f(struct OGLES2Context *ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
	uniform(QT_OGLES2_U_4F, location, floatWord(v0), floatWord(v1), floatWord(v2), floatWord(v3));
}
void ogles2_GLUniform1i(struct OGLES2Context *ctx, GLint location, GLint v0) {
	uniform(QT_OGLES2_U_1I, location, (ULONG) v0, 0, 0, 0);
}
void ogles2_GLUniform2i(struct OGLES2Context *ctx, GLint location, GLint v0, GLint v1) {
	uniform(QT_OGLES2_U_2I, location, (ULONG) v0, (ULONG) v1, 0, 0);
}
void ogles2_GLUniform3i(struct OGLES2Context *ctx, GLint location, GLint v0, GLint v1, GLint v2) {
	uniform(QT_OGLES2_U_3I, location, (ULONG) v0, (ULONG) v1, (ULONG) v2, 0);
}
void ogles2_GLUniform4i(struct OGLES2Context *ctx, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
	uniform(QT_OGLES2_U_4I, location, (ULONG) v0, (ULONG) v1, (ULONG) v2, (ULONG) v3);
}

/* params/length/infoLog are given straight to the host (QT_OGLES2_GET_IV,
 * QT_OGLES2_GET_LOG): it writes the (big-endian) result into them itself,
 * synchronously, unlike the generic command buffer's raw pointer forwarding
 * (gl/glgen.cpp), which would leave a host-native (little-endian) value for
 * these multi-byte results. */
void ogles2_GLGetShaderiv(struct OGLES2Context *ctx, GLuint shader, GLenum pname, GLint *params) {
	ULONG *w = qt_reserve(QT_OGLES2_GET_IV_WORDS);
	w[0] = ((ULONG) QT_OGLES2_GET_IV << 16) | QT_OGLES2_GET_IV_WORDS;
	w[1] = QT_OGLES2_G_Shader;
	w[2] = shader;
	w[3] = pname;
	w[4] = (ULONG) params;
	qt_flush();
}

void ogles2_GLGetProgramiv(struct OGLES2Context *ctx, GLuint program, GLenum pname, GLint *params) {
	ULONG *w;
	/* GL_PROGRAM_BINARY_LENGTH: no binaries are offered (glGetProgramBinary). */
	if (pname == 0x8741) {
		if (params) *params = 0;
		return;
	}
	w = qt_reserve(QT_OGLES2_GET_IV_WORDS);
	w[0] = ((ULONG) QT_OGLES2_GET_IV << 16) | QT_OGLES2_GET_IV_WORDS;
	w[1] = QT_OGLES2_G_Program;
	w[2] = program;
	w[3] = pname;
	w[4] = (ULONG) params;
	qt_flush();
}

static void getLog(ULONG subop, GLuint id, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
	ULONG *w = qt_reserve(QT_OGLES2_GET_LOG_WORDS);
	w[0] = ((ULONG) QT_OGLES2_GET_LOG << 16) | QT_OGLES2_GET_LOG_WORDS;
	w[1] = subop;
	w[2] = id;
	w[3] = (ULONG) bufSize;
	w[4] = (ULONG) length;
	w[5] = (ULONG) infoLog;
	qt_flush();
}

void ogles2_GLGetShaderInfoLog(struct OGLES2Context *ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
	getLog(QT_OGLES2_G_Shader, shader, bufSize, length, infoLog);
}
void ogles2_GLGetProgramInfoLog(struct OGLES2Context *ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
	getLog(QT_OGLES2_G_Program, program, bufSize, length, infoLog);
}

/* glGenBuffers/glDeleteBuffers: QuarkTex's own small-integer buffer names
 * (host/ogles2.cpp), not real GL buffer objects -- its vertex and index data
 * needs converting from big-endian at draw time, once a type is known
 * (glVertexAttribPointer, glDrawElements), the same as minigl.library's
 * vertex arrays (host/mgl.cpp), which real GL buffer objects could not do. */
void ogles2_GLGenBuffers(struct OGLES2Context *ctx, GLsizei n, GLuint *buffers) {
	ULONG *w = qt_reserve(QT_OGLES2_GEN_BUFFERS_WORDS);
	w[0] = ((ULONG) QT_OGLES2_GEN_BUFFERS << 16) | QT_OGLES2_GEN_BUFFERS_WORDS;
	w[1] = (ULONG) n;
	w[2] = (ULONG) buffers;
	qt_flush();
}

void ogles2_GLDeleteBuffers(struct OGLES2Context *ctx, GLsizei n, const GLuint *buffers) {
	ULONG *w = qt_reserve(QT_OGLES2_DELETE_BUFFERS_WORDS);
	w[0] = ((ULONG) QT_OGLES2_DELETE_BUFFERS << 16) | QT_OGLES2_DELETE_BUFFERS_WORDS;
	w[1] = (ULONG) n;
	w[2] = (ULONG) buffers;
	qt_flush();
}

/* GL_ARRAY_BUFFER/GL_ELEMENT_ARRAY_BUFFER bindings: per context, here only.
 * The host is told the buffer's name with each command that uses one, so
 * several programs' bindings never meet there (docs/multitasking.md), and
 * glVertexAttribPointer/glDrawElements send the buffer name that was bound
 * at that call, as GLES2 itself requires. */
/* The binding of target (OpenGL ES 3.0's targets as well), NULL for none. */
static GLuint *binding(struct OGLES2Context *ctx, GLenum target) {
	switch (target) {
	case GL_ARRAY_BUFFER: return &ctx->arrayBuffer;
	case GL_ELEMENT_ARRAY_BUFFER: return &ctx->elementArrayBuffer;
	case 0x8A11: return &ctx->uniformBuffer;            /* GL_UNIFORM_BUFFER */
	case 0x8F36: return &ctx->copyReadBuffer;           /* GL_COPY_READ_BUFFER */
	case 0x8F37: return &ctx->copyWriteBuffer;          /* GL_COPY_WRITE_BUFFER */
	case 0x88EB: return &ctx->pixelPackBuffer;          /* GL_PIXEL_PACK_BUFFER */
	case 0x88EC: return &ctx->pixelUnpackBuffer;        /* GL_PIXEL_UNPACK_BUFFER */
	case 0x8C8E: return &ctx->transformFeedbackBuffer;  /* GL_TRANSFORM_FEEDBACK_BUFFER */
	case 0x90D2: return &ctx->shaderStorageBuffer;      /* GL_SHADER_STORAGE_BUFFER */
	case 0x92C0: return &ctx->atomicCounterBuffer;      /* GL_ATOMIC_COUNTER_BUFFER */
	case 0x90EE: return &ctx->dispatchIndirectBuffer;   /* GL_DISPATCH_INDIRECT_BUFFER */
	case 0x8F3F: return &ctx->drawIndirectBuffer;       /* GL_DRAW_INDIRECT_BUFFER */
	}
	return NULL;
}

void ogles2_GLBindBuffer(struct OGLES2Context *ctx, GLenum target, GLuint buffer) {
	GLuint *b;
	if (ctx && (b = binding(ctx, target))) *b = buffer;
}

/* The buffer bound to target, 0 for none or another target. */
GLuint boundBuffer(struct OGLES2Context *ctx, GLenum target) {
	GLuint *b;
	return ctx && (b = binding(ctx, target)) ? *b : 0;
}

void ogles2_GLBufferData(struct OGLES2Context *ctx, GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
	GLuint buffer = boundBuffer(ctx, target);
	ULONG *w;
	if (!buffer) return;
	w = qt_reserve(QT_OGLES2_BUFFER_DATA_WORDS);
	w[0] = ((ULONG) QT_OGLES2_BUFFER_DATA << 16) | QT_OGLES2_BUFFER_DATA_WORDS;
	w[1] = buffer;
	w[2] = (ULONG) size;
	w[3] = (ULONG) data;
	w[4] = usage;
	qt_flush();
}

/* glVertexAttribPointer, glEnable/DisableVertexAttribArray: 68k-side only
 * (gl/oglescmd.h); glDrawArrays/glDrawElements send the current state of
 * all QT_OGLES2_MAX_ATTRIBS of them with the draw. */
void ogles2_resetAttribs(QtOgles2Attrib *attribs) {
	int i;
	for (i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) attribs[i].binding = (GLuint) i;
}

void ogles2_bindingChanged(struct OGLES2Context *ctx, GLuint b) {
	const QtOgles2Binding *binding = &ctx->bindings[b];
	int i;
	for (i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) {
		QtOgles2Attrib *a = &ctx->attribs[i];
		if (a->binding != b) continue;
		a->bufferName = binding->buffer;
		a->pointer = binding->offset + a->relativeOffset;
		a->stride = binding->stride;
		a->divisor = binding->divisor;
	}
}

/* OpenGL ES 3.1's definition: the format with relative offset 0, the
 * attribute reading binding index, and that binding the buffer bound to
 * GL_ARRAY_BUFFER (or none: pointer is then an Amiga address) at pointer.
 * Stride 0 stays 0, which the host reads as tightly packed. */
void ogles2_vertexAttrib(struct OGLES2Context *ctx, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
		const void *pointer, GLboolean integer) {
	QtOgles2Attrib *a;
	QtOgles2Binding *b;
	if (!ctx || index >= QT_OGLES2_MAX_ATTRIBS) return;
	a = &ctx->attribs[index];
	a->size = size;
	a->type = type;
	a->normalized = normalized;
	a->integer = integer;
	a->relativeOffset = 0;
	a->binding = index;
	b = &ctx->bindings[index];
	b->buffer = ctx->arrayBuffer;
	b->offset = (ULONG) pointer;
	b->stride = stride;
	ogles2_bindingChanged(ctx, index);
}

void ogles2_GLVertexAttribPointer(struct OGLES2Context *ctx, GLuint index, GLint size, GLenum type, GLboolean normalized,
		GLsizei stride, const void *pointer) {
	ogles2_vertexAttrib(ctx, index, size, type, normalized, stride, pointer, GL_FALSE);
}

void ogles2_GLEnableVertexAttribArray(struct OGLES2Context *ctx, GLuint index) {
	if (ctx && index < QT_OGLES2_MAX_ATTRIBS) ctx->attribs[index].enabled = GL_TRUE;
}

void ogles2_GLDisableVertexAttribArray(struct OGLES2Context *ctx, GLuint index) {
	if (ctx && index < QT_OGLES2_MAX_ATTRIBS) ctx->attribs[index].enabled = GL_FALSE;
}

static ULONG *attribWords(ULONG *w, const QtOgles2Attrib *a) {
	w[0] = (a->enabled ? 1 : 0) | (a->integer ? 2 : 0);
	w[1] = (ULONG) a->size;
	w[2] = a->type;
	w[3] = a->normalized;
	w[4] = (ULONG) a->stride;
	w[5] = a->bufferName;
	w[6] = a->pointer;
	w[7] = a->divisor;
	return w + 8;
}

/* instances: 0 for a draw that is not instanced (glDrawArraysInstanced and
 * glDrawElementsInstanced give it, ogles3.library). */
void ogles2_draw(struct OGLES2Context *ctx, GLenum mode, GLint first, GLsizei count, GLenum indexType, const void *indices,
		GLsizei instances) {
	ULONG *w;
	int i;
	if (!ctx) return;
	w = qt_reserve(QT_OGLES2_DRAW_WORDS);
	w[0] = ((ULONG) QT_OGLES2_DRAW << 16) | QT_OGLES2_DRAW_WORDS;
	w[1] = mode;
	w[2] = (ULONG) first;
	w[3] = (ULONG) count;
	w[4] = indexType;
	w[5] = indexType ? ctx->elementArrayBuffer : 0;
	w[6] = (ULONG) indices;
	w[7] = (ULONG) instances;
	w += 8;
	for (i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) w = attribWords(w, &ctx->attribs[i]);
	qt_flush();
}

void ogles2_GLDrawArrays(struct OGLES2Context *ctx, GLenum mode, GLint first, GLsizei count) {
	ogles2_draw(ctx, mode, first, count, 0, 0, 0);
}

void ogles2_GLDrawElements(struct OGLES2Context *ctx, GLenum mode, GLsizei count, GLenum type, const void *indices) {
	ogles2_draw(ctx, mode, 0, count, type, indices, 0);
}

/*
 * Textures and framebuffers (stage 2, cube/boingocean): real host objects
 * (gl/oglescmd.h, host/ogles2.cpp), reached generically like shaders and
 * programs -- QT_OGLES2_GEN/DELETE (glGenTextures and the rest give back
 * real GLuint names, host-swapped, unlike QT_OGLES2_GEN_BUFFERS's fake
 * ones) and QT_OGLES2_CALL (the scalar entries, by subop).
 */

void genObjects(ULONG kind, GLsizei n, GLuint *names) {
	ULONG *w = qt_reserve(QT_OGLES2_GEN_WORDS);
	w[0] = ((ULONG) QT_OGLES2_GEN << 16) | QT_OGLES2_GEN_WORDS;
	w[1] = kind;
	w[2] = (ULONG) n;
	w[3] = (ULONG) names;
	qt_flush();
}

void deleteObjects(ULONG kind, GLsizei n, const GLuint *names) {
	ULONG *w = qt_reserve(QT_OGLES2_DELETE_WORDS);
	w[0] = ((ULONG) QT_OGLES2_DELETE << 16) | QT_OGLES2_DELETE_WORDS;
	w[1] = kind;
	w[2] = (ULONG) n;
	w[3] = (ULONG) names;
	qt_flush();
}

void ogles2_GLGenTextures(struct OGLES2Context *ctx, GLsizei n, GLuint *textures) { genObjects(QT_OGLES2_K_Texture, n, textures); }
void ogles2_GLDeleteTextures(struct OGLES2Context *ctx, GLsizei n, const GLuint *textures) {
	deleteObjects(QT_OGLES2_K_Texture, n, textures);
}
void ogles2_GLGenFramebuffers(struct OGLES2Context *ctx, GLsizei n, GLuint *framebuffers) {
	genObjects(QT_OGLES2_K_Framebuffer, n, framebuffers);
}
void ogles2_GLDeleteFramebuffers(struct OGLES2Context *ctx, GLsizei n, const GLuint *framebuffers) {
	deleteObjects(QT_OGLES2_K_Framebuffer, n, framebuffers);
}
void ogles2_GLGenRenderbuffers(struct OGLES2Context *ctx, GLsizei n, GLuint *renderbuffers) {
	genObjects(QT_OGLES2_K_Renderbuffer, n, renderbuffers);
}
void ogles2_GLDeleteRenderbuffers(struct OGLES2Context *ctx, GLsizei n, const GLuint *renderbuffers) {
	deleteObjects(QT_OGLES2_K_Renderbuffer, n, renderbuffers);
}

/* QT_OGLES2_CALL with up to ten argument words (words[0..9]). */
LONG ogles2_call10(ULONG subop, const ULONG *words) {
	ULONG *w = qt_reserve(QT_OGLES2_CALL_WORDS);
	int i;
	w[0] = ((ULONG) QT_OGLES2_CALL << 16) | QT_OGLES2_CALL_WORDS;
	w[1] = subop;
	for (i = 0; i < 10; ++i) w[2 + i] = words[i];
	return (LONG) qt_flush();
}

LONG call(ULONG subop, ULONG w0, ULONG w1, ULONG w2, ULONG w3, ULONG w4) {
	ULONG words[10] = {0};
	words[0] = w0;
	words[1] = w1;
	words[2] = w2;
	words[3] = w3;
	words[4] = w4;
	return ogles2_call10(subop, words);
}

void ogles2_GLBindTexture(struct OGLES2Context *ctx, GLenum target, GLuint texture) {
	call(QT_OGLES2_C_BindTexture, target, texture, 0, 0, 0);
}
void ogles2_GLActiveTexture(struct OGLES2Context *ctx, GLenum texture) { call(QT_OGLES2_C_ActiveTexture, texture, 0, 0, 0, 0); }
void ogles2_GLGenerateMipmap(struct OGLES2Context *ctx, GLenum target) { call(QT_OGLES2_C_GenerateMipmap, target, 0, 0, 0, 0); }
void ogles2_GLTexParameteri(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint param) {
	call(QT_OGLES2_C_TexParameteri, target, pname, (ULONG) param, 0, 0);
}
void ogles2_GLBindFramebuffer(struct OGLES2Context *ctx, GLenum target, GLuint framebuffer) {
	call(QT_OGLES2_C_BindFramebuffer, target, framebuffer, 0, 0, 0);
}
void ogles2_GLBindRenderbuffer(struct OGLES2Context *ctx, GLenum target, GLuint renderbuffer) {
	call(QT_OGLES2_C_BindRenderbuffer, target, renderbuffer, 0, 0, 0);
}
void ogles2_GLFramebufferTexture2D(struct OGLES2Context *ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
		GLint level) {
	call(QT_OGLES2_C_FramebufferTexture2D, target, attachment, textarget, texture, (ULONG) level);
}
void ogles2_GLFramebufferRenderbuffer(struct OGLES2Context *ctx, GLenum target, GLenum attachment, GLenum renderbuffertarget,
		GLuint renderbuffer) {
	call(QT_OGLES2_C_FramebufferRenderbuffer, target, attachment, renderbuffertarget, renderbuffer, 0);
}
GLenum ogles2_GLCheckFramebufferStatus(struct OGLES2Context *ctx, GLenum target) {
	return (GLenum) call(QT_OGLES2_C_CheckFramebufferStatus, target, 0, 0, 0, 0);
}
void ogles2_GLRenderbufferStorage(struct OGLES2Context *ctx, GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
	call(QT_OGLES2_C_RenderbufferStorage, target, internalformat, (ULONG) width, (ULONG) height, 0);
}

/* GL_UNSIGNED_BYTE pixels only so far (host/ogles2.cpp). */
void ogles2_GLTexImage2D(struct OGLES2Context *ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
		GLint border, GLenum format, GLenum type, const void *pixels) {
	ULONG *w = qt_reserve(QT_OGLES2_TEX_IMAGE_2D_WORDS);
	w[0] = ((ULONG) QT_OGLES2_TEX_IMAGE_2D << 16) | QT_OGLES2_TEX_IMAGE_2D_WORDS;
	w[1] = target;
	w[2] = (ULONG) level;
	w[3] = (ULONG) internalformat;
	w[4] = (ULONG) width;
	w[5] = (ULONG) height;
	w[6] = (ULONG) border;
	w[7] = format;
	w[8] = type;
	w[9] = (ULONG) pixels;
	qt_flush();
}

void ogles2_GLUniformMatrix4fv(struct OGLES2Context *ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	ULONG *w;
	if (transpose) {
		/* GL_INVALID_VALUE in GLES2; OpenGL ES 3.0 allows it. */
		if (ctx && ctx->esVersion >= 30) ogles2_uniformV(QT_OGLES2_U_M4, location, count, value, GL_TRUE);
		return;
	}
	w = qt_reserve(QT_OGLES2_UNIFORM_MATRIX4FV_WORDS);
	w[0] = ((ULONG) QT_OGLES2_UNIFORM_MATRIX4FV << 16) | QT_OGLES2_UNIFORM_MATRIX4FV_WORDS;
	w[1] = (ULONG) location;
	w[2] = (ULONG) count;
	w[3] = 0;
	w[4] = (ULONG) value;
	qt_flush();
}

/*
 * The rest of GLES2 (stage 3, gl/oglescmd.h). Entries desktop GL 1.1 has
 * with no pointer argument go through its existing _gl<Name> encoders;
 * small input arrays (glTexParameterfv, glVertexAttrib4fv, ...) are read
 * here and sent as scalars.
 */

void ogles2_GLBlendFunc(struct OGLES2Context *ctx, GLenum sfactor, GLenum dfactor) { _glBlendFunc(sfactor, dfactor); }
void ogles2_GLStencilFunc(struct OGLES2Context *ctx, GLenum func, GLint ref, GLuint mask) { _glStencilFunc(func, ref, mask); }
void ogles2_GLStencilMask(struct OGLES2Context *ctx, GLuint mask) { _glStencilMask(mask); }
void ogles2_GLStencilOp(struct OGLES2Context *ctx, GLenum fail, GLenum zfail, GLenum zpass) { _glStencilOp(fail, zfail, zpass); }
void ogles2_GLPolygonOffset(struct OGLES2Context *ctx, GLfloat factor, GLfloat units) { _glPolygonOffset(factor, units); }
void ogles2_GLPixelStorei(struct OGLES2Context *ctx, GLenum pname, GLint param) { _glPixelStorei(pname, param); }
void ogles2_GLDepthRangef(struct OGLES2Context *ctx, GLfloat n, GLfloat f) { _glDepthRange(n, f); }
void ogles2_GLPolygonMode(struct OGLES2Context *ctx, GLenum face, GLenum mode) { _glPolygonMode(face, mode); }
GLboolean ogles2_GLIsTexture(struct OGLES2Context *ctx, GLuint texture) { return _glIsTexture(texture); }
void ogles2_GLTexParameterf(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLfloat param) {
	_glTexParameterf(target, pname, param);
}
/* GLES2's texture parameters are all single values. */
void ogles2_GLTexParameterfv(struct OGLES2Context *ctx, GLenum target, GLenum pname, const GLfloat *params) {
	if (params) _glTexParameterf(target, pname, params[0]);
}
void ogles2_GLTexParameteriv(struct OGLES2Context *ctx, GLenum target, GLenum pname, const GLint *params) {
	if (params) call(QT_OGLES2_C_TexParameteri, target, pname, (ULONG) params[0], 0, 0);
}
void ogles2_GLCopyTexImage2D(struct OGLES2Context *ctx, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y,
		GLsizei width, GLsizei height, GLint border) {
	_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}
void ogles2_GLCopyTexSubImage2D(struct OGLES2Context *ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x,
		GLint y, GLsizei width, GLsizei height) {
	_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

void ogles2_GLBlendColor(struct OGLES2Context *ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	call(QT_OGLES2_C_BlendColor, floatWord(red), floatWord(green), floatWord(blue), floatWord(alpha), 0);
}
void ogles2_GLBlendEquation(struct OGLES2Context *ctx, GLenum mode) { call(QT_OGLES2_C_BlendEquation, mode, 0, 0, 0, 0); }
void ogles2_GLBlendEquationSeparate(struct OGLES2Context *ctx, GLenum modeRGB, GLenum modeAlpha) {
	call(QT_OGLES2_C_BlendEquationSeparate, modeRGB, modeAlpha, 0, 0, 0);
}
void ogles2_GLBlendFuncSeparate(struct OGLES2Context *ctx, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha,
		GLenum dfactorAlpha) {
	call(QT_OGLES2_C_BlendFuncSeparate, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha, 0);
}
void ogles2_GLStencilFuncSeparate(struct OGLES2Context *ctx, GLenum face, GLenum func, GLint ref, GLuint mask) {
	call(QT_OGLES2_C_StencilFuncSeparate, face, func, (ULONG) ref, mask, 0);
}
void ogles2_GLStencilMaskSeparate(struct OGLES2Context *ctx, GLenum face, GLuint mask) {
	call(QT_OGLES2_C_StencilMaskSeparate, face, mask, 0, 0, 0);
}
void ogles2_GLStencilOpSeparate(struct OGLES2Context *ctx, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
	call(QT_OGLES2_C_StencilOpSeparate, face, sfail, dpfail, dppass, 0);
}
void ogles2_GLSampleCoverage(struct OGLES2Context *ctx, GLfloat value, GLboolean invert) {
	call(QT_OGLES2_C_SampleCoverage, floatWord(value), invert, 0, 0, 0);
}
GLboolean ogles2_GLIsFramebuffer(struct OGLES2Context *ctx, GLuint framebuffer) {
	return (GLboolean) call(QT_OGLES2_C_IsFramebuffer, framebuffer, 0, 0, 0, 0);
}
GLboolean ogles2_GLIsRenderbuffer(struct OGLES2Context *ctx, GLuint renderbuffer) {
	return (GLboolean) call(QT_OGLES2_C_IsRenderbuffer, renderbuffer, 0, 0, 0, 0);
}
GLboolean ogles2_GLIsBuffer(struct OGLES2Context *ctx, GLuint buffer) { return (GLboolean) call(QT_OGLES2_C_IsBuffer, buffer, 0, 0, 0, 0); }
void ogles2_GLProvokingVertex(struct OGLES2Context *ctx, GLenum mode) { call(QT_OGLES2_C_ProvokingVertex, mode, 0, 0, 0, 0); }

/* Hints, or nothing to do on a host that always has its compiler. */
void ogles2_GLReleaseShaderCompiler(struct OGLES2Context *ctx) {}
void ogles2_GLDiscardFramebufferEXT(struct OGLES2Context *ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments) {}
void ogles2_GLInsertEventMarkerEXT(struct OGLES2Context *ctx, GLsizei length, const GLchar *marker) {}
void ogles2_GLPushGroupMarkerEXT(struct OGLES2Context *ctx, GLsizei length, const GLchar *marker) {}
void ogles2_GLPopGroupMarkerEXT(struct OGLES2Context *ctx) {}

/* The host compiles GLSL ES as desktop GLSL: IEEE single precision floats
 * and 32-bit ints whatever the precision qualifier (the values a desktop
 * GLES2 implementation reports for highp). */
void ogles2_GLGetShaderPrecisionFormat(struct OGLES2Context *ctx, GLenum shadertype, GLenum precisiontype, GLint *range,
		GLint *precision) {
	int isFloat = precisiontype == GL_LOW_FLOAT || precisiontype == GL_MEDIUM_FLOAT || precisiontype == GL_HIGH_FLOAT;
	if (range) {
		range[0] = isFloat ? 127 : 31;
		range[1] = isFloat ? 127 : 30;
	}
	if (precision) *precision = isFloat ? 23 : 0;
}

/* Generic vertex attributes: their current values, used by a disabled
 * attribute array, live on the host (glVertexAttrib4f there); a copy is
 * kept in the context for glGetVertexAttribfv. */
static void vertexAttrib(struct OGLES2Context *ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	if (!ctx) return;
	if (index < QT_OGLES2_MAX_ATTRIBS) {
		ctx->currentAttrib[index][0] = x;
		ctx->currentAttrib[index][1] = y;
		ctx->currentAttrib[index][2] = z;
		ctx->currentAttrib[index][3] = w;
	}
	call(QT_OGLES2_C_VertexAttrib4f, index, floatWord(x), floatWord(y), floatWord(z), floatWord(w));
}

void ogles2_GLVertexAttrib1f(struct OGLES2Context *ctx, GLuint index, GLfloat x) { vertexAttrib(ctx, index, x, 0, 0, 1); }
void ogles2_GLVertexAttrib2f(struct OGLES2Context *ctx, GLuint index, GLfloat x, GLfloat y) { vertexAttrib(ctx, index, x, y, 0, 1); }
void ogles2_GLVertexAttrib3f(struct OGLES2Context *ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z) {
	vertexAttrib(ctx, index, x, y, z, 1);
}
void ogles2_GLVertexAttrib4f(struct OGLES2Context *ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	vertexAttrib(ctx, index, x, y, z, w);
}
void ogles2_GLVertexAttrib1fv(struct OGLES2Context *ctx, GLuint index, const GLfloat *v) { if (v) vertexAttrib(ctx, index, v[0], 0, 0, 1); }
void ogles2_GLVertexAttrib2fv(struct OGLES2Context *ctx, GLuint index, const GLfloat *v) {
	if (v) vertexAttrib(ctx, index, v[0], v[1], 0, 1);
}
void ogles2_GLVertexAttrib3fv(struct OGLES2Context *ctx, GLuint index, const GLfloat *v) {
	if (v) vertexAttrib(ctx, index, v[0], v[1], v[2], 1);
}
void ogles2_GLVertexAttrib4fv(struct OGLES2Context *ctx, GLuint index, const GLfloat *v) {
	if (v) vertexAttrib(ctx, index, v[0], v[1], v[2], v[3]);
}

/* glGetVertexAttrib*: the array state is 68k-side only (QtOgles2Attrib). */
static int vertexAttribValue(struct OGLES2Context *ctx, GLuint index, GLenum pname, GLint *value) {
	const QtOgles2Attrib *a;
	if (!ctx || index >= QT_OGLES2_MAX_ATTRIBS) return 0;
	a = &ctx->attribs[index];
	switch (pname) {
	case GL_VERTEX_ATTRIB_ARRAY_ENABLED: *value = a->enabled; return 1;
	case GL_VERTEX_ATTRIB_ARRAY_SIZE: *value = a->size ? a->size : 4; return 1;
	case GL_VERTEX_ATTRIB_ARRAY_STRIDE: *value = a->stride; return 1;
	case GL_VERTEX_ATTRIB_ARRAY_TYPE: *value = a->type ? a->type : GL_FLOAT; return 1;
	case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED: *value = a->normalized; return 1;
	case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING: *value = a->bufferName; return 1;
	case 0x82D4: *value = (GLint) a->binding; return 1;        /* GL_VERTEX_ATTRIB_BINDING (ES 3.1) */
	case 0x82D5: *value = (GLint) a->relativeOffset; return 1; /* GL_VERTEX_ATTRIB_RELATIVE_OFFSET */
	}
	return 0;
}

void ogles2_GLGetVertexAttribiv(struct OGLES2Context *ctx, GLuint index, GLenum pname, GLint *params) {
	int i;
	if (!params || !ctx) return;
	if (pname == GL_CURRENT_VERTEX_ATTRIB) {
		if (index < QT_OGLES2_MAX_ATTRIBS) for (i = 0; i < 4; ++i) params[i] = (GLint) ctx->currentAttrib[index][i];
		return;
	}
	vertexAttribValue(ctx, index, pname, params);
}

void ogles2_GLGetVertexAttribfv(struct OGLES2Context *ctx, GLuint index, GLenum pname, GLfloat *params) {
	GLint value;
	int i;
	if (!params || !ctx) return;
	if (pname == GL_CURRENT_VERTEX_ATTRIB) {
		if (index < QT_OGLES2_MAX_ATTRIBS) for (i = 0; i < 4; ++i) params[i] = ctx->currentAttrib[index][i];
		return;
	}
	if (vertexAttribValue(ctx, index, pname, &value)) params[0] = (GLfloat) value;
}

void ogles2_GLGetVertexAttribPointerv(struct OGLES2Context *ctx, GLuint index, GLenum pname, void **pointer) {
	if (ctx && pointer && index < QT_OGLES2_MAX_ATTRIBS && pname == GL_VERTEX_ATTRIB_ARRAY_POINTER) {
		*pointer = (void *) ctx->attribs[index].pointer;
	}
}

void ogles2_uniformV(ULONG subop, GLint location, GLsizei count, const void *value, GLboolean transpose) {
	ULONG *w = qt_reserve(QT_OGLES2_UNIFORM_V_WORDS);
	w[0] = ((ULONG) QT_OGLES2_UNIFORM_V << 16) | QT_OGLES2_UNIFORM_V_WORDS;
	w[1] = subop;
	w[2] = (ULONG) location;
	w[3] = (ULONG) count;
	w[4] = (ULONG) value;
	w[5] = transpose ? 1 : 0;
	qt_flush();
}

#define uniformV(subop, location, count, value) ogles2_uniformV(subop, location, count, value, GL_FALSE)

void ogles2_GLUniform1fv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLfloat *value) {
	uniformV(QT_OGLES2_U_1F, location, count, value);
}
void ogles2_GLUniform2fv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLfloat *value) {
	uniformV(QT_OGLES2_U_2F, location, count, value);
}
void ogles2_GLUniform3fv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLfloat *value) {
	uniformV(QT_OGLES2_U_3F, location, count, value);
}
void ogles2_GLUniform4fv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLfloat *value) {
	uniformV(QT_OGLES2_U_4F, location, count, value);
}
void ogles2_GLUniform1iv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLint *value) {
	uniformV(QT_OGLES2_U_1I, location, count, value);
}
void ogles2_GLUniform2iv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLint *value) {
	uniformV(QT_OGLES2_U_2I, location, count, value);
}
void ogles2_GLUniform3iv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLint *value) {
	uniformV(QT_OGLES2_U_3I, location, count, value);
}
void ogles2_GLUniform4iv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLint *value) {
	uniformV(QT_OGLES2_U_4I, location, count, value);
}
/* transpose must be GL_FALSE in GLES2 (GL_INVALID_VALUE otherwise); OpenGL
 * ES 3.0 allows it. */
void ogles2_GLUniformMatrix2fv(struct OGLES2Context *ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	if (!transpose || (ctx && ctx->esVersion >= 30)) ogles2_uniformV(QT_OGLES2_U_M2, location, count, value, transpose);
}
void ogles2_GLUniformMatrix3fv(struct OGLES2Context *ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	if (!transpose || (ctx && ctx->esVersion >= 30)) ogles2_uniformV(QT_OGLES2_U_M3, location, count, value, transpose);
}

void bufferCopy(ULONG opcode, GLuint buffer, ULONG offset, ULONG size, const void *data) {
	ULONG *w;
	if (!buffer) return;
	w = qt_reserve(QT_OGLES2_BUFFER_SUB_DATA_WORDS);
	w[0] = (opcode << 16) | QT_OGLES2_BUFFER_SUB_DATA_WORDS;
	w[1] = buffer;
	w[2] = offset;
	w[3] = size;
	w[4] = (ULONG) data;
	qt_flush();
}

void ogles2_GLBufferSubData(struct OGLES2Context *ctx, GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
	bufferCopy(QT_OGLES2_BUFFER_SUB_DATA, boundBuffer(ctx, target), (ULONG) offset, (ULONG) size, data);
}

void getParam(ULONG subop, ULONG w0, ULONG w1, ULONG w2, void *out) {
	ULONG *w = qt_reserve(QT_OGLES2_GET_PARAM_WORDS);
	w[0] = ((ULONG) QT_OGLES2_GET_PARAM << 16) | QT_OGLES2_GET_PARAM_WORDS;
	w[1] = subop;
	w[2] = w0;
	w[3] = w1;
	w[4] = w2;
	w[5] = (ULONG) out;
	qt_flush();
}

/* glMapBufferOES: the fake buffers live on the host, so a mapping is a
 * 68k-side copy, written back whole by glUnmapBufferOES. */
#define GL_WRITE_ONLY_OES 0x88B9
#define GL_BUFFER_ACCESS_OES 0x88BB
#define GL_BUFFER_MAPPED_OES 0x88BC
#define GL_BUFFER_MAP_POINTER_OES 0x88BD

/* 0 for GL_ARRAY_BUFFER, 1 for GL_ELEMENT_ARRAY_BUFFER, -1 for others. */
static int bufferSlot(GLenum target) {
	return target == GL_ARRAY_BUFFER ? 0 : target == GL_ELEMENT_ARRAY_BUFFER ? 1 : -1;
}

void ogles2_GLGetBufferParameteriv(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint *params) {
	int slot = bufferSlot(target);
	if (!params || slot < 0 || !ctx) return;
	if (pname == GL_BUFFER_MAPPED_OES) *params = ctx->mapped[slot] != NULL;
	else if (pname == GL_BUFFER_ACCESS_OES) *params = GL_WRITE_ONLY_OES;
	else getParam(QT_OGLES2_P_BufferParameteriv, boundBuffer(ctx, target), pname, 0, params);
}
void ogles2_GLGetBufferParameterivOES(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint *params) {
	ogles2_GLGetBufferParameteriv(ctx, target, pname, params);
}

void *ogles2_GLMapBufferOES(struct OGLES2Context *ctx, GLenum target, GLenum access) {
	int slot = bufferSlot(target);
	GLuint buffer = boundBuffer(ctx, target);
	GLint size = 0;
	if (slot < 0 || !buffer || ctx->mapped[slot]) return NULL;
	getParam(QT_OGLES2_P_BufferParameteriv, buffer, GL_BUFFER_SIZE, 0, &size);
	if (size <= 0) return NULL;
	ctx->mapped[slot] = AllocVec((ULONG) size, MEMF_ANY);
	if (!ctx->mapped[slot]) return NULL;
	ctx->mappedSize[slot] = (ULONG) size;
	bufferCopy(QT_OGLES2_READ_BUFFER, buffer, 0, (ULONG) size, ctx->mapped[slot]);
	return ctx->mapped[slot];
}

GLboolean ogles2_GLUnmapBufferOES(struct OGLES2Context *ctx, GLenum target) {
	int slot = bufferSlot(target);
	if (!ctx || slot < 0 || !ctx->mapped[slot]) return GL_FALSE;
	bufferCopy(QT_OGLES2_BUFFER_SUB_DATA, boundBuffer(ctx, target), 0, ctx->mappedSize[slot], ctx->mapped[slot]);
	FreeVec(ctx->mapped[slot]);
	ctx->mapped[slot] = NULL;
	return GL_TRUE;
}

void ogles2_GLGetBufferPointervOES(struct OGLES2Context *ctx, GLenum target, GLenum pname, void **params) {
	int slot = bufferSlot(target);
	if (params) *params = ctx && slot >= 0 && pname == GL_BUFFER_MAP_POINTER_OES ? ctx->mapped[slot] : NULL;
}

void ogles2_GLGetTexParameteriv(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint *params) {
	getParam(QT_OGLES2_P_TexParameteriv, target, pname, 0, params);
}
void ogles2_GLGetTexParameterfv(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLfloat *params) {
	getParam(QT_OGLES2_P_TexParameterfv, target, pname, 0, params);
}
void ogles2_GLGetRenderbufferParameteriv(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint *params) {
	getParam(QT_OGLES2_P_RenderbufferParameteriv, target, pname, 0, params);
}
void ogles2_GLGetFramebufferAttachmentParameteriv(struct OGLES2Context *ctx, GLenum target, GLenum attachment, GLenum pname,
		GLint *params) {
	getParam(QT_OGLES2_P_FramebufferAttachmentParameteriv, target, attachment, pname, params);
}
void ogles2_GLGetUniformfv(struct OGLES2Context *ctx, GLuint program, GLint location, GLfloat *params) {
	getParam(QT_OGLES2_P_Uniformfv, program, (ULONG) location, 0, params);
}
void ogles2_GLGetUniformiv(struct OGLES2Context *ctx, GLuint program, GLint location, GLint *params) {
	getParam(QT_OGLES2_P_Uniformiv, program, (ULONG) location, 0, params);
}

/* glGet*: the buffer bindings and the attribute count are 68k-side state;
 * the rest is the host's (QT_OGLES2_GET). */
static int localValue(struct OGLES2Context *ctx, GLenum pname, GLint *value) {
	switch (pname) {
	case GL_ARRAY_BUFFER_BINDING: *value = ctx ? ctx->arrayBuffer : 0; return 1;
	case GL_ELEMENT_ARRAY_BUFFER_BINDING: *value = ctx ? ctx->elementArrayBuffer : 0; return 1;
	case GL_MAX_VERTEX_ATTRIBS: *value = QT_OGLES2_MAX_ATTRIBS; return 1;
	}
	if (!ctx || ctx->esVersion < 30) return 0;
#ifdef QT_OGLES3
	{
		int ogles2_debugValue(struct OGLES2Context *ctx, GLenum pname, GLint *value); /* es3more.c */
		int ogles2_extensionCount(struct OGLES2Context *ctx); /* es3.c */
		if (ogles2_debugValue(ctx, pname, value)) return 1;
		/* GL_NUM_EXTENSIONS: the length of glGetStringi's list, not the host's count. */
		if (pname == 0x821D) { *value = ogles2_extensionCount(ctx); return 1; }
	}
#endif
	/* OpenGL ES 3.x: its version, not the host's, and the buffers bound to
	 * the generic targets (fake names, 68k-side). */
	switch (pname) {
	case 0x821B: *value = 3; return 1;                               /* GL_MAJOR_VERSION */
	case 0x821C: *value = ctx->esVersion == 31 ? 1 : 0; return 1;    /* GL_MINOR_VERSION */
	case 0x8A28: *value = ctx->uniformBuffer; return 1;              /* GL_UNIFORM_BUFFER_BINDING */
	case 0x8F36: *value = ctx->copyReadBuffer; return 1;             /* GL_COPY_READ_BUFFER_BINDING */
	case 0x8F37: *value = ctx->copyWriteBuffer; return 1;            /* GL_COPY_WRITE_BUFFER_BINDING */
	case 0x88ED: *value = ctx->pixelPackBuffer; return 1;            /* GL_PIXEL_PACK_BUFFER_BINDING */
	case 0x88EF: *value = ctx->pixelUnpackBuffer; return 1;          /* GL_PIXEL_UNPACK_BUFFER_BINDING */
	case 0x8C8F: *value = ctx->transformFeedbackBuffer; return 1;    /* GL_TRANSFORM_FEEDBACK_BUFFER_BINDING */
	}
	if (ctx->esVersion < 31) return 0;
	switch (pname) {
	case 0x82DA: *value = QT_OGLES2_MAX_ATTRIBS; return 1;           /* GL_MAX_VERTEX_ATTRIB_BINDINGS */
	case 0x82E5: *value = 2048; return 1;                            /* GL_MAX_VERTEX_ATTRIB_STRIDE (OpenGL 4.4's on the host) */
	case 0x90D3: *value = ctx->shaderStorageBuffer; return 1;        /* GL_SHADER_STORAGE_BUFFER_BINDING */
	case 0x92C1: *value = ctx->atomicCounterBuffer; return 1;        /* GL_ATOMIC_COUNTER_BUFFER_BINDING */
	case 0x90EF: *value = ctx->dispatchIndirectBuffer; return 1;     /* GL_DISPATCH_INDIRECT_BUFFER_BINDING */
	case 0x8F43: *value = ctx->drawIndirectBuffer; return 1;         /* GL_DRAW_INDIRECT_BUFFER_BINDING */
	}
	return 0;
}

static void get(ULONG kind, GLenum pname, void *data) {
	ULONG *w = qt_reserve(QT_OGLES2_GET_WORDS);
	w[0] = ((ULONG) QT_OGLES2_GET << 16) | QT_OGLES2_GET_WORDS;
	w[1] = kind;
	w[2] = pname;
	w[3] = (ULONG) data;
	qt_flush();
}

void ogles2_GLGetIntegerv(struct OGLES2Context *ctx, GLenum pname, GLint *data) {
	if (data && !localValue(ctx, pname, data)) get(QT_OGLES2_GET_Integer, pname, data);
}
void ogles2_GLGetFloatv(struct OGLES2Context *ctx, GLenum pname, GLfloat *data) {
	GLint value;
	if (!data) return;
	if (localValue(ctx, pname, &value)) *data = (GLfloat) value;
	else get(QT_OGLES2_GET_Float, pname, data);
}
void ogles2_GLGetBooleanv(struct OGLES2Context *ctx, GLenum pname, GLboolean *data) {
	GLint value;
	if (!data) return;
	if (localValue(ctx, pname, &value)) *data = value != 0;
	else get(QT_OGLES2_GET_Boolean, pname, data);
}

void ogles2_getActive(ULONG subop, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type,
		GLchar *name) {
	ULONG *w = qt_reserve(QT_OGLES2_GET_ACTIVE_WORDS);
	w[0] = ((ULONG) QT_OGLES2_GET_ACTIVE << 16) | QT_OGLES2_GET_ACTIVE_WORDS;
	w[1] = subop;
	w[2] = program;
	w[3] = index;
	w[4] = (ULONG) bufSize;
	w[5] = (ULONG) length;
	w[6] = (ULONG) size;
	w[7] = (ULONG) type;
	w[8] = (ULONG) name;
	qt_flush();
}

void ogles2_GLGetActiveAttrib(struct OGLES2Context *ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size,
		GLenum *type, GLchar *name) {
	ogles2_getActive(QT_OGLES2_A_Attrib, program, index, bufSize, length, size, type, name);
}
void ogles2_GLGetActiveUniform(struct OGLES2Context *ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size,
		GLenum *type, GLchar *name) {
	ogles2_getActive(QT_OGLES2_A_Uniform, program, index, bufSize, length, size, type, name);
}

void ogles2_GLGetAttachedShaders(struct OGLES2Context *ctx, GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) {
	ULONG *w = qt_reserve(QT_OGLES2_GET_ATTACHED_SHADERS_WORDS);
	w[0] = ((ULONG) QT_OGLES2_GET_ATTACHED_SHADERS << 16) | QT_OGLES2_GET_ATTACHED_SHADERS_WORDS;
	w[1] = program;
	w[2] = (ULONG) maxCount;
	w[3] = (ULONG) count;
	w[4] = (ULONG) shaders;
	qt_flush();
}

void ogles2_GLGetShaderSource(struct OGLES2Context *ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) {
	getLog(QT_OGLES2_G_ShaderSource, shader, bufSize, length, source);
}

void ogles2_GLTexSubImage2D(struct OGLES2Context *ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
		GLsizei height, GLenum format, GLenum type, const void *pixels) {
	ULONG *w = qt_reserve(QT_OGLES2_TEX_SUB_IMAGE_2D_WORDS);
	w[0] = ((ULONG) QT_OGLES2_TEX_SUB_IMAGE_2D << 16) | QT_OGLES2_TEX_SUB_IMAGE_2D_WORDS;
	w[1] = target;
	w[2] = (ULONG) level;
	w[3] = (ULONG) xoffset;
	w[4] = (ULONG) yoffset;
	w[5] = (ULONG) width;
	w[6] = (ULONG) height;
	w[7] = format;
	w[8] = type;
	w[9] = (ULONG) pixels;
	qt_flush();
}

void ogles2_GLReadPixels(struct OGLES2Context *ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
		void *pixels) {
	ULONG *w = qt_reserve(QT_OGLES2_READ_PIXELS_WORDS);
	w[0] = ((ULONG) QT_OGLES2_READ_PIXELS << 16) | QT_OGLES2_READ_PIXELS_WORDS;
	w[1] = (ULONG) x;
	w[2] = (ULONG) y;
	w[3] = (ULONG) width;
	w[4] = (ULONG) height;
	w[5] = format;
	w[6] = type;
	w[7] = (ULONG) pixels;
	qt_flush();
}

void ogles2_GLDrawBuffersEXT(struct OGLES2Context *ctx, GLsizei n, const GLenum *bufs) {
	ULONG *w = qt_reserve(QT_OGLES2_DRAW_BUFFERS_WORDS);
	w[0] = ((ULONG) QT_OGLES2_DRAW_BUFFERS << 16) | QT_OGLES2_DRAW_BUFFERS_WORDS;
	w[1] = (ULONG) n;
	w[2] = (ULONG) bufs;
	qt_flush();
}

/* start/end are only a hint. */
void ogles2_GLDrawRangeElements(struct OGLES2Context *ctx, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
		const void *indices) {
	ogles2_draw(ctx, mode, 0, count, type, indices, 0);
}

/* QT_OGLES2_DRAW's first is the base vertex of an indexed draw. */
void ogles2_GLDrawElementsBaseVertexOES(struct OGLES2Context *ctx, GLenum mode, GLsizei count, GLenum type, const void *indices,
		GLint basevertex) {
	ogles2_draw(ctx, mode, basevertex, count, type, indices, 0);
}

/*
 * Compressed textures (gl/oglescmd.h, QT_OGLES2_COMPRESSED_TEX): ETC1,
 * OpenGL ES 3.0's ETC2/EAC formats and S3TC's DXT1/3/5, passed to the host
 * as they are.
 */
void ogles2_compressedTex(ULONG subop, GLenum target, GLint level, ULONG xOrFormat, GLint y, GLint z, GLsizei width,
		GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data) {
	ULONG *w = qt_reserve(QT_OGLES2_COMPRESSED_TEX_WORDS);
	w[0] = ((ULONG) QT_OGLES2_COMPRESSED_TEX << 16) | QT_OGLES2_COMPRESSED_TEX_WORDS;
	w[1] = subop;
	w[2] = target;
	w[3] = (ULONG) level;
	w[4] = xOrFormat;
	w[5] = (ULONG) y;
	w[6] = (ULONG) z;
	w[7] = (ULONG) width;
	w[8] = (ULONG) height;
	w[9] = (ULONG) depth;
	w[10] = format;
	w[11] = (ULONG) imageSize;
	w[12] = (ULONG) data;
	qt_flush();
}
void ogles2_GLCompressedTexImage2D(struct OGLES2Context *ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width,
		GLsizei height, GLint border, GLsizei imageSize, const void *data) {
	if (border) { ogles2_error(ctx, GL_INVALID_VALUE); return; }
	ogles2_compressedTex(QT_OGLES2_CT_Image2D, target, level, internalformat, 0, 0, width, height, 1, 0, imageSize, data);
}
void ogles2_GLCompressedTexSubImage2D(struct OGLES2Context *ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset,
		GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) {
	ogles2_compressedTex(QT_OGLES2_CT_Sub2D, target, level, (ULONG) xoffset, yoffset, 0, width, height, 1, format, imageSize, data);
}

/* No shader or program binary formats are offered (GL_NUM_*_BINARY_FORMATS
 * are 0): a binary of any format is refused, and a program has none to give
 * (GL_PROGRAM_BINARY_LENGTH 0), so programs compile their sources. */
void ogles2_GLShaderBinary(struct OGLES2Context *ctx, GLsizei count, const GLuint *shaders, GLenum binaryFormat,
		const void *binary, GLsizei length) {
	ogles2_error(ctx, GL_INVALID_ENUM);
}
void ogles2_GLGetProgramBinaryOES(struct OGLES2Context *ctx, GLuint program, GLsizei bufSize, GLsizei *length,
		GLenum *binaryFormat, void *binary) {
	if (length) *length = 0;
	ogles2_error(ctx, GL_INVALID_OPERATION);
}
void ogles2_GLProgramBinaryOES(struct OGLES2Context *ctx, GLuint program, GLenum binaryFormat, const void *binary, GLint length) {
	ogles2_error(ctx, GL_INVALID_ENUM);
}

/* glClipControlEXT: the host's glClipControl (OpenGL 4.5), where it has one;
 * GL_INVALID_OPERATION where not (the extension is not offered). */
void ogles2_GLClipControlEXT(struct OGLES2Context *ctx, GLenum origin, GLenum depth) {
	if (!ogles2_call(QT_OGLES2_C_ClipControl, origin, depth, 0, 0, 0)) ogles2_error(ctx, GL_INVALID_OPERATION);
}

/* lib.c: an entry the library does not implement yet. */
void ogles2_missing(const char *name) {
	static const char *logged[64];
	static int count;
	int i;
	for (i = 0; i < count; ++i) if (logged[i] == name) return;
	if (count < 64) logged[count++] = name;
	logString("ogles2.library: not implemented:");
	logString((char *) name);
}
