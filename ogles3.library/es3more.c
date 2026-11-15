#include "ogles2.h"
#include <proto/exec.h>
#include <string.h>

/*
 * The entries the SDK's table has beyond OpenGL ES 3.1 proper
 * (docs/phase10-ogles3.md, "The last entries"): 3D compressed textures and
 * program binaries (ES 3.0), GL_KHR_debug, GL_OES_texture_3D's
 * glFramebufferTexture3DOES, GL_EXT_multi_draw_arrays, the base vertex draws
 * of GL_OES/EXT_draw_elements_base_vertex and GL_OES/EXT_primitive_bounding_
 * box.
 */

/* glfuncs.c's and es3.c's. */
GLboolean ogles2_GLIsEnabled(struct OGLES2Context *ctx, GLenum cap);
void ogles2_GLGetProgramBinaryOES(struct OGLES2Context *ctx, GLuint program, GLsizei bufSize, GLsizei *length,
	GLenum *binaryFormat, void *binary);
void ogles2_GLFramebufferTextureLayer(struct OGLES2Context *ctx, GLenum target, GLenum attachment, GLuint texture, GLint level,
	GLint layer);

/* --- Compressed 3D textures (glfuncs.c has the 2D ones) ------------------ */

void ogles2_GLCompressedTexImage3D(struct OGLES2Context *ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width,
		GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data) {
	if (border) { ogles2_error(ctx, GL_INVALID_VALUE); return; }
	ogles2_compressedTex(QT_OGLES2_CT_Image3D, target, level, internalformat, 0, 0, width, height, depth, 0, imageSize, data);
}
void ogles2_GLCompressedTexSubImage3D(struct OGLES2Context *ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset,
		GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data) {
	ogles2_compressedTex(QT_OGLES2_CT_Sub3D, target, level, (ULONG) xoffset, yoffset, zoffset, width, height, depth, format,
		imageSize, data);
}
void ogles2_GLCompressedTexImage3DOES(struct OGLES2Context *ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width,
		GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data) {
	ogles2_GLCompressedTexImage3D(ctx, target, level, internalformat, width, height, depth, border, imageSize, data);
}
void ogles2_GLCompressedTexSubImage3DOES(struct OGLES2Context *ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset,
		GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data) {
	ogles2_GLCompressedTexSubImage3D(ctx, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

/* --- Program binaries: none offered (as glGetProgramBinaryOES) ------------ */

void ogles2_GLGetProgramBinary(struct OGLES2Context *ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat,
		void *binary) {
	ogles2_GLGetProgramBinaryOES(ctx, program, bufSize, length, binaryFormat, binary);
}
void ogles2_GLProgramBinary(struct OGLES2Context *ctx, GLuint program, GLenum binaryFormat, const void *binary, GLsizei length) {
	ogles2_error(ctx, GL_INVALID_ENUM);
}

/* --- GL_KHR_debug ----------------------------------------------------------
 * The application's own messages (glDebugMessageInsertKHR, pushing and
 * popping debug groups) go to its callback, as the extension says; the
 * host driver's are not passed on (a host thread cannot call 68k code), so
 * the message log stays empty. Object labels are not kept: they read back
 * empty. The extension is not in GL_EXTENSIONS. */

typedef void (*DebugProc)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
	const void *userParam);

#define MAX_DEBUG_DEPTH 64     /* GL_MAX_DEBUG_GROUP_STACK_DEPTH */

static void debugMessage(struct OGLES2Context *ctx, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
		const GLchar *text) {
	if (!ctx || !ctx->debugCallback || !text) return;
	if (!ogles2_GLIsEnabled(ctx, 0x92E0)) return; /* GL_DEBUG_OUTPUT */
	if (length < 0) length = (GLsizei) strlen(text);
	((DebugProc) ctx->debugCallback)(source, type, id, severity, length, text, ctx->debugUserParam);
}

void ogles2_GLDebugMessageControlKHR(struct OGLES2Context *ctx, GLenum source, GLenum type, GLenum severity, GLsizei count,
		const GLuint *ids, GLboolean enabled) {
	if (count < 0) ogles2_error(ctx, GL_INVALID_VALUE);
}
void ogles2_GLDebugMessageInsertKHR(struct OGLES2Context *ctx, GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar *buf) {
	/* Only GL_DEBUG_SOURCE_APPLICATION and _THIRD_PARTY may be inserted. */
	if (source != 0x824A && source != 0x8249) { ogles2_error(ctx, GL_INVALID_ENUM); return; }
	debugMessage(ctx, source, type, id, severity, length, buf);
}
void ogles2_GLDebugMessageCallbackKHR(struct OGLES2Context *ctx, GLDEBUGPROCKHR callback, const void *userParam) {
	if (!ctx) return;
	ctx->debugCallback = (void *) callback;
	ctx->debugUserParam = userParam;
}
GLuint ogles2_GLGetDebugMessageLogKHR(struct OGLES2Context *ctx, GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types,
		GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog) {
	return 0;
}
void ogles2_GLPushDebugGroupKHR(struct OGLES2Context *ctx, GLenum source, GLuint id, GLsizei length, const GLchar *message) {
	if (!ctx) return;
	if (source != 0x824A && source != 0x8249) { ogles2_error(ctx, GL_INVALID_ENUM); return; }
	if (ctx->debugDepth + 1 >= MAX_DEBUG_DEPTH) { ogles2_error(ctx, 0x0503); return; } /* GL_STACK_OVERFLOW */
	++ctx->debugDepth;
	debugMessage(ctx, source, 0x8269, id, 0x826B, length, message); /* GL_DEBUG_TYPE_PUSH_GROUP, _SEVERITY_NOTIFICATION */
}
void ogles2_GLPopDebugGroupKHR(struct OGLES2Context *ctx) {
	if (!ctx) return;
	if (!ctx->debugDepth) { ogles2_error(ctx, 0x0504); return; } /* GL_STACK_UNDERFLOW */
	--ctx->debugDepth;
	debugMessage(ctx, 0x824A, 0x826A, 0, 0x826B, -1, "pop"); /* GL_DEBUG_TYPE_POP_GROUP */
}
void ogles2_GLObjectLabelKHR(struct OGLES2Context *ctx, GLenum identifier, GLuint name, GLsizei length, const GLchar *label) {}
void ogles2_GLGetObjectLabelKHR(struct OGLES2Context *ctx, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length,
		GLchar *label) {
	if (length) *length = 0;
	if (label && bufSize > 0) label[0] = 0;
}
void ogles2_GLObjectPtrLabelKHR(struct OGLES2Context *ctx, const void *ptr, GLsizei length, const GLchar *label) {}
void ogles2_GLGetObjectPtrLabelKHR(struct OGLES2Context *ctx, const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label) {
	ogles2_GLGetObjectLabelKHR(ctx, 0, 0, bufSize, length, label);
}
void ogles2_GLGetPointervKHR(struct OGLES2Context *ctx, GLenum pname, void **params) {
	if (!params) return;
	switch (pname) {
	case 0x8244: *params = ctx ? ctx->debugCallback : NULL; return;                /* GL_DEBUG_CALLBACK_FUNCTION */
	case 0x8245: *params = ctx ? (void *) ctx->debugUserParam : NULL; return;      /* GL_DEBUG_CALLBACK_USER_PARAM */
	}
	*params = NULL;
	ogles2_error(ctx, GL_INVALID_ENUM);
}

/* GL_KHR_debug's glGet values (glfuncs.c's localValue asks). */
int ogles2_debugValue(struct OGLES2Context *ctx, GLenum pname, GLint *value) {
	switch (pname) {
	case 0x826C: *value = MAX_DEBUG_DEPTH; return 1;                       /* GL_MAX_DEBUG_GROUP_STACK_DEPTH */
	case 0x826D: *value = (ctx ? ctx->debugDepth : 0) + 1; return 1;       /* GL_DEBUG_GROUP_STACK_DEPTH */
	case 0x82E8: *value = 256; return 1;                                   /* GL_MAX_LABEL_LENGTH */
	case 0x9143: *value = 256; return 1;                                   /* GL_MAX_DEBUG_MESSAGE_LENGTH */
	case 0x9144: *value = 1; return 1;                                     /* GL_MAX_DEBUG_LOGGED_MESSAGES */
	case 0x9145: case 0x8243: *value = 0; return 1;                        /* GL_DEBUG_LOGGED_MESSAGES, _NEXT_LOGGED_MESSAGE_LENGTH */
	}
	return 0;
}

/* --- GL_OES_texture_3D's attachment: a layer of the 3D texture ------------ */

void ogles2_GLFramebufferTexture3DOES(struct OGLES2Context *ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
		GLint level, GLint zoffset) {
	if (texture && textarget != 0x806F) { ogles2_error(ctx, GL_INVALID_ENUM); return; } /* GL_TEXTURE_3D */
	ogles2_GLFramebufferTextureLayer(ctx, target, attachment, texture, level, zoffset);
}

/* --- Several draws in one call, and base vertices -------------------------
 * QT_OGLES2_DRAW's "first" is the base vertex of an indexed draw. */

void ogles2_GLMultiDrawArraysEXT(struct OGLES2Context *ctx, GLenum mode, const GLint *first, const GLsizei *count,
		GLsizei primcount) {
	GLsizei i;
	if (primcount < 0) { ogles2_error(ctx, GL_INVALID_VALUE); return; }
	if (!first || !count) return;
	for (i = 0; i < primcount; ++i) if (count[i] > 0) ogles2_draw(ctx, mode, first[i], count[i], 0, 0, 0);
}
void ogles2_GLMultiDrawElementsEXT(struct OGLES2Context *ctx, GLenum mode, const GLsizei *count, GLenum type,
		const void *const *indices, GLsizei primcount) {
	GLsizei i;
	if (primcount < 0) { ogles2_error(ctx, GL_INVALID_VALUE); return; }
	if (!count || !indices) return;
	for (i = 0; i < primcount; ++i) if (count[i] > 0) ogles2_draw(ctx, mode, 0, count[i], type, indices[i], 0);
}
void ogles2_GLMultiDrawElementsBaseVertexEXT(struct OGLES2Context *ctx, GLenum mode, const GLsizei *count, GLenum type,
		const void *const *indices, GLsizei drawcount, const GLint *basevertex) {
	GLsizei i;
	if (drawcount < 0) { ogles2_error(ctx, GL_INVALID_VALUE); return; }
	if (!count || !indices || !basevertex) return;
	for (i = 0; i < drawcount; ++i) if (count[i] > 0) ogles2_draw(ctx, mode, basevertex[i], count[i], type, indices[i], 0);
}

void ogles2_GLDrawElementsBaseVertexEXT(struct OGLES2Context *ctx, GLenum mode, GLsizei count, GLenum type, const void *indices,
		GLint basevertex) {
	ogles2_draw(ctx, mode, basevertex, count, type, indices, 0);
}
/* start/end are only a hint, as for glDrawRangeElements. */
void ogles2_GLDrawRangeElementsBaseVertexOES(struct OGLES2Context *ctx, GLenum mode, GLuint start, GLuint end, GLsizei count,
		GLenum type, const void *indices, GLint basevertex) {
	if (end < start) { ogles2_error(ctx, GL_INVALID_VALUE); return; }
	ogles2_draw(ctx, mode, basevertex, count, type, indices, 0);
}
void ogles2_GLDrawRangeElementsBaseVertexEXT(struct OGLES2Context *ctx, GLenum mode, GLuint start, GLuint end, GLsizei count,
		GLenum type, const void *indices, GLint basevertex) {
	ogles2_GLDrawRangeElementsBaseVertexOES(ctx, mode, start, end, count, type, indices, basevertex);
}
void ogles2_GLDrawElementsInstancedBaseVertexOES(struct OGLES2Context *ctx, GLenum mode, GLsizei count, GLenum type,
		const void *indices, GLsizei instancecount, GLint basevertex) {
	if (instancecount > 0) ogles2_draw(ctx, mode, basevertex, count, type, indices, instancecount);
}
void ogles2_GLDrawElementsInstancedBaseVertexEXT(struct OGLES2Context *ctx, GLenum mode, GLsizei count, GLenum type,
		const void *indices, GLsizei instancecount, GLint basevertex) {
	ogles2_GLDrawElementsInstancedBaseVertexOES(ctx, mode, count, type, indices, instancecount, basevertex);
}

/* --- Primitive bounding box: a hint for tessellation, which OpenGL ES 3.1
 * has not; nothing to do. ------------------------------------------------- */

void ogles2_GLPrimitiveBoundingBoxOES(struct OGLES2Context *ctx, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
		GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW) {}
void ogles2_GLPrimitiveBoundingBoxEXT(struct OGLES2Context *ctx, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
		GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW) {}
