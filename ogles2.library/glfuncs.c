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
