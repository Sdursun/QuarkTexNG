// The OpenGL 3.3 functions and constants the host library uses beyond the
// OpenGL 1.1 that opengl32.dll exports. Declared here instead of taken from
// glext.h, which Visual Studio does not ship.
#ifndef QUARKTEX_GL3_H
#define QUARKTEX_GL3_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <cstddef>

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

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
	F(void, DrawBuffers, (GLsizei n, const GLenum* bufs))

namespace gl3 {
#define QT_GL3_DECLARE(result, name, parameters) extern result (APIENTRY* name) parameters;
	QT_GL3_FUNCTIONS(QT_GL3_DECLARE)
#undef QT_GL3_DECLARE

	// With a context current. False if a function is missing.
	bool load();

	// An OpenGL 3.3 core profile context for the device context, whose pixel
	// format is set already. 0 if the driver has none.
	HGLRC createCoreContext(HDC deviceContext);
}

#endif
