#include "ogles2.h"
#include "gldeclarations.auto.h"
#include <exec/memory.h>
#include <proto/exec.h>

/*
 * OpenGL ES 3.0's entries (docs/phase10-ogles3.md, stage 2), on top of
 * ogles2.library's (glfuncs.c, whose command helpers these use).
 *
 * Vertex array objects live here, on the 68k side, as the attributes they
 * hold do: glBindVertexArray copies the live attribute state out to the
 * object it leaves and the new one's in. Samplers and queries are real host
 * objects (QT_OGLES2_GEN's kinds); uniform buffers are the fake buffers,
 * bound by name (QT_OGLES2_BIND_BUFFER_RANGE). Sync objects are emulated:
 * the host runs every command buffer to its end before the 68k goes on, so
 * a fence is signalled once glFinish has returned.
 */

/* glfuncs.c's entries these use. */
void ogles2_GLBindBuffer(struct OGLES2Context *ctx, GLenum target, GLuint buffer);
void ogles2_GLDrawBuffersEXT(struct OGLES2Context *ctx, GLsizei n, const GLenum *bufs);
const GLubyte *ogles2_GLGetString(struct OGLES2Context *ctx, GLenum name);
void ogles2_GLGetIntegerv(struct OGLES2Context *ctx, GLenum pname, GLint *data);
void ogles2_GLFinish(struct OGLES2Context *ctx);

/* --- Vertex array objects ----------------------------------------------- */

static QtOgles2Vao *findVao(struct OGLES2Context *ctx, GLuint name) {
	QtOgles2Vao *v;
	for (v = ctx->vaos; v; v = v->next) if (v->name == name) return v;
	return NULL;
}

void ogles2_GLGenVertexArrays(struct OGLES2Context *ctx, GLsizei n, GLuint *arrays) {
	GLsizei i;
	if (!ctx || !arrays) return;
	for (i = 0; i < n; ++i) {
		QtOgles2Vao *v = (QtOgles2Vao *) AllocVec(sizeof(QtOgles2Vao), MEMF_ANY | MEMF_CLEAR);
		arrays[i] = 0;
		if (!v) continue;
		ogles2_resetAttribs(v->attribs);
		v->name = ++ctx->nextVao;
		v->next = ctx->vaos;
		ctx->vaos = v;
		arrays[i] = v->name;
	}
}

/* The live state into the bound object, then the object's into the live
 * state (NULL: the default one). */
static void switchVao(struct OGLES2Context *ctx, QtOgles2Vao *to) {
	QtOgles2Vao *from = ctx->vao ? ctx->vao : &ctx->defaultVao;
	CopyMem(ctx->attribs, from->attribs, sizeof(ctx->attribs));
	CopyMem(ctx->bindings, from->bindings, sizeof(ctx->bindings));
	from->elementArrayBuffer = ctx->elementArrayBuffer;
	ctx->vao = to;
	to = to ? to : &ctx->defaultVao;
	CopyMem(to->attribs, ctx->attribs, sizeof(ctx->attribs));
	CopyMem(to->bindings, ctx->bindings, sizeof(ctx->bindings));
	ctx->elementArrayBuffer = to->elementArrayBuffer;
}

void ogles2_GLBindVertexArray(struct OGLES2Context *ctx, GLuint array) {
	QtOgles2Vao *v = NULL;
	if (!ctx) return;
	if (array && !(v = findVao(ctx, array))) return; /* GL_INVALID_OPERATION */
	if (v != ctx->vao) switchVao(ctx, v);
}

void ogles2_GLDeleteVertexArrays(struct OGLES2Context *ctx, GLsizei n, const GLuint *arrays) {
	GLsizei i;
	if (!ctx || !arrays) return;
	for (i = 0; i < n; ++i) {
		QtOgles2Vao **link, *v;
		if (!arrays[i]) continue;
		for (link = &ctx->vaos; (v = *link) && v->name != arrays[i]; link = &v->next);
		if (!v) continue;
		if (ctx->vao == v) switchVao(ctx, NULL);
		*link = v->next;
		FreeVec(v);
	}
}

GLboolean ogles2_GLIsVertexArray(struct OGLES2Context *ctx, GLuint array) {
	return ctx && array && findVao(ctx, array) ? GL_TRUE : GL_FALSE;
}

void ogles2_GLGenVertexArraysOES(struct OGLES2Context *ctx, GLsizei n, GLuint *arrays) { ogles2_GLGenVertexArrays(ctx, n, arrays); }
void ogles2_GLBindVertexArrayOES(struct OGLES2Context *ctx, GLuint array) { ogles2_GLBindVertexArray(ctx, array); }
void ogles2_GLDeleteVertexArraysOES(struct OGLES2Context *ctx, GLsizei n, const GLuint *arrays) {
	ogles2_GLDeleteVertexArrays(ctx, n, arrays);
}
GLboolean ogles2_GLIsVertexArrayOES(struct OGLES2Context *ctx, GLuint array) { return ogles2_GLIsVertexArray(ctx, array); }

/* --- Attributes and instanced drawing ----------------------------------- */

void ogles2_vertexAttrib(struct OGLES2Context *ctx, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
	const void *pointer, GLboolean integer);

void ogles2_GLVertexAttribIPointer(struct OGLES2Context *ctx, GLuint index, GLint size, GLenum type, GLsizei stride,
		const void *pointer) {
	ogles2_vertexAttrib(ctx, index, size, type, GL_FALSE, stride, pointer, GL_TRUE);
}

/* OpenGL ES 3.1's definition: the attribute reads binding index, whose
 * divisor this is. */
void ogles2_GLVertexAttribDivisor(struct OGLES2Context *ctx, GLuint index, GLuint divisor) {
	if (!ctx || index >= QT_OGLES2_MAX_ATTRIBS) return;
	ctx->attribs[index].binding = index;
	ctx->bindings[index].divisor = divisor;
	ogles2_bindingChanged(ctx, index);
}
void ogles2_GLVertexAttribDivisorEXT(struct OGLES2Context *ctx, GLuint index, GLuint divisor) {
	ogles2_GLVertexAttribDivisor(ctx, index, divisor);
}

void ogles2_GLDrawArraysInstanced(struct OGLES2Context *ctx, GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
	if (instancecount > 0) ogles2_draw(ctx, mode, first, count, 0, 0, instancecount);
}
void ogles2_GLDrawElementsInstanced(struct OGLES2Context *ctx, GLenum mode, GLsizei count, GLenum type, const void *indices,
		GLsizei instancecount) {
	if (instancecount > 0) ogles2_draw(ctx, mode, 0, count, type, indices, instancecount);
}
void ogles2_GLDrawArraysInstancedEXT(struct OGLES2Context *ctx, GLenum mode, GLint start, GLsizei count, GLsizei primcount) {
	ogles2_GLDrawArraysInstanced(ctx, mode, start, count, primcount);
}
void ogles2_GLDrawElementsInstancedEXT(struct OGLES2Context *ctx, GLenum mode, GLsizei count, GLenum type, const void *indices,
		GLsizei primcount) {
	ogles2_GLDrawElementsInstanced(ctx, mode, count, type, indices, primcount);
}

void ogles2_GLVertexAttribI4i(struct OGLES2Context *ctx, GLuint index, GLint x, GLint y, GLint z, GLint w) {
	ogles2_call(QT_OGLES2_C_VertexAttribI4i, index, (ULONG) x, (ULONG) y, (ULONG) z, (ULONG) w);
}
void ogles2_GLVertexAttribI4ui(struct OGLES2Context *ctx, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
	ogles2_call(QT_OGLES2_C_VertexAttribI4ui, index, x, y, z, w);
}
void ogles2_GLVertexAttribI4iv(struct OGLES2Context *ctx, GLuint index, const GLint *v) {
	if (v) ogles2_GLVertexAttribI4i(ctx, index, v[0], v[1], v[2], v[3]);
}
void ogles2_GLVertexAttribI4uiv(struct OGLES2Context *ctx, GLuint index, const GLuint *v) {
	if (v) ogles2_GLVertexAttribI4ui(ctx, index, v[0], v[1], v[2], v[3]);
}

/* --- Uniforms and uniform buffers -------------------------------------- */

void ogles2_GLUniform1ui(struct OGLES2Context *ctx, GLint location, GLuint v0) {
	ogles2_uniform(QT_OGLES2_U_1UI, location, v0, 0, 0, 0);
}
void ogles2_GLUniform2ui(struct OGLES2Context *ctx, GLint location, GLuint v0, GLuint v1) {
	ogles2_uniform(QT_OGLES2_U_2UI, location, v0, v1, 0, 0);
}
void ogles2_GLUniform3ui(struct OGLES2Context *ctx, GLint location, GLuint v0, GLuint v1, GLuint v2) {
	ogles2_uniform(QT_OGLES2_U_3UI, location, v0, v1, v2, 0);
}
void ogles2_GLUniform4ui(struct OGLES2Context *ctx, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
	ogles2_uniform(QT_OGLES2_U_4UI, location, v0, v1, v2, v3);
}
void ogles2_GLUniform1uiv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLuint *value) {
	ogles2_uniformV(QT_OGLES2_U_1UI, location, count, value, GL_FALSE);
}
void ogles2_GLUniform2uiv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLuint *value) {
	ogles2_uniformV(QT_OGLES2_U_2UI, location, count, value, GL_FALSE);
}
void ogles2_GLUniform3uiv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLuint *value) {
	ogles2_uniformV(QT_OGLES2_U_3UI, location, count, value, GL_FALSE);
}
void ogles2_GLUniform4uiv(struct OGLES2Context *ctx, GLint location, GLsizei count, const GLuint *value) {
	ogles2_uniformV(QT_OGLES2_U_4UI, location, count, value, GL_FALSE);
}
void ogles2_GLUniformMatrix2x3fv(struct OGLES2Context *ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	ogles2_uniformV(QT_OGLES2_U_M2x3, location, count, value, transpose);
}
void ogles2_GLUniformMatrix3x2fv(struct OGLES2Context *ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	ogles2_uniformV(QT_OGLES2_U_M3x2, location, count, value, transpose);
}
void ogles2_GLUniformMatrix2x4fv(struct OGLES2Context *ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	ogles2_uniformV(QT_OGLES2_U_M2x4, location, count, value, transpose);
}
void ogles2_GLUniformMatrix4x2fv(struct OGLES2Context *ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	ogles2_uniformV(QT_OGLES2_U_M4x2, location, count, value, transpose);
}
void ogles2_GLUniformMatrix3x4fv(struct OGLES2Context *ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	ogles2_uniformV(QT_OGLES2_U_M3x4, location, count, value, transpose);
}
void ogles2_GLUniformMatrix4x3fv(struct OGLES2Context *ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	ogles2_uniformV(QT_OGLES2_U_M4x3, location, count, value, transpose);
}
/* An unsigned uniform's words are read back as the signed ones are. */
void ogles2_GLGetUniformuiv(struct OGLES2Context *ctx, GLuint program, GLint location, GLuint *params) {
	ogles2_getParam(QT_OGLES2_P_Uniformiv, program, (ULONG) location, 0, params);
}

GLint ogles2_GLGetFragDataLocation(struct OGLES2Context *ctx, GLuint program, const GLchar *name) {
	return (GLint) ogles2_nameCall(QT_OGLES2_N_GetFragDataLocation, program, 0, name);
}
GLuint ogles2_GLGetUniformBlockIndex(struct OGLES2Context *ctx, GLuint program, const GLchar *uniformBlockName) {
	return (GLuint) ogles2_nameCall(QT_OGLES2_N_GetUniformBlockIndex, program, 0, uniformBlockName);
}
void ogles2_GLUniformBlockBinding(struct OGLES2Context *ctx, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {
	ogles2_call(QT_OGLES2_C_UniformBlockBinding, program, uniformBlockIndex, uniformBlockBinding, 0, 0);
}

/* glBindBufferBase/Range bind the generic target too, as OpenGL ES does;
 * the host keeps the uniform and transform feedback buffers bound by
 * index. */
void ogles2_GLBindBufferRange(struct OGLES2Context *ctx, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
	ULONG *w;
	if (!ctx) return;
	ogles2_GLBindBuffer(ctx, target, buffer);
	/* GL_UNIFORM_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER, GL_SHADER_STORAGE_BUFFER, GL_ATOMIC_COUNTER_BUFFER */
	if (target != 0x8A11 && target != 0x8C8E && target != 0x90D2 && target != 0x92C0) return;
	w = qt_reserve(QT_OGLES2_BIND_BUFFER_RANGE_WORDS);
	w[0] = ((ULONG) QT_OGLES2_BIND_BUFFER_RANGE << 16) | QT_OGLES2_BIND_BUFFER_RANGE_WORDS;
	w[1] = target;
	w[2] = index;
	w[3] = buffer;
	w[4] = (ULONG) offset;
	w[5] = (ULONG) size;
	qt_flush();
}
void ogles2_GLBindBufferBase(struct OGLES2Context *ctx, GLenum target, GLuint index, GLuint buffer) {
	ogles2_GLBindBufferRange(ctx, target, index, buffer, 0, 0);
}

void ogles2_GLCopyBufferSubData(struct OGLES2Context *ctx, GLenum readTarget, GLenum writeTarget, GLintptr readOffset,
		GLintptr writeOffset, GLsizeiptr size) {
	ULONG *w = qt_reserve(QT_OGLES2_COPY_BUFFER_SUB_DATA_WORDS);
	w[0] = ((ULONG) QT_OGLES2_COPY_BUFFER_SUB_DATA << 16) | QT_OGLES2_COPY_BUFFER_SUB_DATA_WORDS;
	w[1] = ogles2_boundBuffer(ctx, readTarget);
	w[2] = ogles2_boundBuffer(ctx, writeTarget);
	w[3] = (ULONG) readOffset;
	w[4] = (ULONG) writeOffset;
	w[5] = (ULONG) size;
	qt_flush();
}

/* glMapBufferRange: as glMapBufferOES (glfuncs.c), a 68k-side copy of the
 * range, written back by glUnmapBuffer (all of it, or with GL_MAP_FLUSH_
 * EXPLICIT_BIT only what glFlushMappedBufferRange named), for any buffer
 * target. */
#define GL_MAP_READ_BIT_ 0x0001
#define GL_MAP_WRITE_BIT_ 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT_ 0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT_ 0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT_ 0x0010

/* 0 and 1 as glfuncs.c's bufferSlot (array and element buffers), the rest
 * of OpenGL ES 3.0's targets after them. */
static int bufferSlot3(GLenum target) {
	switch (target) {
	case GL_ARRAY_BUFFER: return 0;
	case GL_ELEMENT_ARRAY_BUFFER: return 1;
	case 0x8A11: return 2; /* GL_UNIFORM_BUFFER */
	case 0x8F36: return 3; /* GL_COPY_READ_BUFFER */
	case 0x8F37: return 4; /* GL_COPY_WRITE_BUFFER */
	case 0x88EB: return 5; /* GL_PIXEL_PACK_BUFFER */
	case 0x88EC: return 6; /* GL_PIXEL_UNPACK_BUFFER */
	case 0x8C8E: return 7; /* GL_TRANSFORM_FEEDBACK_BUFFER */
	case 0x90D2: return 8; /* GL_SHADER_STORAGE_BUFFER */
	case 0x92C0: return 9; /* GL_ATOMIC_COUNTER_BUFFER */
	case 0x90EE: return 10; /* GL_DISPATCH_INDIRECT_BUFFER */
	case 0x8F3F: return 11; /* GL_DRAW_INDIRECT_BUFFER */
	}
	return -1;
}

void *ogles2_GLMapBufferRange(struct OGLES2Context *ctx, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
	int slot = bufferSlot3(target);
	GLuint buffer = ogles2_boundBuffer(ctx, target);
	if (slot < 0 || !buffer || ctx->mapped[slot] || length <= 0) return NULL;
	ctx->mapped[slot] = AllocVec((ULONG) length, MEMF_ANY | MEMF_CLEAR);
	if (!ctx->mapped[slot]) return NULL;
	ctx->mappedSize[slot] = (ULONG) length;
	ctx->mappedOffset[slot] = (ULONG) offset;
	ctx->mappedAccess[slot] = access;
	if (!(access & (GL_MAP_INVALIDATE_RANGE_BIT_ | GL_MAP_INVALIDATE_BUFFER_BIT_)))
		ogles2_bufferCopy(QT_OGLES2_READ_BUFFER, buffer, (ULONG) offset, (ULONG) length, ctx->mapped[slot]);
	return ctx->mapped[slot];
}

void ogles2_GLFlushMappedBufferRange(struct OGLES2Context *ctx, GLenum target, GLintptr offset, GLsizeiptr length) {
	int slot = bufferSlot3(target);
	if (!ctx || slot < 0 || !ctx->mapped[slot] || offset < 0 || offset + length > (GLintptr) ctx->mappedSize[slot]) return;
	ogles2_bufferCopy(QT_OGLES2_BUFFER_SUB_DATA, ogles2_boundBuffer(ctx, target), ctx->mappedOffset[slot] + (ULONG) offset,
		(ULONG) length, (UBYTE *) ctx->mapped[slot] + offset);
}

GLboolean ogles2_GLUnmapBuffer(struct OGLES2Context *ctx, GLenum target) {
	int slot = bufferSlot3(target);
	if (!ctx || slot < 0 || !ctx->mapped[slot]) return GL_FALSE;
	if ((ctx->mappedAccess[slot] & GL_MAP_WRITE_BIT_) && !(ctx->mappedAccess[slot] & GL_MAP_FLUSH_EXPLICIT_BIT_))
		ogles2_bufferCopy(QT_OGLES2_BUFFER_SUB_DATA, ogles2_boundBuffer(ctx, target), ctx->mappedOffset[slot], ctx->mappedSize[slot],
			ctx->mapped[slot]);
	FreeVec(ctx->mapped[slot]);
	ctx->mapped[slot] = NULL;
	ctx->mappedOffset[slot] = 0;
	return GL_TRUE;
}

void *ogles2_GLMapBufferRangeEXT(struct OGLES2Context *ctx, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
	return ogles2_GLMapBufferRange(ctx, target, offset, length, access);
}
void ogles2_GLFlushMappedBufferRangeEXT(struct OGLES2Context *ctx, GLenum target, GLintptr offset, GLsizeiptr length) {
	ogles2_GLFlushMappedBufferRange(ctx, target, offset, length);
}
void ogles2_GLGetBufferPointerv(struct OGLES2Context *ctx, GLenum target, GLenum pname, void **params) {
	int slot = bufferSlot3(target);
	if (params) *params = ctx && slot >= 0 && pname == 0x88BD ? ctx->mapped[slot] : NULL; /* GL_BUFFER_MAP_POINTER */
}

/* --- Samplers ------------------------------------------------------------ */

void ogles2_GLGenSamplers(struct OGLES2Context *ctx, GLsizei count, GLuint *samplers) {
	ogles2_genObjects(QT_OGLES2_K_Sampler, count, samplers);
}
void ogles2_GLDeleteSamplers(struct OGLES2Context *ctx, GLsizei count, const GLuint *samplers) {
	ogles2_deleteObjects(QT_OGLES2_K_Sampler, count, samplers);
}
GLboolean ogles2_GLIsSampler(struct OGLES2Context *ctx, GLuint sampler) {
	return (GLboolean) ogles2_call(QT_OGLES2_C_IsSampler, sampler, 0, 0, 0, 0);
}
void ogles2_GLBindSampler(struct OGLES2Context *ctx, GLuint unit, GLuint sampler) {
	ogles2_call(QT_OGLES2_C_BindSampler, unit, sampler, 0, 0, 0);
}
void ogles2_GLSamplerParameteri(struct OGLES2Context *ctx, GLuint sampler, GLenum pname, GLint param) {
	ogles2_call(QT_OGLES2_C_SamplerParameteri, sampler, pname, (ULONG) param, 0, 0);
}
void ogles2_GLSamplerParameterf(struct OGLES2Context *ctx, GLuint sampler, GLenum pname, GLfloat param) {
	ogles2_call(QT_OGLES2_C_SamplerParameterf, sampler, pname, ogles2_floatWord(param), 0, 0);
}
/* OpenGL ES 3.0's sampler parameters all have one value. */
void ogles2_GLSamplerParameteriv(struct OGLES2Context *ctx, GLuint sampler, GLenum pname, const GLint *param) {
	if (param) ogles2_GLSamplerParameteri(ctx, sampler, pname, param[0]);
}
void ogles2_GLSamplerParameterfv(struct OGLES2Context *ctx, GLuint sampler, GLenum pname, const GLfloat *param) {
	if (param) ogles2_GLSamplerParameterf(ctx, sampler, pname, param[0]);
}

/* --- Textures ------------------------------------------------------------ */

void ogles2_GLTexImage3D(struct OGLES2Context *ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
		GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels) {
	ULONG *w = qt_reserve(QT_OGLES2_TEX_IMAGE_3D_WORDS);
	w[0] = ((ULONG) QT_OGLES2_TEX_IMAGE_3D << 16) | QT_OGLES2_TEX_IMAGE_3D_WORDS;
	w[1] = target;
	w[2] = (ULONG) level;
	w[3] = (ULONG) internalformat;
	w[4] = (ULONG) width;
	w[5] = (ULONG) height;
	w[6] = (ULONG) depth;
	w[7] = (ULONG) border;
	w[8] = format;
	w[9] = type;
	w[10] = (ULONG) pixels;
	qt_flush();
}

void ogles2_GLTexSubImage3D(struct OGLES2Context *ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) {
	ULONG *w = qt_reserve(QT_OGLES2_TEX_SUB_IMAGE_3D_WORDS);
	w[0] = ((ULONG) QT_OGLES2_TEX_SUB_IMAGE_3D << 16) | QT_OGLES2_TEX_SUB_IMAGE_3D_WORDS;
	w[1] = target;
	w[2] = (ULONG) level;
	w[3] = (ULONG) xoffset;
	w[4] = (ULONG) yoffset;
	w[5] = (ULONG) zoffset;
	w[6] = (ULONG) width;
	w[7] = (ULONG) height;
	w[8] = (ULONG) depth;
	w[9] = format;
	w[10] = type;
	w[11] = (ULONG) pixels;
	qt_flush();
}

void ogles2_GLCopyTexSubImage3D(struct OGLES2Context *ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLint x, GLint y, GLsizei width, GLsizei height) {
	ULONG words[10] = {0};
	words[0] = target;
	words[1] = (ULONG) level;
	words[2] = (ULONG) xoffset;
	words[3] = (ULONG) yoffset;
	words[4] = (ULONG) zoffset;
	words[5] = (ULONG) x;
	words[6] = (ULONG) y;
	words[7] = (ULONG) width;
	words[8] = (ULONG) height;
	ogles2_call10(QT_OGLES2_C_CopyTexSubImage3D, words);
}

void ogles2_GLTexStorage2D(struct OGLES2Context *ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
		GLsizei height) {
	ogles2_call(QT_OGLES2_C_TexStorage2D, target, (ULONG) levels, internalformat, (ULONG) width, (ULONG) height);
}

void ogles2_GLTexStorage3D(struct OGLES2Context *ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
		GLsizei height, GLsizei depth) {
	ULONG words[10] = {0};
	words[0] = target;
	words[1] = (ULONG) levels;
	words[2] = internalformat;
	words[3] = (ULONG) width;
	words[4] = (ULONG) height;
	words[5] = (ULONG) depth;
	ogles2_call10(QT_OGLES2_C_TexStorage3D, words);
}

void ogles2_GLTexImage3DOES(struct OGLES2Context *ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width,
		GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels) {
	ogles2_GLTexImage3D(ctx, target, level, (GLint) internalformat, width, height, depth, border, format, type, pixels);
}
void ogles2_GLTexSubImage3DOES(struct OGLES2Context *ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) {
	ogles2_GLTexSubImage3D(ctx, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}
void ogles2_GLCopyTexSubImage3DOES(struct OGLES2Context *ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset,
		GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
	ogles2_GLCopyTexSubImage3D(ctx, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}
void ogles2_GLTexStorage2DEXT(struct OGLES2Context *ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
		GLsizei height) {
	ogles2_GLTexStorage2D(ctx, target, levels, internalformat, width, height);
}
void ogles2_GLTexStorage3DEXT(struct OGLES2Context *ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
		GLsizei height, GLsizei depth) {
	ogles2_GLTexStorage3D(ctx, target, levels, internalformat, width, height, depth);
}

/* --- Framebuffers --------------------------------------------------------- */

void ogles2_GLDrawBuffers(struct OGLES2Context *ctx, GLsizei n, const GLenum *bufs) {
	ogles2_GLDrawBuffersEXT(ctx, n, bufs);
}
void ogles2_GLReadBuffer(struct OGLES2Context *ctx, GLenum src) {
	_glReadBuffer(src);
}

/* Only hints, which the host's renderer has no use for. */
void ogles2_GLInvalidateFramebuffer(struct OGLES2Context *ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments) {}
void ogles2_GLInvalidateSubFramebuffer(struct OGLES2Context *ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments,
		GLint x, GLint y, GLsizei width, GLsizei height) {}

void ogles2_GLBlitFramebuffer(struct OGLES2Context *ctx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0,
		GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
	ULONG words[10];
	words[0] = (ULONG) srcX0;
	words[1] = (ULONG) srcY0;
	words[2] = (ULONG) srcX1;
	words[3] = (ULONG) srcY1;
	words[4] = (ULONG) dstX0;
	words[5] = (ULONG) dstY0;
	words[6] = (ULONG) dstX1;
	words[7] = (ULONG) dstY1;
	words[8] = mask;
	words[9] = filter;
	ogles2_call10(QT_OGLES2_C_BlitFramebuffer, words);
}

void ogles2_GLRenderbufferStorageMultisample(struct OGLES2Context *ctx, GLenum target, GLsizei samples, GLenum internalformat,
		GLsizei width, GLsizei height) {
	ogles2_call(QT_OGLES2_C_RenderbufferStorageMultisample, target, (ULONG) samples, internalformat, (ULONG) width, (ULONG) height);
}

void ogles2_GLFramebufferTextureLayer(struct OGLES2Context *ctx, GLenum target, GLenum attachment, GLuint texture, GLint level,
		GLint layer) {
	ogles2_call(QT_OGLES2_C_FramebufferTextureLayer, target, attachment, texture, (ULONG) level, (ULONG) layer);
}

static void clearBuffer(ULONG subop, GLenum buffer, GLint drawbuffer, ULONG w4, ULONG w5) {
	ULONG *w = qt_reserve(QT_OGLES2_CLEAR_BUFFER_WORDS);
	w[0] = ((ULONG) QT_OGLES2_CLEAR_BUFFER << 16) | QT_OGLES2_CLEAR_BUFFER_WORDS;
	w[1] = subop;
	w[2] = buffer;
	w[3] = (ULONG) drawbuffer;
	w[4] = w4;
	w[5] = w5;
	qt_flush();
}
void ogles2_GLClearBufferiv(struct OGLES2Context *ctx, GLenum buffer, GLint drawbuffer, const GLint *value) {
	clearBuffer(QT_OGLES2_CB_iv, buffer, drawbuffer, (ULONG) value, 0);
}
void ogles2_GLClearBufferuiv(struct OGLES2Context *ctx, GLenum buffer, GLint drawbuffer, const GLuint *value) {
	clearBuffer(QT_OGLES2_CB_uiv, buffer, drawbuffer, (ULONG) value, 0);
}
void ogles2_GLClearBufferfv(struct OGLES2Context *ctx, GLenum buffer, GLint drawbuffer, const GLfloat *value) {
	clearBuffer(QT_OGLES2_CB_fv, buffer, drawbuffer, (ULONG) value, 0);
}
void ogles2_GLClearBufferfi(struct OGLES2Context *ctx, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
	clearBuffer(QT_OGLES2_CB_fi, buffer, drawbuffer, ogles2_floatWord(depth), (ULONG) stencil);
}

/* --- Queries of state ----------------------------------------------------- */

/* glGetStringi(GL_EXTENSIONS, i) and GL_NUM_EXTENSIONS: the words of the
 * list glGetString(GL_EXTENSIONS) gives the calling context (ES 2.0 and
 * ES 3.x contexts have different lists), each list split once into words
 * and kept (library data, shared by every program; the lists never change). */
#define QT_MAX_EXTENSIONS 32
typedef struct {
	const char *list;     /* glGetString's list these are the words of; 0 while free */
	int count;
	const char *words[QT_MAX_EXTENSIONS];
	char text[512];
} QtExtensionWords;
static QtExtensionWords extensionLists[2];

static QtExtensionWords *extensionWords(struct OGLES2Context *ctx) {
	const char *all = (const char *) ogles2_GLGetString(ctx, GL_EXTENSIONS);
	QtExtensionWords *e = NULL;
	ULONG i, n = 0;
	char *p;
	for (i = 0; i < sizeof(extensionLists) / sizeof(extensionLists[0]); ++i)
		if (extensionLists[i].list == all) return &extensionLists[i];
	Forbid();
	for (i = 0; i < sizeof(extensionLists) / sizeof(extensionLists[0]) && !e; ++i)
		if (extensionLists[i].list == all || !extensionLists[i].list) e = &extensionLists[i];
	if (!e) e = &extensionLists[0]; /* not reached: two lists, two slots */
	if (e->list != all) {
		e->count = 0;
		for (i = 0; all[i] && n + 1 < sizeof(e->text); ++i) e->text[n++] = all[i];
		e->text[n] = 0;
		p = e->text;
		while (*p && e->count < QT_MAX_EXTENSIONS) {
			while (*p == ' ') *p++ = 0;
			if (!*p) break;
			e->words[e->count++] = p;
			while (*p && *p != ' ') ++p;
		}
		e->list = all;
	}
	Permit();
	return e;
}

int ogles2_extensionCount(struct OGLES2Context *ctx) {
	return extensionWords(ctx)->count;
}

const GLubyte *ogles2_GLGetStringi(struct OGLES2Context *ctx, GLenum name, GLuint index) {
	QtExtensionWords *e;
	if (name != GL_EXTENSIONS) return NULL;
	e = extensionWords(ctx);
	return index < (GLuint) e->count ? (const GLubyte *) e->words[index] : NULL;
}

void ogles2_GLGetInteger64v(struct OGLES2Context *ctx, GLenum pname, GLint64 *data) {
	GLint v[4] = {0, 0, 0, 0};
	int i;
	if (!data) return;
	ogles2_GLGetIntegerv(ctx, pname, v);
	/* Never more than four values (GL_VIEWPORT and the like). */
	for (i = 0; i < 4; ++i) data[i] = v[i];
}

/* --- Sync objects --------------------------------------------------------- */

/* A fence is done when glFinish returns (the host runs each command buffer
 * through before the 68k goes on), so ClientWaitSync waits with that. */
#define QT_SYNC ((GLsync) 0x51594E43) /* "QSYN" */
#define GL_ALREADY_SIGNALED_ 0x911A
#define GL_SIGNALED_ 0x9119
#define GL_SYNC_STATUS_ 0x9114
#define GL_OBJECT_TYPE_ 0x9112
#define GL_SYNC_FENCE_ 0x9116
#define GL_SYNC_CONDITION_ 0x9113
#define GL_SYNC_GPU_COMMANDS_COMPLETE_ 0x9117

GLsync ogles2_GLFenceSync(struct OGLES2Context *ctx, GLenum condition, GLbitfield flags) { return QT_SYNC; }
GLboolean ogles2_GLIsSync(struct OGLES2Context *ctx, GLsync sync) { return sync == QT_SYNC; }
void ogles2_GLDeleteSync(struct OGLES2Context *ctx, GLsync sync) {}
GLenum ogles2_GLClientWaitSync(struct OGLES2Context *ctx, GLsync sync, GLbitfield flags, GLuint64 timeout) {
	ogles2_GLFinish(ctx);
	return GL_ALREADY_SIGNALED_;
}
void ogles2_GLWaitSync(struct OGLES2Context *ctx, GLsync sync, GLbitfield flags, GLuint64 timeout) {}
void ogles2_GLGetSynciv(struct OGLES2Context *ctx, GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values) {
	GLint v = 0;
	switch (pname) {
	case GL_SYNC_STATUS_: v = GL_SIGNALED_; break;
	case GL_OBJECT_TYPE_: v = GL_SYNC_FENCE_; break;
	case GL_SYNC_CONDITION_: v = GL_SYNC_GPU_COMMANDS_COMPLETE_; break;
	}
	if (values && count > 0) values[0] = v;
	if (length) *length = values && count > 0 ? 1 : 0;
}

/* glProgramParameteri: es31b.c (GL_PROGRAM_SEPARABLE). */
