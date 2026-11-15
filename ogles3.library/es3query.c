#include "ogles2.h"
#include <exec/memory.h>
#include <proto/exec.h>

/*
 * The rest of OpenGL ES 3.0 (docs/phase10-ogles3.md, stage 2b): query
 * objects, transform feedback, the uniform block and indexed queries.
 *
 * Transform feedback writes into the host's copies of the buffers bound to
 * GL_TRANSFORM_FEEDBACK_BUFFER; glEndTransformFeedback brings what was
 * captured back into the fake buffers (host/ogles2.cpp), so the 68k reads it
 * with glMapBufferRange as it reads any buffer.
 */

void ogles2_GLBindBufferRange(struct OGLES2Context *ctx, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
void ogles2_GLGetIntegerv(struct OGLES2Context *ctx, GLenum pname, GLint *data);
void ogles2_GLGetBufferParameteriv(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint *params);
void ogles2_GLGetVertexAttribiv(struct OGLES2Context *ctx, GLuint index, GLenum pname, GLint *params);

/* --- Query objects ------------------------------------------------------- */

void ogles2_GLGenQueries(struct OGLES2Context *ctx, GLsizei n, GLuint *ids) { ogles2_genObjects(QT_OGLES2_K_Query, n, ids); }
void ogles2_GLDeleteQueries(struct OGLES2Context *ctx, GLsizei n, const GLuint *ids) { ogles2_deleteObjects(QT_OGLES2_K_Query, n, ids); }
GLboolean ogles2_GLIsQuery(struct OGLES2Context *ctx, GLuint id) { return (GLboolean) ogles2_call(QT_OGLES2_C_IsQuery, id, 0, 0, 0, 0); }
void ogles2_GLBeginQuery(struct OGLES2Context *ctx, GLenum target, GLuint id) { ogles2_call(QT_OGLES2_C_BeginQuery, target, id, 0, 0, 0); }
void ogles2_GLEndQuery(struct OGLES2Context *ctx, GLenum target) { ogles2_call(QT_OGLES2_C_EndQuery, target, 0, 0, 0, 0); }
void ogles2_GLGetQueryiv(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint *params) {
	if (params) ogles2_getParam(QT_OGLES2_P_Queryiv, target, pname, 0, params);
}
void ogles2_GLGetQueryObjectuiv(struct OGLES2Context *ctx, GLuint id, GLenum pname, GLuint *params) {
	if (params) ogles2_getParam(QT_OGLES2_P_QueryObjectuiv, id, pname, 0, params);
}

void ogles2_GLGenQueriesEXT(struct OGLES2Context *ctx, GLsizei n, GLuint *ids) { ogles2_GLGenQueries(ctx, n, ids); }
void ogles2_GLDeleteQueriesEXT(struct OGLES2Context *ctx, GLsizei n, const GLuint *ids) { ogles2_GLDeleteQueries(ctx, n, ids); }
GLboolean ogles2_GLIsQueryEXT(struct OGLES2Context *ctx, GLuint id) { return ogles2_GLIsQuery(ctx, id); }
void ogles2_GLBeginQueryEXT(struct OGLES2Context *ctx, GLenum target, GLuint id) { ogles2_GLBeginQuery(ctx, target, id); }
void ogles2_GLEndQueryEXT(struct OGLES2Context *ctx, GLenum target) { ogles2_GLEndQuery(ctx, target); }
void ogles2_GLGetQueryivEXT(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint *params) {
	ogles2_GLGetQueryiv(ctx, target, pname, params);
}
void ogles2_GLGetQueryObjectuivEXT(struct OGLES2Context *ctx, GLuint id, GLenum pname, GLuint *params) {
	ogles2_GLGetQueryObjectuiv(ctx, id, pname, params);
}

/* --- Transform feedback --------------------------------------------------- */

void ogles2_GLTransformFeedbackVaryings(struct OGLES2Context *ctx, GLuint program, GLsizei count, const GLchar *const *varyings,
		GLenum bufferMode) {
	ULONG *w = qt_reserve(QT_OGLES2_TRANSFORM_FEEDBACK_VARYINGS_WORDS);
	w[0] = ((ULONG) QT_OGLES2_TRANSFORM_FEEDBACK_VARYINGS << 16) | QT_OGLES2_TRANSFORM_FEEDBACK_VARYINGS_WORDS;
	w[1] = program;
	w[2] = (ULONG) count;
	w[3] = (ULONG) varyings;
	w[4] = bufferMode;
	qt_flush();
}

void ogles2_GLGetTransformFeedbackVarying(struct OGLES2Context *ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length,
		GLsizei *size, GLenum *type, GLchar *name) {
	ogles2_getActive(QT_OGLES2_A_TransformFeedbackVarying, program, index, bufSize, length, (GLint *) size, type, name);
}

void ogles2_GLBeginTransformFeedback(struct OGLES2Context *ctx, GLenum primitiveMode) {
	ogles2_call(QT_OGLES2_C_BeginTransformFeedback, primitiveMode, 0, 0, 0, 0);
}
void ogles2_GLEndTransformFeedback(struct OGLES2Context *ctx) { ogles2_call(QT_OGLES2_C_EndTransformFeedback, 0, 0, 0, 0, 0); }
void ogles2_GLPauseTransformFeedback(struct OGLES2Context *ctx) { ogles2_call(QT_OGLES2_C_PauseTransformFeedback, 0, 0, 0, 0, 0); }
void ogles2_GLResumeTransformFeedback(struct OGLES2Context *ctx) { ogles2_call(QT_OGLES2_C_ResumeTransformFeedback, 0, 0, 0, 0, 0); }
void ogles2_GLBindTransformFeedback(struct OGLES2Context *ctx, GLenum target, GLuint id) {
	ogles2_call(QT_OGLES2_C_BindTransformFeedback, target, id, 0, 0, 0);
}
void ogles2_GLGenTransformFeedbacks(struct OGLES2Context *ctx, GLsizei n, GLuint *ids) {
	ogles2_genObjects(QT_OGLES2_K_TransformFeedback, n, ids);
}
void ogles2_GLDeleteTransformFeedbacks(struct OGLES2Context *ctx, GLsizei n, const GLuint *ids) {
	ogles2_deleteObjects(QT_OGLES2_K_TransformFeedback, n, ids);
}
GLboolean ogles2_GLIsTransformFeedback(struct OGLES2Context *ctx, GLuint id) {
	return (GLboolean) ogles2_call(QT_OGLES2_C_IsTransformFeedback, id, 0, 0, 0, 0);
}

/* --- Uniform blocks ------------------------------------------------------- */

void ogles2_GLGetActiveUniformBlockiv(struct OGLES2Context *ctx, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params) {
	if (params) ogles2_getParam(QT_OGLES2_P_ActiveUniformBlockiv, program, uniformBlockIndex, pname, params);
}

void ogles2_GLGetActiveUniformBlockName(struct OGLES2Context *ctx, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize,
		GLsizei *length, GLchar *uniformBlockName) {
	ogles2_getActive(QT_OGLES2_A_UniformBlockName, program, uniformBlockIndex, bufSize, length, NULL, NULL, uniformBlockName);
}

void ogles2_GLGetUniformIndices(struct OGLES2Context *ctx, GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames,
		GLuint *uniformIndices) {
	ULONG *w = qt_reserve(QT_OGLES2_UNIFORM_INDICES_WORDS);
	w[0] = ((ULONG) QT_OGLES2_UNIFORM_INDICES << 16) | QT_OGLES2_UNIFORM_INDICES_WORDS;
	w[1] = program;
	w[2] = (ULONG) uniformCount;
	w[3] = (ULONG) uniformNames;
	w[4] = (ULONG) uniformIndices;
	qt_flush();
}

void ogles2_GLGetActiveUniformsiv(struct OGLES2Context *ctx, GLuint program, GLsizei uniformCount, const GLuint *uniformIndices,
		GLenum pname, GLint *params) {
	ULONG *w = qt_reserve(QT_OGLES2_ACTIVE_UNIFORMS_IV_WORDS);
	w[0] = ((ULONG) QT_OGLES2_ACTIVE_UNIFORMS_IV << 16) | QT_OGLES2_ACTIVE_UNIFORMS_IV_WORDS;
	w[1] = program;
	w[2] = (ULONG) uniformCount;
	w[3] = (ULONG) uniformIndices;
	w[4] = pname;
	w[5] = (ULONG) params;
	qt_flush();
}

/* --- Indexed and 64-bit queries ------------------------------------------- */

int ogles2_bindingValue(struct OGLES2Context *ctx, GLenum target, GLuint index, GLint *value); /* es31b.c */

/* The vertex buffer bindings are 68k-side (es31b.c), the rest the host's. */
void ogles2_GLGetIntegeri_v(struct OGLES2Context *ctx, GLenum target, GLuint index, GLint *data) {
	if (data && !ogles2_bindingValue(ctx, target, index, data)) ogles2_getParam(QT_OGLES2_P_Integeri_v, target, index, 0, data);
}
void ogles2_GLGetInteger64i_v(struct OGLES2Context *ctx, GLenum target, GLuint index, GLint64 *data) {
	GLint value = 0;
	if (!data) return;
	ogles2_GLGetIntegeri_v(ctx, target, index, &value);
	*data = value;
}
void ogles2_GLGetBooleani_v(struct OGLES2Context *ctx, GLenum target, GLuint index, GLboolean *data) {
	GLint value = 0;
	if (!data) return;
	ogles2_GLGetIntegeri_v(ctx, target, index, &value);
	*data = value != 0;
}
void ogles2_GLGetBufferParameteri64v(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint64 *params) {
	GLint value = 0;
	if (!params) return;
	ogles2_GLGetBufferParameteriv(ctx, target, pname, &value);
	*params = value;
}

void ogles2_GLGetSamplerParameteriv(struct OGLES2Context *ctx, GLuint sampler, GLenum pname, GLint *params) {
	if (params) ogles2_getParam(QT_OGLES2_P_SamplerParameteriv, sampler, pname, 0, params);
}
void ogles2_GLGetSamplerParameterfv(struct OGLES2Context *ctx, GLuint sampler, GLenum pname, GLfloat *params) {
	if (params) ogles2_getParam(QT_OGLES2_P_SamplerParameterfv, sampler, pname, 0, params);
}

/* Integer attributes keep their values as glVertexAttribI4* set them on
 * the host; the array state is the 68k side's, as glGetVertexAttribiv's. */
void ogles2_GLGetVertexAttribIiv(struct OGLES2Context *ctx, GLuint index, GLenum pname, GLint *params) {
	if (ctx && params && index < QT_OGLES2_MAX_ATTRIBS && pname == 0x88FD) *params = ctx->attribs[index].integer; /* GL_VERTEX_ATTRIB_ARRAY_INTEGER */
	else if (ctx && params && index < QT_OGLES2_MAX_ATTRIBS && pname == 0x88FE) *params = (GLint) ctx->attribs[index].divisor; /* _DIVISOR */
	else ogles2_GLGetVertexAttribiv(ctx, index, pname, params);
}
void ogles2_GLGetVertexAttribIuiv(struct OGLES2Context *ctx, GLuint index, GLenum pname, GLuint *params) {
	ogles2_GLGetVertexAttribIiv(ctx, index, pname, (GLint *) params);
}

/* glGetInternalformativ: every colour, depth and stencil format renders
 * with 4 samples on the host (GL_MAX_SAMPLES is far more), which is what
 * OpenGL ES 3.0 asks for at least. */
void ogles2_GLGetInternalformativ(struct OGLES2Context *ctx, GLenum target, GLenum internalformat, GLenum pname, GLsizei count,
		GLint *params) {
	if (!params || count < 1) return;
	if (pname == 0x9380) params[0] = 1;        /* GL_NUM_SAMPLE_COUNTS */
	else if (pname == 0x80A9) params[0] = 4;   /* GL_SAMPLES */
}
