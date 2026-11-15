// The OpenGL 3.3 and 4.3 functions and constants the host library uses
// beyond the OpenGL 1.1 that opengl32.dll exports. Declared here instead of
// taken from glext.h, which Visual Studio does not ship.
//
// Core contexts are OpenGL 4.3 since 2026-11 (ogles3.library's OpenGL ES 3.1
// needs it: compute shaders, storage buffers, images); a driver without 4.3
// gets 3.3, with which everything but ES 3.1 still runs. QT_GL3_FUNCTIONS
// must all be there; QT_GL43_FUNCTIONS only in a 4.3 context (has43()).
#ifndef QUARKTEX_GL3_H
#define QUARKTEX_GL3_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <cstddef>

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

#define GL_CLAMP_TO_EDGE 0x812F
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_RG 0x8227
#define GL_R8 0x8229
#define GL_RG8 0x822B
#define GL_TEXTURE_SWIZZLE_R 0x8E42
#define GL_TEXTURE_SWIZZLE_G 0x8E43
#define GL_TEXTURE_SWIZZLE_B 0x8E44
#define GL_TEXTURE_SWIZZLE_A 0x8E45
#define GL_ARRAY_BUFFER 0x8892
#define GL_STREAM_DRAW 0x88E0
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84

// wglCreateContextAttribsARB (WGL_ARB_create_context, _profile)
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x0001

#define QT_GL3_FUNCTIONS(F) \
	F(GLuint, CreateShader, (GLenum type)) \
	F(void, ShaderSource, (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)) \
	F(void, CompileShader, (GLuint shader)) \
	F(void, GetShaderiv, (GLuint shader, GLenum pname, GLint* params)) \
	F(void, GetShaderInfoLog, (GLuint shader, GLsizei size, GLsizei* length, GLchar* log)) \
	F(void, DeleteShader, (GLuint shader)) \
	F(GLuint, CreateProgram, (void)) \
	F(void, AttachShader, (GLuint program, GLuint shader)) \
	F(void, DetachShader, (GLuint program, GLuint shader)) \
	F(void, LinkProgram, (GLuint program)) \
	F(void, GetProgramiv, (GLuint program, GLenum pname, GLint* params)) \
	F(void, GetProgramInfoLog, (GLuint program, GLsizei size, GLsizei* length, GLchar* log)) \
	F(void, UseProgram, (GLuint program)) \
	F(void, DeleteProgram, (GLuint program)) \
	F(void, ValidateProgram, (GLuint program)) \
	F(GLboolean, IsShader, (GLuint shader)) \
	F(GLboolean, IsProgram, (GLuint program)) \
	F(void, BindAttribLocation, (GLuint program, GLuint index, const GLchar* name)) \
	F(GLint, GetAttribLocation, (GLuint program, const GLchar* name)) \
	F(GLint, GetUniformLocation, (GLuint program, const GLchar* name)) \
	F(void, Uniform1i, (GLint location, GLint v0)) \
	F(void, Uniform2i, (GLint location, GLint v0, GLint v1)) \
	F(void, Uniform3i, (GLint location, GLint v0, GLint v1, GLint v2)) \
	F(void, Uniform4i, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
	F(void, Uniform1f, (GLint location, GLfloat v0)) \
	F(void, Uniform2f, (GLint location, GLfloat v0, GLfloat v1)) \
	F(void, Uniform3f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
	F(void, Uniform4f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
	F(void, UniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
	F(void, GenVertexArrays, (GLsizei n, GLuint* arrays)) \
	F(void, BindVertexArray, (GLuint array)) \
	F(void, DeleteVertexArrays, (GLsizei n, const GLuint* arrays)) \
	F(void, GenBuffers, (GLsizei n, GLuint* buffers)) \
	F(void, BindBuffer, (GLenum target, GLuint buffer)) \
	F(void, BufferData, (GLenum target, GLsizeiptr size, const void* data, GLenum usage)) \
	F(void, BufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void* data)) \
	F(void, CopyBufferSubData, (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
	F(void, DeleteBuffers, (GLsizei n, const GLuint* buffers)) \
	F(void, VertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)) \
	F(void, EnableVertexAttribArray, (GLuint index)) \
	F(void, DisableVertexAttribArray, (GLuint index)) \
	F(void, GenerateMipmap, (GLenum target)) \
	F(void, ActiveTexture, (GLenum texture)) \
	F(void, GenFramebuffers, (GLsizei n, GLuint* framebuffers)) \
	F(void, DeleteFramebuffers, (GLsizei n, const GLuint* framebuffers)) \
	F(void, BindFramebuffer, (GLenum target, GLuint framebuffer)) \
	F(void, FramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
	F(GLenum, CheckFramebufferStatus, (GLenum target)) \
	F(void, GenRenderbuffers, (GLsizei n, GLuint* renderbuffers)) \
	F(void, DeleteRenderbuffers, (GLsizei n, const GLuint* renderbuffers)) \
	F(void, BindRenderbuffer, (GLenum target, GLuint renderbuffer)) \
	F(void, RenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)) \
	F(void, FramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
	F(void, BlendColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
	F(void, BlendEquation, (GLenum mode)) \
	F(void, BlendEquationSeparate, (GLenum modeRGB, GLenum modeAlpha)) \
	F(void, BlendFuncSeparate, (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
	F(void, StencilFuncSeparate, (GLenum face, GLenum func, GLint ref, GLuint mask)) \
	F(void, StencilMaskSeparate, (GLenum face, GLuint mask)) \
	F(void, StencilOpSeparate, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)) \
	F(void, SampleCoverage, (GLfloat value, GLboolean invert)) \
	F(GLboolean, IsFramebuffer, (GLuint framebuffer)) \
	F(GLboolean, IsRenderbuffer, (GLuint renderbuffer)) \
	F(void, VertexAttrib4f, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
	F(void, ProvokingVertex, (GLenum mode)) \
	F(void, Uniform1fv, (GLint location, GLsizei count, const GLfloat* value)) \
	F(void, Uniform2fv, (GLint location, GLsizei count, const GLfloat* value)) \
	F(void, Uniform3fv, (GLint location, GLsizei count, const GLfloat* value)) \
	F(void, Uniform4fv, (GLint location, GLsizei count, const GLfloat* value)) \
	F(void, Uniform1iv, (GLint location, GLsizei count, const GLint* value)) \
	F(void, Uniform2iv, (GLint location, GLsizei count, const GLint* value)) \
	F(void, Uniform3iv, (GLint location, GLsizei count, const GLint* value)) \
	F(void, Uniform4iv, (GLint location, GLsizei count, const GLint* value)) \
	F(void, UniformMatrix2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
	F(void, UniformMatrix3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
	F(void, GetUniformfv, (GLuint program, GLint location, GLfloat* params)) \
	F(void, GetUniformiv, (GLuint program, GLint location, GLint* params)) \
	F(void, GetActiveAttrib, (GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)) \
	F(void, GetActiveUniform, (GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)) \
	F(void, GetAttachedShaders, (GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)) \
	F(void, GetShaderSource, (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source)) \
	F(void, GetRenderbufferParameteriv, (GLenum target, GLenum pname, GLint* params)) \
	F(void, GetFramebufferAttachmentParameteriv, (GLenum target, GLenum attachment, GLenum pname, GLint* params)) \
	F(void, DrawBuffers, (GLsizei n, const GLenum* bufs)) \
	/* OpenGL ES 3.0's, for ogles3.library (all OpenGL 3.3 core). */ \
	F(void, VertexAttribIPointer, (GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)) \
	F(void, VertexAttribDivisor, (GLuint index, GLuint divisor)) \
	F(void, DrawArraysInstanced, (GLenum mode, GLint first, GLsizei count, GLsizei instances)) \
	F(void, DrawElementsInstanced, (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instances)) \
	F(void, BindBufferRange, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
	F(GLuint, GetUniformBlockIndex, (GLuint program, const GLchar* name)) \
	F(void, UniformBlockBinding, (GLuint program, GLuint index, GLuint binding)) \
	F(void, GenSamplers, (GLsizei n, GLuint* samplers)) \
	F(void, DeleteSamplers, (GLsizei n, const GLuint* samplers)) \
	F(GLboolean, IsSampler, (GLuint sampler)) \
	F(void, BindSampler, (GLuint unit, GLuint sampler)) \
	F(void, SamplerParameteri, (GLuint sampler, GLenum pname, GLint param)) \
	F(void, SamplerParameterf, (GLuint sampler, GLenum pname, GLfloat param)) \
	F(void, GenQueries, (GLsizei n, GLuint* ids)) \
	F(void, DeleteQueries, (GLsizei n, const GLuint* ids)) \
	F(void, TexImage3D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)) \
	F(void, TexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)) \
	F(void, CopyTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
	F(void, Uniform1uiv, (GLint location, GLsizei count, const GLuint* value)) \
	F(void, Uniform2uiv, (GLint location, GLsizei count, const GLuint* value)) \
	F(void, Uniform3uiv, (GLint location, GLsizei count, const GLuint* value)) \
	F(void, Uniform4uiv, (GLint location, GLsizei count, const GLuint* value)) \
	F(void, UniformMatrix2x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
	F(void, UniformMatrix3x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
	F(void, UniformMatrix2x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
	F(void, UniformMatrix4x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
	F(void, UniformMatrix3x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
	F(void, UniformMatrix4x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
	F(GLint, GetFragDataLocation, (GLuint program, const GLchar* name)) \
	F(void, BlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
	F(void, RenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
	F(void, FramebufferTextureLayer, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
	F(void, ClearBufferiv, (GLenum buffer, GLint drawbuffer, const GLint* value)) \
	F(void, ClearBufferuiv, (GLenum buffer, GLint drawbuffer, const GLuint* value)) \
	F(void, ClearBufferfv, (GLenum buffer, GLint drawbuffer, const GLfloat* value)) \
	F(void, ClearBufferfi, (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)) \
	F(void, VertexAttribI4i, (GLuint index, GLint x, GLint y, GLint z, GLint w)) \
	F(void, VertexAttribI4ui, (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
	F(void, GetUniformuiv, (GLuint program, GLint location, GLuint* params)) \
	F(void, BeginQuery, (GLenum target, GLuint id)) \
	F(void, EndQuery, (GLenum target)) \
	F(GLboolean, IsQuery, (GLuint id)) \
	F(void, GetQueryiv, (GLenum target, GLenum pname, GLint* params)) \
	F(void, GetQueryObjectuiv, (GLuint id, GLenum pname, GLuint* params)) \
	F(void, BeginTransformFeedback, (GLenum mode)) \
	F(void, EndTransformFeedback, (void)) \
	F(void, TransformFeedbackVaryings, (GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)) \
	F(void, GetTransformFeedbackVarying, (GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)) \
	F(void, GetActiveUniformBlockiv, (GLuint program, GLuint index, GLenum pname, GLint* params)) \
	F(void, GetActiveUniformBlockName, (GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)) \
	F(void, GetUniformIndices, (GLuint program, GLsizei count, const GLchar* const* names, GLuint* indices)) \
	F(void, GetActiveUniformsiv, (GLuint program, GLsizei count, const GLuint* indices, GLenum pname, GLint* params)) \
	F(void, GetIntegeri_v, (GLenum target, GLuint index, GLint* data)) \
	F(void, GetSamplerParameteriv, (GLuint sampler, GLenum pname, GLint* params)) \
	F(void, GetSamplerParameterfv, (GLuint sampler, GLenum pname, GLfloat* params)) \
	F(void, GetBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, void* data)) \
	/* OpenGL 1.3's compressed textures, which a core context reaches only */ \
	/* through wglGetProcAddress too. */ \
	F(void, CompressedTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)) \
	F(void, CompressedTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)) \
	F(void, CompressedTexImage3D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)) \
	F(void, CompressedTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data))

// Loaded where the driver has them, needed by neither list: a missing one is
// 0 (check before calling).
#define QT_GL_OPTIONAL_FUNCTIONS(F) \
	F(void, ClipControl, (GLenum origin, GLenum depth)) /* OpenGL 4.5 (GL_EXT_clip_control) */

// OpenGL 4.3's (OpenGL ES 3.1, ogles3.library stage 3).
#define QT_GL43_FUNCTIONS(F) \
	F(void, DispatchCompute, (GLuint x, GLuint y, GLuint z)) \
	F(void, DispatchComputeIndirect, (GLintptr indirect)) \
	F(void, MemoryBarrier, (GLbitfield barriers)) \
	F(void, BindImageTexture, (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)) \
	F(void, DrawArraysIndirect, (GLenum mode, const void* indirect)) \
	F(void, DrawElementsIndirect, (GLenum mode, GLenum type, const void* indirect)) \
	/* Transform feedback objects (OpenGL 4.0; ES 3.0 has them). */ \
	F(void, PauseTransformFeedback, (void)) \
	F(void, ResumeTransformFeedback, (void)) \
	F(void, BindTransformFeedback, (GLenum target, GLuint id)) \
	F(void, GenTransformFeedbacks, (GLsizei n, GLuint* ids)) \
	F(void, DeleteTransformFeedbacks, (GLsizei n, const GLuint* ids)) \
	F(GLboolean, IsTransformFeedback, (GLuint id)) \
	/* The rest of OpenGL ES 3.1 (stage 3b): separable programs and */ \
	/* pipelines (4.1), the program interface queries, multisample */ \
	/* textures (3.2, 4.3), framebuffer parameters (4.3). */ \
	F(void, ProgramParameteri, (GLuint program, GLenum pname, GLint value)) \
	F(void, GenProgramPipelines, (GLsizei n, GLuint* pipelines)) \
	F(void, DeleteProgramPipelines, (GLsizei n, const GLuint* pipelines)) \
	F(void, BindProgramPipeline, (GLuint pipeline)) \
	F(void, UseProgramStages, (GLuint pipeline, GLbitfield stages, GLuint program)) \
	F(void, ActiveShaderProgram, (GLuint pipeline, GLuint program)) \
	F(void, ValidateProgramPipeline, (GLuint pipeline)) \
	F(GLboolean, IsProgramPipeline, (GLuint pipeline)) \
	F(void, GetProgramPipelineiv, (GLuint pipeline, GLenum pname, GLint* params)) \
	F(void, GetProgramPipelineInfoLog, (GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog)) \
	F(void, GetProgramInterfaceiv, (GLuint program, GLenum programInterface, GLenum pname, GLint* params)) \
	F(GLuint, GetProgramResourceIndex, (GLuint program, GLenum programInterface, const GLchar* name)) \
	F(void, GetProgramResourceName, (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)) \
	F(void, GetProgramResourceiv, (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei count, GLsizei* length, GLint* params)) \
	F(GLint, GetProgramResourceLocation, (GLuint program, GLenum programInterface, const GLchar* name)) \
	F(void, TexStorage2DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
	F(void, SampleMaski, (GLuint maskNumber, GLbitfield mask)) \
	F(void, GetMultisamplefv, (GLenum pname, GLuint index, GLfloat* val)) \
	F(void, FramebufferParameteri, (GLenum target, GLenum pname, GLint param)) \
	F(void, GetFramebufferParameteriv, (GLenum target, GLenum pname, GLint* params))

namespace gl3 {
#define QT_GL3_DECLARE(result, name, parameters) extern result (APIENTRY* name) parameters;
	QT_GL3_FUNCTIONS(QT_GL3_DECLARE)
	QT_GL43_FUNCTIONS(QT_GL3_DECLARE)
	QT_GL_OPTIONAL_FUNCTIONS(QT_GL3_DECLARE)
#undef QT_GL3_DECLARE

	// With a context current. False if one of QT_GL3_FUNCTIONS is missing;
	// QT_GL43_FUNCTIONS are loaded too, and has43() says whether all of them
	// are there.
	bool load();
	bool has43();

	// A core profile context for the device context, whose pixel format is
	// set already: OpenGL 4.3, or 3.3 if the driver has no 4.3 (version
	// receives 43 or 33). 0 if it has neither. With share, the new context
	// shares its objects (textures, buffers, shaders, programs,
	// renderbuffers) with that one's.
	HGLRC createCoreContext(HDC deviceContext, int* version, HGLRC share = 0);
}

#endif
