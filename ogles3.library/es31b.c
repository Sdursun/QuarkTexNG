#include "ogles2.h"
#include <proto/exec.h>

/*
 * The rest of OpenGL ES 3.1 (docs/phase10-ogles3.md, stage 3b): separate
 * vertex attribute formats and bindings, separable programs and program
 * pipelines, glProgramUniform*, the program interface queries,
 * multisample textures and the small queries.
 *
 * Attribute formats and bindings are 68k-side state, as the attributes are
 * (ogles2.h). Pipelines are real host objects; glProgramUniform* makes the
 * program current on the host for the call (host/ogles2.cpp). These need
 * the host's OpenGL 4.3 (an ES 3.1 context's condition); without it the
 * host ignores them.
 */

/* --- Separate vertex attribute formats and bindings ----------------------- */

/* What a draw needs is made of the attribute's format and its binding by
 * ogles2_bindingChanged (glfuncs.c). A binding with stride 0 is read as
 * tightly packed (the host's rule for glVertexAttribPointer), not as every
 * vertex reading the same element. */

static void attribFormat(struct OGLES2Context *ctx, GLuint index, GLint size, GLenum type, GLboolean normalized, GLuint offset,
		GLboolean integer) {
	QtOgles2Attrib *a;
	if (!ctx || index >= QT_OGLES2_MAX_ATTRIBS) return;
	a = &ctx->attribs[index];
	a->size = size;
	a->type = type;
	a->normalized = normalized;
	a->integer = integer;
	a->relativeOffset = offset;
	ogles2_bindingChanged(ctx, a->binding);
}

void ogles2_GLVertexAttribFormat(struct OGLES2Context *ctx, GLuint attribindex, GLint size, GLenum type, GLboolean normalized,
		GLuint relativeoffset) {
	attribFormat(ctx, attribindex, size, type, normalized, relativeoffset, GL_FALSE);
}

void ogles2_GLVertexAttribIFormat(struct OGLES2Context *ctx, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {
	attribFormat(ctx, attribindex, size, type, GL_FALSE, relativeoffset, GL_TRUE);
}

void ogles2_GLVertexAttribBinding(struct OGLES2Context *ctx, GLuint attribindex, GLuint bindingindex) {
	if (!ctx || attribindex >= QT_OGLES2_MAX_ATTRIBS || bindingindex >= QT_OGLES2_MAX_ATTRIBS) return;
	ctx->attribs[attribindex].binding = bindingindex;
	ogles2_bindingChanged(ctx, bindingindex);
}

void ogles2_GLBindVertexBuffer(struct OGLES2Context *ctx, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) {
	QtOgles2Binding *b;
	if (!ctx || bindingindex >= QT_OGLES2_MAX_ATTRIBS) return;
	b = &ctx->bindings[bindingindex];
	b->buffer = buffer;
	b->offset = (ULONG) offset;
	b->stride = stride;
	ogles2_bindingChanged(ctx, bindingindex);
}

void ogles2_GLVertexBindingDivisor(struct OGLES2Context *ctx, GLuint bindingindex, GLuint divisor) {
	if (!ctx || bindingindex >= QT_OGLES2_MAX_ATTRIBS) return;
	ctx->bindings[bindingindex].divisor = divisor;
	ogles2_bindingChanged(ctx, bindingindex);
}

/* glGetIntegeri_v's binding queries are answered here (es3query.c asks). */
int ogles2_bindingValue(struct OGLES2Context *ctx, GLenum target, GLuint index, GLint *value) {
	const QtOgles2Binding *b;
	if (!ctx || index >= QT_OGLES2_MAX_ATTRIBS) return 0;
	b = &ctx->bindings[index];
	switch (target) {
	case 0x8F4F: *value = (GLint) b->buffer; return 1;  /* GL_VERTEX_BINDING_BUFFER */
	case 0x82D6: *value = (GLint) b->divisor; return 1; /* GL_VERTEX_BINDING_DIVISOR */
	case 0x82D7: *value = (GLint) b->offset; return 1;  /* GL_VERTEX_BINDING_OFFSET */
	case 0x82D8: *value = b->stride; return 1;          /* GL_VERTEX_BINDING_STRIDE */
	}
	return 0;
}

/* --- Separable programs and program pipelines ----------------------------- */

#define GL_PROGRAM_SEPARABLE_ 0x8258
#define GL_COMPILE_STATUS_ 0x8B81

void ogles2_GLGenProgramPipelines(struct OGLES2Context *ctx, GLsizei n, GLuint *pipelines) {
	ogles2_genObjects(QT_OGLES2_K_ProgramPipeline, n, pipelines);
}
void ogles2_GLDeleteProgramPipelines(struct OGLES2Context *ctx, GLsizei n, const GLuint *pipelines) {
	ogles2_deleteObjects(QT_OGLES2_K_ProgramPipeline, n, pipelines);
}
GLboolean ogles2_GLIsProgramPipeline(struct OGLES2Context *ctx, GLuint pipeline) {
	return (GLboolean) ogles2_call(QT_OGLES2_C_IsProgramPipeline, pipeline, 0, 0, 0, 0);
}
void ogles2_GLBindProgramPipeline(struct OGLES2Context *ctx, GLuint pipeline) {
	ogles2_call(QT_OGLES2_C_BindProgramPipeline, pipeline, 0, 0, 0, 0);
}
void ogles2_GLUseProgramStages(struct OGLES2Context *ctx, GLuint pipeline, GLbitfield stages, GLuint program) {
	ogles2_call(QT_OGLES2_C_UseProgramStages, pipeline, stages, program, 0, 0);
}
void ogles2_GLActiveShaderProgram(struct OGLES2Context *ctx, GLuint pipeline, GLuint program) {
	ogles2_call(QT_OGLES2_C_ActiveShaderProgram, pipeline, program, 0, 0, 0);
}
void ogles2_GLValidateProgramPipeline(struct OGLES2Context *ctx, GLuint pipeline) {
	ogles2_call(QT_OGLES2_C_ValidateProgramPipeline, pipeline, 0, 0, 0, 0);
}
void ogles2_GLGetProgramPipelineiv(struct OGLES2Context *ctx, GLuint pipeline, GLenum pname, GLint *params) {
	if (params) ogles2_getParam(QT_OGLES2_P_ProgramPipelineiv, pipeline, pname, 0, params);
}
void ogles2_GLGetProgramPipelineInfoLog(struct OGLES2Context *ctx, GLuint pipeline, GLsizei bufSize, GLsizei *length,
		GLchar *infoLog) {
	ULONG *w = qt_reserve(QT_OGLES2_GET_LOG_WORDS);
	w[0] = ((ULONG) QT_OGLES2_GET_LOG << 16) | QT_OGLES2_GET_LOG_WORDS;
	w[1] = QT_OGLES2_G_ProgramPipeline;
	w[2] = pipeline;
	w[3] = (ULONG) bufSize;
	w[4] = (ULONG) length;
	w[5] = (ULONG) infoLog;
	qt_flush();
}

/* glfuncs.c's entries glCreateShaderProgramv is made of. */
GLuint ogles2_GLCreateShader(struct OGLES2Context *ctx, GLenum type);
void ogles2_GLShaderSource(struct OGLES2Context *ctx, GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length);
void ogles2_GLCompileShader(struct OGLES2Context *ctx, GLuint shader);
void ogles2_GLGetShaderiv(struct OGLES2Context *ctx, GLuint shader, GLenum pname, GLint *params);
GLuint ogles2_GLCreateProgram(struct OGLES2Context *ctx);
void ogles2_GLAttachShader(struct OGLES2Context *ctx, GLuint program, GLuint shader);
void ogles2_GLDetachShader(struct OGLES2Context *ctx, GLuint program, GLuint shader);
void ogles2_GLLinkProgram(struct OGLES2Context *ctx, GLuint program);
void ogles2_GLDeleteShader(struct OGLES2Context *ctx, GLuint shader);

/* GL_PROGRAM_BINARY_RETRIEVABLE_HINT: no binaries, nothing to do. */
void ogles2_GLProgramParameteri(struct OGLES2Context *ctx, GLuint program, GLenum pname, GLint value) {
	if (pname == GL_PROGRAM_SEPARABLE_) ogles2_call(QT_OGLES2_C_ProgramParameteri, program, pname, (ULONG) value, 0, 0);
}

/* As OpenGL ES 3.1 defines it: a shader compiled, and a separable program
 * linked of it alone. The shader's compiler messages are not added to the
 * program's log. */
GLuint ogles2_GLCreateShaderProgramv(struct OGLES2Context *ctx, GLenum type, GLsizei count, const GLchar *const *strings) {
	GLuint shader, program;
	GLint compiled = 0;
	if (!ctx || !(shader = ogles2_GLCreateShader(ctx, type))) return 0;
	ogles2_GLShaderSource(ctx, shader, count, strings, NULL);
	ogles2_GLCompileShader(ctx, shader);
	program = ogles2_GLCreateProgram(ctx);
	if (program) {
		ogles2_GLGetShaderiv(ctx, shader, GL_COMPILE_STATUS_, &compiled);
		ogles2_GLProgramParameteri(ctx, program, GL_PROGRAM_SEPARABLE_, GL_TRUE);
		if (compiled) {
			ogles2_GLAttachShader(ctx, program, shader);
			ogles2_GLLinkProgram(ctx, program);
			ogles2_GLDetachShader(ctx, program, shader);
		}
	}
	ogles2_GLDeleteShader(ctx, shader);
	return program;
}

/* --- glProgramUniform* ----------------------------------------------------- */

static void programUniformV(ULONG subop, GLuint program, GLint location, GLsizei count, const void *value, GLboolean transpose) {
	ULONG *w = qt_reserve(QT_OGLES2_PROGRAM_UNIFORM_V_WORDS);
	w[0] = ((ULONG) QT_OGLES2_PROGRAM_UNIFORM_V << 16) | QT_OGLES2_PROGRAM_UNIFORM_V_WORDS;
	w[1] = subop;
	w[2] = program;
	w[3] = (ULONG) location;
	w[4] = (ULONG) count;
	w[5] = (ULONG) value;
	w[6] = transpose ? 1 : 0;
	qt_flush();
}

/* The scalar forms: their values put in an array, one vector of them. */
static void programUniform(ULONG subop, GLuint program, GLint location, ULONG v0, ULONG v1, ULONG v2, ULONG v3) {
	ULONG v[4];
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
	v[3] = v3;
	programUniformV(subop, program, location, 1, v, GL_FALSE);
}

#define F ogles2_floatWord
void ogles2_GLProgramUniform1f(struct OGLES2Context *ctx, GLuint program, GLint location, GLfloat v0) {
	programUniform(QT_OGLES2_U_1F, program, location, F(v0), 0, 0, 0);
}
void ogles2_GLProgramUniform2f(struct OGLES2Context *ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1) {
	programUniform(QT_OGLES2_U_2F, program, location, F(v0), F(v1), 0, 0);
}
void ogles2_GLProgramUniform3f(struct OGLES2Context *ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
	programUniform(QT_OGLES2_U_3F, program, location, F(v0), F(v1), F(v2), 0);
}
void ogles2_GLProgramUniform4f(struct OGLES2Context *ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2,
		GLfloat v3) {
	programUniform(QT_OGLES2_U_4F, program, location, F(v0), F(v1), F(v2), F(v3));
}
#undef F
void ogles2_GLProgramUniform1i(struct OGLES2Context *ctx, GLuint program, GLint location, GLint v0) {
	programUniform(QT_OGLES2_U_1I, program, location, (ULONG) v0, 0, 0, 0);
}
void ogles2_GLProgramUniform2i(struct OGLES2Context *ctx, GLuint program, GLint location, GLint v0, GLint v1) {
	programUniform(QT_OGLES2_U_2I, program, location, (ULONG) v0, (ULONG) v1, 0, 0);
}
void ogles2_GLProgramUniform3i(struct OGLES2Context *ctx, GLuint program, GLint location, GLint v0, GLint v1, GLint v2) {
	programUniform(QT_OGLES2_U_3I, program, location, (ULONG) v0, (ULONG) v1, (ULONG) v2, 0);
}
void ogles2_GLProgramUniform4i(struct OGLES2Context *ctx, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
	programUniform(QT_OGLES2_U_4I, program, location, (ULONG) v0, (ULONG) v1, (ULONG) v2, (ULONG) v3);
}
void ogles2_GLProgramUniform1ui(struct OGLES2Context *ctx, GLuint program, GLint location, GLuint v0) {
	programUniform(QT_OGLES2_U_1UI, program, location, v0, 0, 0, 0);
}
void ogles2_GLProgramUniform2ui(struct OGLES2Context *ctx, GLuint program, GLint location, GLuint v0, GLuint v1) {
	programUniform(QT_OGLES2_U_2UI, program, location, v0, v1, 0, 0);
}
void ogles2_GLProgramUniform3ui(struct OGLES2Context *ctx, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) {
	programUniform(QT_OGLES2_U_3UI, program, location, v0, v1, v2, 0);
}
void ogles2_GLProgramUniform4ui(struct OGLES2Context *ctx, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2,
		GLuint v3) {
	programUniform(QT_OGLES2_U_4UI, program, location, v0, v1, v2, v3);
}

/* Written out, not by a macro: oglesgen.py finds the entries by their
 * definitions. */
void ogles2_GLProgramUniform1fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_1F, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform2fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_2F, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform3fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_3F, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform4fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_4F, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform1iv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLint *value) {
	programUniformV(QT_OGLES2_U_1I, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform2iv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLint *value) {
	programUniformV(QT_OGLES2_U_2I, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform3iv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLint *value) {
	programUniformV(QT_OGLES2_U_3I, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform4iv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLint *value) {
	programUniformV(QT_OGLES2_U_4I, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform1uiv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLuint *value) {
	programUniformV(QT_OGLES2_U_1UI, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform2uiv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLuint *value) {
	programUniformV(QT_OGLES2_U_2UI, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform3uiv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLuint *value) {
	programUniformV(QT_OGLES2_U_3UI, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniform4uiv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count, const GLuint *value) {
	programUniformV(QT_OGLES2_U_4UI, program, location, count, value, GL_FALSE);
}
void ogles2_GLProgramUniformMatrix2fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count,
		GLboolean transpose, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_M2, program, location, count, value, transpose);
}
void ogles2_GLProgramUniformMatrix3fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count,
		GLboolean transpose, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_M3, program, location, count, value, transpose);
}
void ogles2_GLProgramUniformMatrix4fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count,
		GLboolean transpose, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_M4, program, location, count, value, transpose);
}
void ogles2_GLProgramUniformMatrix2x3fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count,
		GLboolean transpose, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_M2x3, program, location, count, value, transpose);
}
void ogles2_GLProgramUniformMatrix3x2fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count,
		GLboolean transpose, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_M3x2, program, location, count, value, transpose);
}
void ogles2_GLProgramUniformMatrix2x4fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count,
		GLboolean transpose, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_M2x4, program, location, count, value, transpose);
}
void ogles2_GLProgramUniformMatrix4x2fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count,
		GLboolean transpose, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_M4x2, program, location, count, value, transpose);
}
void ogles2_GLProgramUniformMatrix3x4fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count,
		GLboolean transpose, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_M3x4, program, location, count, value, transpose);
}
void ogles2_GLProgramUniformMatrix4x3fv(struct OGLES2Context *ctx, GLuint program, GLint location, GLsizei count,
		GLboolean transpose, const GLfloat *value) {
	programUniformV(QT_OGLES2_U_M4x3, program, location, count, value, transpose);
}

/* --- Program interface queries --------------------------------------------- */

static LONG programResource(ULONG subop, GLuint program, GLenum programInterface, ULONG index, const void *address, ULONG propCount,
		GLsizei bufSize, GLsizei *length, void *out) {
	ULONG *w = qt_reserve(QT_OGLES2_PROGRAM_RESOURCE_WORDS);
	w[0] = ((ULONG) QT_OGLES2_PROGRAM_RESOURCE << 16) | QT_OGLES2_PROGRAM_RESOURCE_WORDS;
	w[1] = subop;
	w[2] = program;
	w[3] = programInterface;
	w[4] = index;
	w[5] = (ULONG) address;
	w[6] = propCount;
	w[7] = (ULONG) bufSize;
	w[8] = (ULONG) length;
	w[9] = (ULONG) out;
	return (LONG) qt_flush();
}

void ogles2_GLGetProgramInterfaceiv(struct OGLES2Context *ctx, GLuint program, GLenum programInterface, GLenum pname, GLint *params) {
	if (params) programResource(QT_OGLES2_R_Interfaceiv, program, programInterface, pname, NULL, 0, 0, NULL, params);
}
GLuint ogles2_GLGetProgramResourceIndex(struct OGLES2Context *ctx, GLuint program, GLenum programInterface, const GLchar *name) {
	return (GLuint) programResource(QT_OGLES2_R_Index, program, programInterface, 0, name, 0, 0, NULL, NULL);
}
GLint ogles2_GLGetProgramResourceLocation(struct OGLES2Context *ctx, GLuint program, GLenum programInterface, const GLchar *name) {
	return (GLint) programResource(QT_OGLES2_R_Location, program, programInterface, 0, name, 0, 0, NULL, NULL);
}
void ogles2_GLGetProgramResourceName(struct OGLES2Context *ctx, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize,
		GLsizei *length, GLchar *name) {
	programResource(QT_OGLES2_R_Name, program, programInterface, index, NULL, 0, bufSize, length, name);
}
void ogles2_GLGetProgramResourceiv(struct OGLES2Context *ctx, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount,
		const GLenum *props, GLsizei count, GLsizei *length, GLint *params) {
	if (propCount <= 0 || !props) return;
	programResource(QT_OGLES2_R_iv, program, programInterface, index, props, (ULONG) propCount, count, length, params);
}

/* --- Multisample textures, sample masks, framebuffer parameters ---------- */

void ogles2_GLTexStorage2DMultisample(struct OGLES2Context *ctx, GLenum target, GLsizei samples, GLenum internalformat,
		GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
	ULONG words[10] = {0};
	words[0] = target;
	words[1] = (ULONG) samples;
	words[2] = internalformat;
	words[3] = (ULONG) width;
	words[4] = (ULONG) height;
	words[5] = fixedsamplelocations;
	ogles2_call10(QT_OGLES2_C_TexStorage2DMultisample, words);
}
void ogles2_GLSampleMaski(struct OGLES2Context *ctx, GLuint maskNumber, GLbitfield mask) {
	ogles2_call(QT_OGLES2_C_SampleMaski, maskNumber, mask, 0, 0, 0);
}
/* Two floats, the sample's position. */
void ogles2_GLGetMultisamplefv(struct OGLES2Context *ctx, GLenum pname, GLuint index, GLfloat *val) {
	if (val) ogles2_getParam(QT_OGLES2_P_Multisamplefv, pname, index, 0, val);
}
void ogles2_GLGetTexLevelParameteriv(struct OGLES2Context *ctx, GLenum target, GLint level, GLenum pname, GLint *params) {
	if (params) ogles2_getParam(QT_OGLES2_P_TexLevelParameteriv, target, (ULONG) level, pname, params);
}
void ogles2_GLGetTexLevelParameterfv(struct OGLES2Context *ctx, GLenum target, GLint level, GLenum pname, GLfloat *params) {
	if (params) ogles2_getParam(QT_OGLES2_P_TexLevelParameterfv, target, (ULONG) level, pname, params);
}
void ogles2_GLFramebufferParameteri(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint param) {
	ogles2_call(QT_OGLES2_C_FramebufferParameteri, target, pname, (ULONG) param, 0, 0);
}
void ogles2_GLGetFramebufferParameteriv(struct OGLES2Context *ctx, GLenum target, GLenum pname, GLint *params) {
	if (params) ogles2_getParam(QT_OGLES2_P_FramebufferParameteriv, target, pname, 0, params);
}
