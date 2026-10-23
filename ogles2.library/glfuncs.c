#include "ogles2.h"
#include "gldeclarations.auto.h"

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
GLenum ogles2_GLGetError(struct OGLES2Context *ctx) { return _glGetError(); }
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
static const char renderer[] = QT_PRODUCT " ogles2.library (host OpenGL)";
static const char version[] = "OpenGL ES 2.0";
static const char shadingVersion[] = "OpenGL ES GLSL ES 1.00";
/* No extensions offered yet (stage 1 draws nothing of its own); real ones
 * (OES_texture_npot etc.) are added as the GL entries they need are. */
static const char extensions[] = "";

const GLubyte *ogles2_GLGetString(struct OGLES2Context *ctx, GLenum name) {
	switch (name) {
	case GL_VENDOR: return (const GLubyte *) vendor;
	case GL_RENDERER: return (const GLubyte *) renderer;
	case GL_VERSION: return (const GLubyte *) version;
	case GL_SHADING_LANGUAGE_VERSION: return (const GLubyte *) shadingVersion;
	case GL_EXTENSIONS: return (const GLubyte *) extensions;
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
static ULONG floatWord(GLfloat f) {
	union { GLfloat f; ULONG l; } u;
	u.f = f;
	return u.l;
}

static LONG simple(ULONG subop, ULONG w0, ULONG w1) {
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

/* count > 1 or an explicit length: not supported yet (QT_OGLES2_SHADER_
 * SOURCE), see gl/oglescmd.h. */
void ogles2_GLShaderSource(struct OGLES2Context *ctx, GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length) {
	ULONG *w;
	if (count != 1 || length || !string || !string[0]) {
		ogles2_missing("GLShaderSource (several chunks or an explicit length)");
		return;
	}
	w = qt_reserve(QT_OGLES2_SHADER_SOURCE_WORDS);
	w[0] = ((ULONG) QT_OGLES2_SHADER_SOURCE << 16) | QT_OGLES2_SHADER_SOURCE_WORDS;
	w[1] = shader;
	w[2] = (ULONG) string[0];
	qt_flush();
}

static LONG nameCall(ULONG subop, ULONG program, ULONG index, const char *text) {
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

static void uniform(ULONG subop, GLint location, ULONG w0, ULONG w1, ULONG w2, ULONG w3) {
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
	ULONG *w = qt_reserve(QT_OGLES2_GET_IV_WORDS);
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

/* GL_ARRAY_BUFFER/GL_ELEMENT_ARRAY_BUFFER bindings, kept here (not only on
 * the host) so glBufferData's caller does not have to be told which one is
 * bound, and so glVertexAttribPointer/glDrawElements can send the buffer
 * name that was bound *at that call*, as GLES2 itself requires. */
static GLuint boundArrayBuffer, boundElementArrayBuffer;

void ogles2_GLBindBuffer(struct OGLES2Context *ctx, GLenum target, GLuint buffer) {
	ULONG *w;
	if (target == GL_ARRAY_BUFFER) boundArrayBuffer = buffer;
	else if (target == GL_ELEMENT_ARRAY_BUFFER) boundElementArrayBuffer = buffer;
	w = qt_reserve(QT_OGLES2_BIND_BUFFER_WORDS);
	w[0] = ((ULONG) QT_OGLES2_BIND_BUFFER << 16) | QT_OGLES2_BIND_BUFFER_WORDS;
	w[1] = target;
	w[2] = buffer;
	qt_flush();
}

void ogles2_GLBufferData(struct OGLES2Context *ctx, GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
	ULONG *w = qt_reserve(QT_OGLES2_BUFFER_DATA_WORDS);
	w[0] = ((ULONG) QT_OGLES2_BUFFER_DATA << 16) | QT_OGLES2_BUFFER_DATA_WORDS;
	w[1] = target;
	w[2] = (ULONG) size;
	w[3] = (ULONG) data;
	w[4] = usage;
	qt_flush();
}

/* glVertexAttribPointer, glEnable/DisableVertexAttribArray: 68k-side only
 * (gl/oglescmd.h); glDrawArrays/glDrawElements send the current state of
 * all QT_OGLES2_MAX_ATTRIBS of them with the draw. */
typedef struct {
	GLboolean enabled;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	GLuint bufferName;  /* the one bound to GL_ARRAY_BUFFER at the call, or 0 */
	ULONG pointer;       /* an offset into it, or an Amiga address */
} Attrib;

static Attrib attribs[QT_OGLES2_MAX_ATTRIBS];

void ogles2_GLVertexAttribPointer(struct OGLES2Context *ctx, GLuint index, GLint size, GLenum type, GLboolean normalized,
		GLsizei stride, const void *pointer) {
	Attrib *a;
	if (index >= QT_OGLES2_MAX_ATTRIBS) return;
	a = &attribs[index];
	a->size = size;
	a->type = type;
	a->normalized = normalized;
	a->stride = stride;
	a->bufferName = boundArrayBuffer;
	a->pointer = (ULONG) pointer;
}

void ogles2_GLEnableVertexAttribArray(struct OGLES2Context *ctx, GLuint index) {
	if (index < QT_OGLES2_MAX_ATTRIBS) attribs[index].enabled = GL_TRUE;
}

void ogles2_GLDisableVertexAttribArray(struct OGLES2Context *ctx, GLuint index) {
	if (index < QT_OGLES2_MAX_ATTRIBS) attribs[index].enabled = GL_FALSE;
}

static ULONG *attribWords(ULONG *w, const Attrib *a) {
	w[0] = a->enabled;
	w[1] = (ULONG) a->size;
	w[2] = a->type;
	w[3] = a->normalized;
	w[4] = (ULONG) a->stride;
	w[5] = a->bufferName;
	w[6] = a->pointer;
	return w + 7;
}

static void draw(GLenum mode, GLint first, GLsizei count, GLenum indexType, GLuint indexBuffer, const void *indices) {
	ULONG *w;
	int i;
	w = qt_reserve(QT_OGLES2_DRAW_WORDS);
	w[0] = ((ULONG) QT_OGLES2_DRAW << 16) | QT_OGLES2_DRAW_WORDS;
	w[1] = mode;
	w[2] = (ULONG) first;
	w[3] = (ULONG) count;
	w[4] = indexType;
	w[5] = indexBuffer;
	w[6] = (ULONG) indices;
	w += 7;
	for (i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) w = attribWords(w, &attribs[i]);
	qt_flush();
}

void ogles2_GLDrawArrays(struct OGLES2Context *ctx, GLenum mode, GLint first, GLsizei count) {
	draw(mode, first, count, 0, 0, 0);
}

void ogles2_GLDrawElements(struct OGLES2Context *ctx, GLenum mode, GLsizei count, GLenum type, const void *indices) {
	draw(mode, 0, count, type, boundElementArrayBuffer, indices);
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
