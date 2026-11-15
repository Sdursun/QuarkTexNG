// Commands of QuarkTex's ogles2.library (gl/oglescmd.h), executed in its
// OpenGL 3.3 core context (docs/phase9-ogles2.md, stage 2).
//
// Shaders, programs and uniforms are real OpenGL 2.0 core entries, the same
// dynamically loaded set host/ffp.cpp uses (host/gl3.h: opengl32.dll only
// exports OpenGL 1.1, so anything newer -- glCreateShader and the rest --
// is fetched through wglGetProcAddress, not linked directly). They are all
// scalar, or take GLchar* text (plain bytes, no order to convert), so they
// go through a handful of generic shapes (QT_OGLES2_SIMPLE/NAME/UNIFORM/
// GET_IV/GET_LOG) rather than one opcode each.
//
// Buffers: GL_ARRAY_BUFFER/GL_ELEMENT_ARRAY_BUFFER data is kept as a raw,
// still big-endian copy, under QuarkTex's own small-integer names (not real
// GL buffer objects): a vertex attribute's type is not known until
// QT_OGLES2_DRAW names it (glVertexAttribPointer is 68k-side only, see
// gl/oglescmd.h), so the byte order conversion happens then, per attribute,
// the same way minigl.library's vertex arrays are (host/mgl.cpp); this file
// shares that file's component()-style conversion. What a draw converts from
// a buffer is kept, per attribute layout, in a real host buffer (Cached), and
// only converted again where glBufferData/glBufferSubData changed it.
//
// The unit test (tests/host) includes this file with QT_TEST defined; the
// OpenGL 2.0 entries are not meaningfully exercised there (no real context,
// and host/gl3.h is Windows-only), only compiled against small stand-ins so
// the file links, the same way the real _gl* encoders are recording stubs
// there.
#ifndef QT_TEST
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include "gl3.h"
#define QT_GL(name) gl##name
#define QT_GL3(name) gl3::name
#else
typedef long GLsizeiptr;
typedef long GLintptr;
#define GL_STREAM_DRAW 0x88E0
namespace gl3test {
	inline GLuint CreateShader(GLenum) { return 1; }
	inline void ShaderSource(GLuint, GLsizei, const GLchar* const*, const GLint*) {}
	inline void CompileShader(GLuint) {}
	inline void GetShaderiv(GLuint, GLenum, GLint* p) { if (p) *p = 1; }
	inline void GetShaderInfoLog(GLuint, GLsizei, GLsizei* length, GLchar*) { if (length) *length = 0; }
	inline void DeleteShader(GLuint) {}
	inline GLuint CreateProgram() { return 1; }
	inline void AttachShader(GLuint, GLuint) {}
	inline void DetachShader(GLuint, GLuint) {}
	inline void LinkProgram(GLuint) {}
	inline void GetProgramiv(GLuint, GLenum, GLint* p) { if (p) *p = 1; }
	inline void GetProgramInfoLog(GLuint, GLsizei, GLsizei* length, GLchar*) { if (length) *length = 0; }
	inline void UseProgram(GLuint) {}
	inline void DeleteProgram(GLuint) {}
	inline void ValidateProgram(GLuint) {}
	inline GLboolean IsShader(GLuint) { return 1; }
	inline GLboolean IsProgram(GLuint) { return 1; }
	inline void BindAttribLocation(GLuint, GLuint, const GLchar*) {}
	inline GLint GetAttribLocation(GLuint, const GLchar*) { return 0; }
	inline GLint GetUniformLocation(GLuint, const GLchar*) { return 0; }
	inline void Uniform1f(GLint, GLfloat) {}
	inline void Uniform2f(GLint, GLfloat, GLfloat) {}
	inline void Uniform3f(GLint, GLfloat, GLfloat, GLfloat) {}
	inline void Uniform4f(GLint, GLfloat, GLfloat, GLfloat, GLfloat) {}
	inline void Uniform1i(GLint, GLint) {}
	inline void Uniform2i(GLint, GLint, GLint) {}
	inline void Uniform3i(GLint, GLint, GLint, GLint) {}
	inline void Uniform4i(GLint, GLint, GLint, GLint, GLint) {}
	inline void VertexAttribPointer(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) {}
	inline void EnableVertexAttribArray(GLuint) {}
	inline void DisableVertexAttribArray(GLuint) {}
	inline void GenBuffers(GLsizei, GLuint* buffers) { if (buffers) *buffers = 1; }
	inline void BindBuffer(GLenum, GLuint) {}
	inline void BufferData(GLenum, GLsizeiptr, const void*, GLenum) {}
	inline void BufferSubData(GLenum, GLintptr, GLsizeiptr, const void*) {}
	inline void CopyBufferSubData(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr) {}
	inline void DeleteBuffers(GLsizei, const GLuint*) {}
	inline void ActiveTexture(GLenum) {}
	inline void GenFramebuffers(GLsizei, GLuint* fb) { if (fb) *fb = 1; }
	inline void DeleteFramebuffers(GLsizei, const GLuint*) {}
	inline void BindFramebuffer(GLenum, GLuint) {}
	inline void FramebufferTexture2D(GLenum, GLenum, GLenum, GLuint, GLint) {}
	inline GLenum CheckFramebufferStatus(GLenum) { return 0x8CD5; }
	inline void GenRenderbuffers(GLsizei, GLuint* rb) { if (rb) *rb = 1; }
	inline void DeleteRenderbuffers(GLsizei, const GLuint*) {}
	inline void BindRenderbuffer(GLenum, GLuint) {}
	inline void RenderbufferStorage(GLenum, GLenum, GLsizei, GLsizei) {}
	inline void FramebufferRenderbuffer(GLenum, GLenum, GLenum, GLuint) {}
	inline void GenerateMipmap(GLenum) {}
	inline void UniformMatrix4fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline void BlendColor(GLfloat, GLfloat, GLfloat, GLfloat) {}
	inline void BlendEquation(GLenum) {}
	inline void BlendEquationSeparate(GLenum, GLenum) {}
	inline void BlendFuncSeparate(GLenum, GLenum, GLenum, GLenum) {}
	inline void StencilFuncSeparate(GLenum, GLenum, GLint, GLuint) {}
	inline void StencilMaskSeparate(GLenum, GLuint) {}
	inline void StencilOpSeparate(GLenum, GLenum, GLenum, GLenum) {}
	inline void SampleCoverage(GLfloat, GLboolean) {}
	inline GLboolean IsFramebuffer(GLuint) { return 1; }
	inline GLboolean IsRenderbuffer(GLuint) { return 1; }
	inline void VertexAttrib4f(GLuint, GLfloat, GLfloat, GLfloat, GLfloat) {}
	inline void ProvokingVertex(GLenum) {}
	inline void Uniform1fv(GLint, GLsizei, const GLfloat*) {}
	inline void Uniform2fv(GLint, GLsizei, const GLfloat*) {}
	inline void Uniform3fv(GLint, GLsizei, const GLfloat*) {}
	inline void Uniform4fv(GLint, GLsizei, const GLfloat*) {}
	inline void Uniform1iv(GLint, GLsizei, const GLint*) {}
	inline void Uniform2iv(GLint, GLsizei, const GLint*) {}
	inline void Uniform3iv(GLint, GLsizei, const GLint*) {}
	inline void Uniform4iv(GLint, GLsizei, const GLint*) {}
	inline void UniformMatrix2fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline void UniformMatrix3fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline void GetUniformfv(GLuint, GLint, GLfloat* p) { if (p) *p = 0; }
	inline void GetUniformiv(GLuint, GLint, GLint* p) { if (p) *p = 0; }
	inline void GetActiveAttrib(GLuint, GLuint, GLsizei, GLsizei* length, GLint* size, GLenum* type, GLchar*) {
		if (length) *length = 0;
		if (size) *size = 1;
		if (type) *type = 0x1406;
	}
	inline void GetActiveUniform(GLuint, GLuint, GLsizei, GLsizei* length, GLint* size, GLenum* type, GLchar*) {
		if (length) *length = 0;
		if (size) *size = 1;
		if (type) *type = 0x1406;
	}
	inline void GetAttachedShaders(GLuint, GLsizei, GLsizei* count, GLuint*) { if (count) *count = 0; }
	inline void GetShaderSource(GLuint, GLsizei, GLsizei* length, GLchar*) { if (length) *length = 0; }
	inline void GetRenderbufferParameteriv(GLenum, GLenum, GLint* p) { if (p) *p = 0; }
	inline void GetFramebufferAttachmentParameteriv(GLenum, GLenum, GLenum, GLint* p) { if (p) *p = 0; }
	inline void DrawBuffers(GLsizei, const GLenum*) {}
	inline void VertexAttribIPointer(GLuint, GLint, GLenum, GLsizei, const void*) {}
	inline void VertexAttribDivisor(GLuint, GLuint) {}
	inline void DrawArraysInstanced(GLenum, GLint, GLsizei, GLsizei) {}
	inline void DrawElementsInstanced(GLenum, GLsizei, GLenum, const void*, GLsizei) {}
	inline void BindBufferRange(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr) {}
	inline GLuint GetUniformBlockIndex(GLuint, const GLchar*) { return 0; }
	inline void UniformBlockBinding(GLuint, GLuint, GLuint) {}
	inline void GenSamplers(GLsizei n, GLuint* s) { for (GLsizei i = 0; i < n; ++i) s[i] = 1; }
	inline void DeleteSamplers(GLsizei, const GLuint*) {}
	inline GLboolean IsSampler(GLuint) { return 1; }
	inline void BindSampler(GLuint, GLuint) {}
	inline void SamplerParameteri(GLuint, GLenum, GLint) {}
	inline void SamplerParameterf(GLuint, GLenum, GLfloat) {}
	inline void GenQueries(GLsizei n, GLuint* q) { for (GLsizei i = 0; i < n; ++i) q[i] = 1; }
	inline void DeleteQueries(GLsizei, const GLuint*) {}
	inline void TexImage3D(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) {}
	inline void TexSubImage3D(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*) {}
	inline void CopyTexSubImage3D(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) {}
	inline void Uniform1uiv(GLint, GLsizei, const GLuint*) {}
	inline void Uniform2uiv(GLint, GLsizei, const GLuint*) {}
	inline void Uniform3uiv(GLint, GLsizei, const GLuint*) {}
	inline void Uniform4uiv(GLint, GLsizei, const GLuint*) {}
	inline void UniformMatrix2x3fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline void UniformMatrix3x2fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline void UniformMatrix2x4fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline void UniformMatrix4x2fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline void UniformMatrix3x4fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline void UniformMatrix4x3fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline GLint GetFragDataLocation(GLuint, const GLchar*) { return 0; }
	inline void BlitFramebuffer(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum) {}
	inline void RenderbufferStorageMultisample(GLenum, GLsizei, GLenum, GLsizei, GLsizei) {}
	inline void FramebufferTextureLayer(GLenum, GLenum, GLuint, GLint, GLint) {}
	inline void ClearBufferiv(GLenum, GLint, const GLint*) {}
	inline void ClearBufferuiv(GLenum, GLint, const GLuint*) {}
	inline void ClearBufferfv(GLenum, GLint, const GLfloat*) {}
	inline void ClearBufferfi(GLenum, GLint, GLfloat, GLint) {}
	inline void VertexAttribI4i(GLuint, GLint, GLint, GLint, GLint) {}
	inline void VertexAttribI4ui(GLuint, GLuint, GLuint, GLuint, GLuint) {}
	inline void GetUniformuiv(GLuint, GLint, GLuint*) {}
	inline void BeginQuery(GLenum, GLuint) {}
	inline void EndQuery(GLenum) {}
	inline GLboolean IsQuery(GLuint) { return 1; }
	inline void GetQueryiv(GLenum, GLenum, GLint* p) { if (p) *p = 0; }
	inline void GetQueryObjectuiv(GLuint, GLenum, GLuint* p) { if (p) *p = 0; }
	inline void BeginTransformFeedback(GLenum) {}
	inline void EndTransformFeedback() {}
	inline void PauseTransformFeedback() {}
	inline void ResumeTransformFeedback() {}
	inline void BindTransformFeedback(GLenum, GLuint) {}
	inline void GenTransformFeedbacks(GLsizei n, GLuint* t) { for (GLsizei i = 0; i < n; ++i) t[i] = 1; }
	inline void DeleteTransformFeedbacks(GLsizei, const GLuint*) {}
	inline GLboolean IsTransformFeedback(GLuint) { return 1; }
	inline void TransformFeedbackVaryings(GLuint, GLsizei, const GLchar* const*, GLenum) {}
	inline void GetTransformFeedbackVarying(GLuint, GLuint, GLsizei, GLsizei* length, GLsizei* size, GLenum* type, GLchar*) {
		if (length) *length = 0;
		if (size) *size = 1;
		if (type) *type = 0x1406;
	}
	inline void GetActiveUniformBlockiv(GLuint, GLuint, GLenum, GLint* p) { if (p) *p = 0; }
	inline void GetActiveUniformBlockName(GLuint, GLuint, GLsizei, GLsizei* length, GLchar*) { if (length) *length = 0; }
	inline void GetUniformIndices(GLuint, GLsizei n, const GLchar* const*, GLuint* i) { for (GLsizei k = 0; k < n; ++k) i[k] = 0; }
	inline void GetActiveUniformsiv(GLuint, GLsizei n, const GLuint*, GLenum, GLint* p) { for (GLsizei k = 0; k < n; ++k) p[k] = 0; }
	inline void GetIntegeri_v(GLenum, GLuint, GLint* p) { if (p) *p = 0; }
	inline void GetSamplerParameteriv(GLuint, GLenum, GLint* p) { if (p) *p = 0; }
	inline void GetSamplerParameterfv(GLuint, GLenum, GLfloat* p) { if (p) *p = 0; }
	inline void GetBufferSubData(GLenum, GLintptr, GLsizeiptr, void*) {}
	inline void DispatchCompute(GLuint, GLuint, GLuint) {}
	inline void DispatchComputeIndirect(GLintptr) {}
	inline void MemoryBarrier(GLbitfield) {}
	inline void BindImageTexture(GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLenum) {}
	inline void ProgramParameteri(GLuint, GLenum, GLint) {}
	inline void GenProgramPipelines(GLsizei n, GLuint* p) { for (GLsizei i = 0; i < n; ++i) p[i] = 1; }
	inline void DeleteProgramPipelines(GLsizei, const GLuint*) {}
	inline void BindProgramPipeline(GLuint) {}
	inline void UseProgramStages(GLuint, GLbitfield, GLuint) {}
	inline void ActiveShaderProgram(GLuint, GLuint) {}
	inline void ValidateProgramPipeline(GLuint) {}
	inline GLboolean IsProgramPipeline(GLuint) { return 1; }
	inline void GetProgramPipelineiv(GLuint, GLenum, GLint* p) { if (p) *p = 0; }
	inline void GetProgramPipelineInfoLog(GLuint, GLsizei, GLsizei* length, GLchar*) { if (length) *length = 0; }
	inline void GetProgramInterfaceiv(GLuint, GLenum, GLenum, GLint* p) { if (p) *p = 0; }
	inline GLuint GetProgramResourceIndex(GLuint, GLenum, const GLchar*) { return 0; }
	inline void GetProgramResourceName(GLuint, GLenum, GLuint, GLsizei, GLsizei* length, GLchar*) { if (length) *length = 0; }
	inline void GetProgramResourceiv(GLuint, GLenum, GLuint, GLsizei, const GLenum*, GLsizei, GLsizei* length, GLint*) {
		if (length) *length = 0;
	}
	inline GLint GetProgramResourceLocation(GLuint, GLenum, const GLchar*) { return 0; }
	inline void TexStorage2DMultisample(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean) {}
	inline void SampleMaski(GLuint, GLbitfield) {}
	inline void GetMultisamplefv(GLenum, GLuint, GLfloat* v) { if (v) v[0] = v[1] = 0.5f; }
	inline void FramebufferParameteri(GLenum, GLenum, GLint) {}
	inline void CompressedTexImage2D(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*) {}
	inline void CompressedTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*) {}
	inline void CompressedTexImage3D(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void*) {}
	inline void CompressedTexSubImage3D(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*) {}
	inline void ClipControl(GLenum, GLenum) {}
	inline void GetFramebufferParameteriv(GLenum, GLenum, GLint* p) { if (p) *p = 0; }
}
#define QT_GL3(name) gl3test::name
namespace gl3 { inline bool has43() { return true; } }
#endif
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "gldecode.h"
#include "oglescmd.h"

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif
#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif
#ifndef GL_COPY_READ_BUFFER
#define GL_COPY_READ_BUFFER 0x8F36
#define GL_COPY_WRITE_BUFFER 0x8F37
#endif
#ifndef GL_UNIFORM_BUFFER
#define GL_UNIFORM_BUFFER 0x8A11
#endif
#ifndef GL_TEXTURE_CUBE_MAP
#define GL_TEXTURE_CUBE_MAP 0x8513
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#endif
#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D 0x806F
#endif
#ifndef GL_COLOR
#define GL_COLOR 0x1800
#endif
#ifndef GL_INT
#define GL_INT 0x1404
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BUFFER
#define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
#endif
#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_ATOMIC_COUNTER_BUFFER 0x92C0
#endif
#ifndef GL_STREAM_DRAW
#define GL_STREAM_DRAW 0x88E0
#endif
#ifndef GL_RG
#define GL_RG 0x8227
#define GL_R8 0x8229
#define GL_RG8 0x822B
#define GL_TEXTURE_SWIZZLE_R 0x8E42
#define GL_TEXTURE_SWIZZLE_G 0x8E43
#define GL_TEXTURE_SWIZZLE_B 0x8E44
#define GL_TEXTURE_SWIZZLE_A 0x8E45
#endif
#ifndef GL_RED
#define GL_RED 0x1903
#endif

namespace ogles2 {
	// A fake buffer: the bytes glBufferData was given, as they are (big
	// endian). 0 is never a valid name (GLES2's "no buffer").
	std::map<uint32_t, std::vector<uint8_t>> buffers;
	std::map<uint32_t, uint32_t> usages; // glBufferData's usage, for glGetBufferParameteriv
	uint32_t nextBuffer = 1;

	// What changed in a buffer, for the converted copies of it (Cached):
	// version is the last change's, whole the last glBufferData's (everything
	// new), changes the byte ranges of the glBufferSubData calls since, the
	// most recent last. Only the last few are kept; dropped is the version of
	// the newest one let go, and a copy older than it is converted again
	// whole.
	struct Change { uint64_t version; uint32_t begin, end; };
	struct Versions {
		uint64_t version, whole, dropped;
		std::vector<Change> changes;
		Versions() : version(1), whole(1), dropped(0) {}
	};
	const size_t maxChanges = 64;
	std::map<uint32_t, Versions> versions;
	uint64_t nextVersion = 2; // one sequence for all buffers, so a recreated name never matches an old copy

	void forgetBuffer(uint32_t name); // the converted copies of a deleted buffer
	// What a dispatch or draw wrote into the buffer brought back before its
	// bytes are read or written (written: replaced whole, nothing to read).
	void currentBytes(uint32_t name, bool replaced = false);

	const std::vector<uint8_t>* findBuffer(uint32_t name) {
		std::map<uint32_t, std::vector<uint8_t>>::const_iterator it = buffers.find(name);
		return it == buffers.end() ? 0 : &it->second;
	}

	uint32_t typeSize(uint32_t type) {
		switch (type) {
		case GL_UNSIGNED_BYTE: case GL_BYTE: return 1;
		case GL_UNSIGNED_SHORT: case GL_SHORT: return 2;
		}
		return 4; // GL_FLOAT, GL_FIXED, GL_INT, GL_UNSIGNED_INT
	}

	// Component k of element i of a vertex attribute, big-endian either in
	// Amiga memory (base from c.resolve) or in a fake buffer's own bytes
	// (base points at its first byte, already host-side); normalise: GLES2's
	// own rule for integer attributes (glVertexAttribPointer's normalized
	// flag), mapping the type's range to -1..1 or 0..1.
	float component(const uint8_t* base, uint32_t type, uint32_t stride, bool normalise, uint32_t i, uint32_t k) {
		const uint8_t* p = base + i * stride + k * typeSize(type);
		float raw;
		switch (type) {
		case GL_FLOAT: {
			uint32_t bits = (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
			memcpy(&raw, &bits, 4);
			return raw;
		}
		case GL_UNSIGNED_BYTE:
			return normalise ? p[0] / 255.0f : static_cast<float>(p[0]);
		case GL_BYTE: {
			int8_t v = static_cast<int8_t>(p[0]);
			return normalise ? (v == -128 ? -1.0f : v / 127.0f) : static_cast<float>(v);
		}
		case GL_UNSIGNED_SHORT: {
			uint16_t v = static_cast<uint16_t>((p[0] << 8) | p[1]);
			return normalise ? v / 65535.0f : static_cast<float>(v);
		}
		case GL_SHORT: {
			int16_t v = static_cast<int16_t>((p[0] << 8) | p[1]);
			return normalise ? (v == -32768 ? -1.0f : v / 32767.0f) : static_cast<float>(v);
		}
		case 0x140C: { // GL_FIXED: 16.16, never normalised
			int32_t v = static_cast<int32_t>((uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3]);
			return v / 65536.0f;
		}
		}
		return 0;
	}

	// A big-endian 32-bit word at p (host memory or a fake buffer's bytes).
	uint32_t word(const uint8_t* p) {
		return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
	}

	void putWord(uint8_t* p, uint32_t value) {
		uint32_t big = qt_swap32(value);
		memcpy(p, &big, 4);
	}

	void putFloat(uint8_t* p, float value) {
		uint32_t bits;
		memcpy(&bits, &value, 4);
		putWord(p, bits);
	}

	struct Attrib {
		bool enabled, integer; // integer: glVertexAttribIPointer's, handed over as ints (ES 3.0)
		uint32_t size, type, stride, bufferName, pointer; // pointer: offset into the buffer, or an Amiga address
		uint32_t divisor;      // glVertexAttribDivisor's (ES 3.0): 0, per vertex
		bool normalised;
	};

	Attrib readAttrib(const Command& c, int word) {
		Attrib a;
		a.enabled = (c.u(word) & 1) != 0;
		a.integer = (c.u(word) & 2) != 0;
		a.size = c.u(word + 1);
		a.type = c.u(word + 2);
		a.normalised = c.u(word + 3) != 0;
		a.stride = c.u(word + 4);
		a.bufferName = c.u(word + 5);
		a.pointer = c.u(word + 6);
		a.divisor = c.u(word + 7);
		return a;
	}

	uint32_t attribStride(const Attrib& a) { return a.stride ? a.stride : a.size * typeSize(a.type); }

	// An integer attribute's component (glVertexAttribIPointer), as the
	// 32-bit int the host's GL_INT/GL_UNSIGNED_INT arrays take: sign- or
	// zero-extended by type, never normalised.
	int32_t integerComponent(const uint8_t* base, uint32_t type, uint32_t stride, uint32_t i, uint32_t k) {
		const uint8_t* p = base + i * stride + k * typeSize(type);
		switch (type) {
		case GL_UNSIGNED_BYTE: return p[0];
		case GL_BYTE: return static_cast<int8_t>(p[0]);
		case GL_UNSIGNED_SHORT: return static_cast<uint16_t>((p[0] << 8) | p[1]);
		case GL_SHORT: return static_cast<int16_t>((p[0] << 8) | p[1]);
		}
		return static_cast<int32_t>(word(p)); // GL_INT, GL_UNSIGNED_INT
	}

	// Elements from to to - 1 of the attribute, from base (its element 0), as
	// host floats (or, integer, 32-bit ints), size components each, into
	// out[0...].
	void convert(const uint8_t* base, const Attrib& a, uint32_t from, uint32_t to, float* out) {
		uint32_t stride = attribStride(a);
		for (uint32_t i = from; i < to; ++i)
			for (uint32_t k = 0; k < a.size; ++k) {
				if (!a.integer) *out++ = component(base, a.type, stride, a.normalised, i, k);
				else {
					int32_t v = integerComponent(base, a.type, stride, i, k);
					memcpy(out++, &v, 4);
				}
			}
	}

	// GLSL ES 1.00 source as GLSL 3.30 core, which every OpenGL 3.3 core
	// context compiles (an unversioned shader is GLSL 1.10, which only some
	// drivers still accept in a core profile): the qualifiers and built-ins
	// 1.00 has and 3.30 core does not are #defines, gl_FragColor/gl_FragData
	// a declared output. #version and GL_OES_* #extension lines are blanked
	// (they would come after the prepended text; the OES extensions'
	// features -- dFdx and the rest -- are core in 3.30 anyway), keeping the
	// line numbers of the compiler's messages the application's own. GL_ES
	// and the other GL_ macros GLSL ES predefines cannot be defined here
	// (reserved names), so a shader's #ifdef GL_ES parts are left out -- in
	// practice precision statements, which 3.30 ignores anyway. A GLSL
	// ES identifier named like a 3.30 built-in function (a sampler called
	// "texture") does not compile; none is known in ogles2 programs.
	//
	// The application's own source is kept for glGetShaderSource and
	// GL_SHADER_SOURCE_LENGTH.
	// Keyed by the current host OpenGL context as well: shader names are its
	// own, and several programs' contexts may each have a shader 1.
#ifndef QT_TEST
	uintptr_t currentGL() { return reinterpret_cast<uintptr_t>(wglGetCurrentContext()); }
#else
	uintptr_t currentGL() { return 0; }
#endif
	// Contexts made to share objects (OGLES2_CCT_SHARE_WITH) share their
	// shaders too, so the sources are keyed by the group: a number each new
	// context gets, or the one of the context it shares with.
	std::map<uintptr_t, uintptr_t> groups; // by host context
	uintptr_t nextGroup = 1;
	uintptr_t currentGroup() {
		std::map<uintptr_t, uintptr_t>::const_iterator it = groups.find(currentGL());
		return it == groups.end() ? currentGL() : it->second;
	}
	typedef std::pair<uintptr_t, GLuint> ShaderKey;
	std::map<ShaderKey, std::string> sources;

	// A program's framebuffer 0 is its offscreen context's framebuffer object
	// (host/present.cpp), not the hidden host window's: binding 0 binds that
	// object, and queries answer 0 for it.
	GLuint hostFramebuffer(GLuint name) {
		return name ? name : qt_default_framebuffer();
	}
	GLint programFramebuffer(GLint name) {
		GLuint own = qt_default_framebuffer();
		return own && static_cast<GLuint>(name) == own ? 0 : name;
	}
	// Whether framebuffer 0 is bound to the target (GL_FRAMEBUFFER,
	// GL_DRAW_FRAMEBUFFER or GL_READ_FRAMEBUFFER) of an offscreen context.
	bool defaultBound(GLenum target) {
		GLuint own = qt_default_framebuffer();
		if (!own) return false;
		GLint bound = 0;
		QT_GL(GetIntegerv)(target == 0x8CA8 ? 0x8CAA : 0x8CA6, &bound); // READ_/DRAW_FRAMEBUFFER_BINDING
		return static_cast<GLuint>(bound) == own;
	}
	// The default framebuffer's buffer names as attachments of the object.
	GLenum defaultAttachment(GLenum attachment) {
		switch (attachment) {
		case GL_BACK: return 0x8CE0;   // GL_COLOR_ATTACHMENT0
		case 0x1801: return 0x8D00;    // GL_DEPTH: GL_DEPTH_ATTACHMENT
		case 0x1802: return 0x8D20;    // GL_STENCIL: GL_STENCIL_ATTACHMENT
		}
		return attachment;
	}

	// The GLSL ES version of a source whose first non-blank line is
	// "#version 300 es" or "#version 310 es" (300, 310), 0 for others.
	int es3Source(const std::string& source) {
		size_t at = source.find_first_not_of(" \t\r\n");
		if (at == std::string::npos || source.compare(at, 8, "#version") != 0) return 0;
		size_t end = source.find('\n', at);
		std::string line = source.substr(at, end == std::string::npos ? std::string::npos : end - at);
		if (line.find("es") == std::string::npos) return 0;
		if (line.find("310") != std::string::npos) return 310;
		return line.find("300") != std::string::npos ? 300 : 0;
	}

	std::string coreSource(const std::string& source, GLint type) {
		bool fragment = type == 0x8B30; // GL_FRAGMENT_SHADER
		// GLSL ES 3.00 (ogles3.library, docs/phase10-ogles3.md) is GLSL 3.30
		// core's near enough as it is: in/out, layout(location), uniform
		// blocks, texture(); precision statements compile there too. GLSL ES
		// 3.10 (compute shaders, storage buffers, images, binding layouts) is
		// GLSL 4.30's. Only the #version line changes, and a vertex shader
		// redeclares gl_PerVertex: desktop OpenGL wants that in a separable
		// program (NVIDIA's compiler refuses one without it; GLSL ES does not
		// have the block), and it does no harm in others. GLSL 3.30 has the
		// redeclaration only with ARB_separate_shader_objects, which a host
		// without OpenGL 4.3 may lack: then it is left out.
		if (int es = es3Source(source)) {
			std::string out = es == 310 ? "#version 430 core\n" : "#version 330 core\n";
			if (type == 0x8B31 && (es == 310 || gl3::has43())) { // GL_VERTEX_SHADER
				if (es != 310) out += "#extension GL_ARB_separate_shader_objects : enable\n";
				out += "out gl_PerVertex { vec4 gl_Position; float gl_PointSize; };\n";
			}
			out += "#line 1\n";
			size_t start = 0;
			while (start < source.size()) {
				size_t end = source.find('\n', start);
				if (end == std::string::npos) end = source.size();
				std::string line = source.substr(start, end - start);
				size_t first = line.find_first_not_of(" \t");
				if (!(first != std::string::npos && line.compare(first, 8, "#version") == 0)) out += line;
				out += '\n';
				start = end + 1;
			}
			return out;
		}
		std::string out =
			"#version 330 core\n"
			"#define texture2D texture\n"
			"#define texture2DProj textureProj\n"
			"#define texture2DLod textureLod\n"
			"#define texture2DProjLod textureProjLod\n"
			"#define textureCube texture\n"
			"#define textureCubeLod textureLod\n";
		if (!fragment) out += "#define attribute in\n#define varying out\n";
		else {
			out += "#define varying in\n";
			if (source.find("gl_FragData") != std::string::npos) out += "out vec4 qt_FragData[4];\n#define gl_FragData qt_FragData\n";
			else out += "out vec4 qt_FragColor;\n#define gl_FragColor qt_FragColor\n";
		}
		out += "#line 1\n";
		size_t start = 0;
		while (start < source.size()) {
			size_t end = source.find('\n', start);
			if (end == std::string::npos) end = source.size();
			std::string line = source.substr(start, end - start);
			size_t first = line.find_first_not_of(" \t");
			bool drop = first != std::string::npos && line[first] == '#' &&
				(line.find("version", first) != std::string::npos ||
				 (line.find("extension", first) != std::string::npos && line.find("GL_OES_", first) != std::string::npos));
			if (!drop) out += line;
			out += '\n';
			start = end + 1;
		}
		return out;
	}

	bool shaderSource(const Command& c) {
		GLuint shader = static_cast<GLuint>(c.u(1));
		const char* source = static_cast<const char*>(c.resolve(c.u(2)));
		if (!source) return true;
		GLint type = 0;
		QT_GL3(GetShaderiv)(shader, 0x8B4F, &type); // GL_SHADER_TYPE
		sources[ShaderKey(currentGroup(), shader)] = source;
		std::string core = coreSource(source, type);
		const char* text = core.c_str();
		QT_GL3(ShaderSource)(shader, 1, &text, 0);
		return true;
	}

	bool genBuffers(const Command& c) {
		uint32_t n = c.u(1);
		uint8_t* out = c.p(2);
		if (!out) return true;
		for (uint32_t i = 0; i < n; ++i) {
			uint32_t name = nextBuffer++;
			buffers[name]; // creates an empty one, as glGenBuffers names are valid to bind right away
			uint32_t big = (name >> 24) | ((name >> 8) & 0xFF00) | ((name << 8) & 0xFF0000) | (name << 24);
			memcpy(out + 4 * i, &big, 4);
		}
		return true;
	}

	bool deleteBuffers(const Command& c) {
		uint32_t n = c.u(1);
		const uint8_t* names = c.p(2);
		if (!names) return true;
		for (uint32_t i = 0; i < n; ++i) {
			uint32_t big;
			memcpy(&big, names + 4 * i, 4);
			buffers.erase(qt_swap32(big));
			usages.erase(qt_swap32(big));
			versions.erase(qt_swap32(big));
			forgetBuffer(qt_swap32(big));
		}
		return true;
	}

	// No bindings here: the 68k side keeps them per context and names the
	// buffer in each command (docs/multitasking.md), so several programs'
	// contexts never share one.
	bool bufferData(const Command& c) {
		uint32_t name = c.u(1), size = c.u(2);
		const uint8_t* data = c.p(3);
		if (!name) return true;
		currentBytes(name, data != 0);
		std::vector<uint8_t>& buffer = buffers[name];
		bool resized = buffer.size() != size;
		buffer.resize(size);
		if (data && size) memcpy(&buffer[0], data, size);
		usages[name] = c.u(4);
		Versions& v = versions[name];
		v.version = nextVersion++;
		// Without data (ioquake3 orphans its buffers that way before every
		// batch of glBufferSubData) the contents are undefined: a converted
		// copy of the same size stays as it is, and only what is written next
		// is converted.
		if (data || resized) {
			v.whole = v.version;
			v.dropped = 0;
			v.changes.clear();
		}
		return true;
	}

	std::vector<uint8_t>* buffer(uint32_t name) {
		std::map<uint32_t, std::vector<uint8_t>>::iterator it = buffers.find(name);
		return name && it != buffers.end() ? &it->second : 0;
	}

	bool bufferSubData(const Command& c) {
		std::vector<uint8_t>* b = buffer(c.u(1));
		uint32_t offset = c.u(2), size = c.u(3);
		const uint8_t* data = c.p(4);
		if (!b || !data || offset > b->size() || size > b->size() - offset) return true;
		if (!size) return true;
		currentBytes(c.u(1));
		memcpy(&(*b)[offset], data, size);
		Versions& v = versions[c.u(1)];
		v.version = nextVersion++;
		Change change = {v.version, offset, offset + size};
		v.changes.push_back(change);
		if (v.changes.size() > maxChanges) {
			v.dropped = v.changes.front().version;
			v.changes.erase(v.changes.begin());
		}
		return true;
	}

	bool readBuffer(const Command& c) {
		std::vector<uint8_t>* b = buffer(c.u(1));
		uint32_t offset = c.u(2), size = c.u(3);
		uint8_t* out = c.p(4);
		if (!b || !out || offset > b->size() || size > b->size() - offset) return true;
		currentBytes(c.u(1));
		if (size) memcpy(out, &(*b)[offset], size);
		return true;
	}

	// What a host OpenGL context keeps for drawing (one per program's
	// context, as GL objects are a context's own):
	// - Cached: an attribute layout of a buffer as floats, in a host buffer
	//   that has room for all of the buffer's elements. Elements are
	//   converted when a draw reads them and they are dirty: never converted
	//   yet, or changed since (glBufferData, glBufferSubData; version is the
	//   buffer version the dirty list goes up to). A static buffer is thus
	//   converted once, and a draw's cost goes with the elements it reads,
	//   not with the size of the buffer they are in.
	// - stream: a host buffer per attribute index for attributes in Amiga
	//   memory (converted at every draw, as the 68k may have changed them),
	//   and elements for the indices: reused, not made and deleted per draw.
	//   Plain client-side pointers (no GL_ARRAY_BUFFER bound) reliably hung
	//   this Intel driver's glDrawArrays under a core profile, even with a
	//   VAO bound and the pointer and size both correct; real buffer objects,
	//   the ordinary modern-GL way, do not.
	struct Layout {
		uint32_t buffer, offset, stride, type, size;
		bool normalised, integer;
		bool operator<(const Layout& o) const {
			if (buffer != o.buffer) return buffer < o.buffer;
			if (offset != o.offset) return offset < o.offset;
			if (stride != o.stride) return stride < o.stride;
			if (type != o.type) return type < o.type;
			if (size != o.size) return size < o.size;
			if (normalised != o.normalised) return normalised < o.normalised;
			return integer < o.integer;
		}
	};
	// A uniform buffer binding (glBindBufferBase/Range, ES 3.0): buffer name,
	// offset, size (0: all of it).
	struct UniformBinding {
		uint32_t buffer, offset, size;
	};
	typedef std::pair<uint32_t, uint32_t> Span; // elements first to second - 1
	struct Cached {
		GLuint gl;
		uint32_t elements;
		uint64_t version;
		std::vector<Span> dirty; // sorted, apart from each other
	};

	// Adds elements from to to - 1 to the dirty list.
	void markDirty(std::vector<Span>& dirty, uint32_t from, uint32_t to) {
		if (from >= to) return;
		std::vector<Span> out;
		size_t i = 0;
		for (; i < dirty.size() && dirty[i].second < from; ++i) out.push_back(dirty[i]);
		for (; i < dirty.size() && dirty[i].first <= to; ++i) {
			if (dirty[i].first < from) from = dirty[i].first;
			if (dirty[i].second > to) to = dirty[i].second;
		}
		out.push_back(Span(from, to));
		for (; i < dirty.size(); ++i) out.push_back(dirty[i]);
		dirty.swap(out);
	}
	struct PerGL {
		std::map<Layout, Cached> cached;
		GLuint stream[QT_OGLES2_MAX_ATTRIBS];
		GLuint elements;
		GLuint staging; // see update()
		std::vector<GLuint> orphans; // copies of deleted buffers, deleted when this context is current
		std::map<uint32_t, UniformBinding> uniformBuffers; // by binding index
		std::map<uint32_t, UniformBinding> feedbackBuffers; // transform feedback buffers, by binding index
		std::map<uint32_t, UniformBinding> storageBuffers;  // shader storage buffers (ES 3.1), by binding index
		std::map<uint32_t, UniformBinding> atomicBuffers;   // atomic counter buffers (ES 3.1), by binding index
		// Fake buffers whose word copy here (wordsOf) a dispatch or draw may
		// have written since: newer than their bytes (syncBack).
		std::set<uint32_t> gpuWritten;
		std::map<uint32_t, GLuint> captures; // a fake buffer's host buffer transform feedback writes into
		PerGL() : elements(0), staging(0) { memset(stream, 0, sizeof(stream)); }
	};
	std::map<uintptr_t, PerGL> perGL;
	std::vector<float> scratch;

	// Writes size bytes at offset into target, a buffer earlier draws may
	// still be reading on the GPU: through a fresh staging buffer and a copy
	// on the GPU, as glBufferSubData into a buffer in use can make the CPU
	// wait for those draws (ioquake3 rewrites one buffer before each batch).
	void update(PerGL& gl, GLuint target, size_t offset, size_t size, const void* data) {
		if (!gl.staging) QT_GL3(GenBuffers)(1, &gl.staging);
		QT_GL3(BindBuffer)(GL_COPY_READ_BUFFER, gl.staging);
		QT_GL3(BufferData)(GL_COPY_READ_BUFFER, static_cast<GLsizeiptr>(size), data, GL_STREAM_DRAW);
		QT_GL3(BindBuffer)(GL_COPY_WRITE_BUFFER, target);
		QT_GL3(CopyBufferSubData)(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size));
	}

	// The host buffer holding the attribute's floats, with elements low to
	// high up to date; 0 if the attribute does not fit into the buffer, or
	// high is past its end.
	void syncBack(PerGL& gl, uint32_t name);
	void swapUnits(uint8_t* p, size_t size, uint32_t unit);

	GLuint cachedArray(PerGL& gl, const Attrib& a, uint32_t low, uint32_t high, bool words = false) {
		// What the GPU wrote comes back into the bytes first (not for the
		// word copy itself, which is what it wrote into).
		if (!words) syncBack(gl, a.bufferName);
		std::map<uint32_t, std::vector<uint8_t>>::const_iterator b = buffers.find(a.bufferName);
		if (b == buffers.end()) return 0;
		const std::vector<uint8_t>& bytes = b->second;
		uint32_t stride = attribStride(a), last = a.size * typeSize(a.type);
		if (a.pointer > bytes.size() || bytes.size() - a.pointer < last) return 0;
		uint32_t elements = (static_cast<uint32_t>(bytes.size()) - a.pointer - last) / stride + 1;
		if (high >= elements) return 0;
		const Versions& v = versions[a.bufferName];
		Layout key = {a.bufferName, a.pointer, stride, a.type, a.size, a.normalised, a.integer};
		Cached& entry = gl.cached[key];
		size_t row = a.size * sizeof(float);
		if (!entry.gl) QT_GL3(GenBuffers)(1, &entry.gl);
		QT_GL3(BindBuffer)(GL_ARRAY_BUFFER, entry.gl);
		if (entry.version != v.version || entry.elements != elements) {
			if (entry.elements != elements || entry.version < v.whole || entry.version < v.dropped) {
				// New, resized, or changed in ways no longer known: all dirty.
				QT_GL3(BufferData)(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(elements * row), 0, GL_DYNAMIC_DRAW);
				entry.elements = elements;
				entry.dirty.assign(1, Span(0, elements));
			}
			else {
				// The elements whose bytes (pointer + i * stride, last bytes
				// long) overlap a range written since.
				for (size_t i = 0; i < v.changes.size(); ++i) {
					const Change& change = v.changes[i];
					if (change.version <= entry.version || change.end <= a.pointer) continue;
					uint32_t from = change.begin < a.pointer + last ? 0 : (change.begin - a.pointer - last) / stride + 1;
					uint32_t to = (change.end - a.pointer - 1) / stride + 1;
					markDirty(entry.dirty, from, to < elements ? to : elements);
				}
			}
			entry.version = v.version;
		}
		// Convert what the draw reads of the dirty elements.
		const uint8_t* base = &bytes[a.pointer];
		std::vector<Span> keep;
		for (size_t i = 0; i < entry.dirty.size(); ++i) {
			Span d = entry.dirty[i];
			uint32_t from = d.first > low ? d.first : low, to = d.second < high + 1 ? d.second : high + 1;
			if (from >= to) {
				keep.push_back(d);
				continue;
			}
			scratch.resize(static_cast<size_t>(to - from) * a.size);
			convert(base, a, from, to, &scratch[0]);
			update(gl, entry.gl, from * row, (to - from) * row, &scratch[0]);
			if (d.first < from) keep.push_back(Span(d.first, from));
			if (to < d.second) keep.push_back(Span(to, d.second));
		}
		entry.dirty.swap(keep);
		return entry.gl;
	}

	// An attribute in Amiga memory: elements low to high converted into the
	// index's stream buffer, at their place (the buffer holds high + 1
	// elements; the ones below low are left undefined, nothing reads them).
	bool streamArray(const Command& c, PerGL& gl, int index, const Attrib& a, uint32_t low, uint32_t high) {
		if (!a.pointer) return false;
		const uint8_t* base = static_cast<const uint8_t*>(c.resolve(a.pointer));
		if (!base) return false;
		size_t row = a.size * sizeof(float);
		scratch.resize(static_cast<size_t>(high - low + 1) * a.size);
		convert(base, a, low, high + 1, &scratch[0]);
		if (!gl.stream[index]) QT_GL3(GenBuffers)(1, &gl.stream[index]);
		QT_GL3(BindBuffer)(GL_ARRAY_BUFFER, gl.stream[index]);
		QT_GL3(BufferData)(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>((high + 1) * row), 0, GL_STREAM_DRAW);
		QT_GL3(BufferSubData)(GL_ARRAY_BUFFER, static_cast<GLintptr>(low * row), static_cast<GLsizeiptr>((high - low + 1) * row), &scratch[0]);
		return true;
	}

	// A fake buffer as 32-bit words (cachedArray's attribute for it, as
	// unsigned ints, so the bits stay as they are).
	Attrib wordsOf(uint32_t name) {
		Attrib words = {true, true, 1, 0x1405, 4, name, 0, 0, false}; // GL_UNSIGNED_INT
		return words;
	}

	Layout wordsKey(uint32_t name) {
		Layout key = {name, 0, 4, 0x1405, 1, false, true};
		return key;
	}

	void recordChange(uint32_t name, uint32_t begin, uint32_t end) {
		Versions& v = versions[name];
		v.version = nextVersion++;
		Change change = {v.version, begin, end};
		v.changes.push_back(change);
		if (v.changes.size() > maxChanges) {
			v.dropped = v.changes.front().version;
			v.changes.erase(v.changes.begin());
		}
	}

	// A fake buffer a dispatch or draw may have written (gpuWritten): its
	// word copy read back into its bytes, every word swapped, recorded as a
	// change, with the word copy marked up to date with it.
	void syncBack(PerGL& gl, uint32_t name) {
		std::set<uint32_t>::iterator written = gl.gpuWritten.find(name);
		if (written == gl.gpuWritten.end()) return;
		gl.gpuWritten.erase(written);
		std::vector<uint8_t>* b = buffer(name);
		std::map<Layout, Cached>::iterator entry = gl.cached.find(wordsKey(name));
		if (!b || b->size() < 4 || entry == gl.cached.end() || !entry->second.gl) return;
		uint32_t size = static_cast<uint32_t>(b->size()) & ~3u;
		QT_GL3(BindBuffer)(GL_COPY_READ_BUFFER, entry->second.gl);
		QT_GL3(GetBufferSubData)(GL_COPY_READ_BUFFER, 0, static_cast<GLsizeiptr>(size), &(*b)[0]);
		swapUnits(&(*b)[0], size, 4);
		recordChange(name, 0, size);
		entry->second.version = versions[name].version;
	}

	// The uniform, storage and atomic counter buffers bound by
	// glBindBufferBase/Range: their 32-bit words (std140 and std430 data
	// have no other kind) in host order, in the word copy, bound before each
	// draw and dispatch; a copy the GPU wrote is bound as it is.
	void bindBuffers(PerGL& gl, std::map<uint32_t, UniformBinding>& bindings, GLenum target) {
		for (std::map<uint32_t, UniformBinding>::const_iterator u = bindings.begin(); u != bindings.end(); ++u) {
			std::map<uint32_t, std::vector<uint8_t>>::const_iterator b = buffers.find(u->second.buffer);
			if (b == buffers.end() || b->second.size() < 4) continue;
			GLuint copy = 0;
			if (gl.gpuWritten.count(u->second.buffer)) copy = gl.cached[wordsKey(u->second.buffer)].gl;
			else copy = cachedArray(gl, wordsOf(u->second.buffer), 0, static_cast<uint32_t>(b->second.size() / 4) - 1, true);
			if (!copy) continue;
			uint32_t size = u->second.size ? u->second.size : static_cast<uint32_t>(b->second.size()) - u->second.offset;
			QT_GL3(BindBufferRange)(target, u->first, copy, static_cast<GLintptr>(u->second.offset), static_cast<GLsizeiptr>(size));
		}
	}

	void bindUniformBuffers(PerGL& gl) {
		bindBuffers(gl, gl.uniformBuffers, GL_UNIFORM_BUFFER);
		if (!gl3::has43()) return;
		bindBuffers(gl, gl.storageBuffers, GL_SHADER_STORAGE_BUFFER);
		bindBuffers(gl, gl.atomicBuffers, GL_ATOMIC_COUNTER_BUFFER);
	}

	// After a dispatch or draw: the storage and atomic counter buffers bound
	// may have been written.
	void markWritten(PerGL& gl) {
		for (std::map<uint32_t, UniformBinding>::const_iterator u = gl.storageBuffers.begin(); u != gl.storageBuffers.end(); ++u)
			gl.gpuWritten.insert(u->second.buffer);
		for (std::map<uint32_t, UniformBinding>::const_iterator u = gl.atomicBuffers.begin(); u != gl.atomicBuffers.end(); ++u)
			gl.gpuWritten.insert(u->second.buffer);
	}

	bool bindBufferRange(const Command& c) {
		PerGL& gl = perGL[currentGL()];
		std::map<uint32_t, UniformBinding>* which = 0;
		switch (c.u(1)) {
		case GL_UNIFORM_BUFFER: which = &gl.uniformBuffers; break;
		case GL_TRANSFORM_FEEDBACK_BUFFER: which = &gl.feedbackBuffers; break;
		case GL_SHADER_STORAGE_BUFFER: which = &gl.storageBuffers; break;
		case GL_ATOMIC_COUNTER_BUFFER: which = &gl.atomicBuffers; break;
		default: return true;
		}
		std::map<uint32_t, UniformBinding>& bindings = *which;
		UniformBinding binding = {c.u(3), c.u(4), c.u(5)};
		if (binding.buffer) bindings[c.u(2)] = binding;
		else bindings.erase(c.u(2));
		return true;
	}

	void swapUnits(uint8_t* p, size_t size, uint32_t unit);

	// Transform feedback (ES 3.0): it writes host-order words into a host
	// buffer, one per fake buffer bound, filled with the fake buffer's words
	// first (so what is not written stays as it was); glEndTransformFeedback
	// copies the bound ranges back into the fake buffers, every word swapped
	// (the outputs are all 32-bit), as changes the attribute copies follow.
	void beginTransformFeedback(GLenum mode) {
		PerGL& gl = perGL[currentGL()];
		for (std::map<uint32_t, UniformBinding>::const_iterator f = gl.feedbackBuffers.begin(); f != gl.feedbackBuffers.end(); ++f) {
			syncBack(gl, f->second.buffer);
			std::vector<uint8_t>* b = buffer(f->second.buffer);
			if (!b || b->empty()) continue;
			GLuint& capture = gl.captures[f->second.buffer];
			if (!capture) QT_GL3(GenBuffers)(1, &capture);
			std::vector<uint8_t> words(*b);
			swapUnits(&words[0], words.size() & ~static_cast<size_t>(3), 4);
			QT_GL3(BindBuffer)(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
			QT_GL3(BufferData)(GL_TRANSFORM_FEEDBACK_BUFFER, static_cast<GLsizeiptr>(words.size()), &words[0], GL_DYNAMIC_DRAW);
			uint32_t size = f->second.size ? f->second.size : static_cast<uint32_t>(b->size()) - f->second.offset;
			QT_GL3(BindBufferRange)(GL_TRANSFORM_FEEDBACK_BUFFER, f->first, capture, static_cast<GLintptr>(f->second.offset),
				static_cast<GLsizeiptr>(size));
		}
		QT_GL3(BeginTransformFeedback)(mode);
	}

	void endTransformFeedback() {
		QT_GL3(EndTransformFeedback)();
		PerGL& gl = perGL[currentGL()];
		for (std::map<uint32_t, UniformBinding>::const_iterator f = gl.feedbackBuffers.begin(); f != gl.feedbackBuffers.end(); ++f) {
			std::vector<uint8_t>* b = buffer(f->second.buffer);
			std::map<uint32_t, GLuint>::const_iterator capture = gl.captures.find(f->second.buffer);
			if (!b || capture == gl.captures.end() || f->second.offset >= b->size()) continue;
			uint32_t size = f->second.size ? f->second.size : static_cast<uint32_t>(b->size()) - f->second.offset;
			if (size > b->size() - f->second.offset) size = static_cast<uint32_t>(b->size()) - f->second.offset;
			QT_GL3(BindBuffer)(GL_TRANSFORM_FEEDBACK_BUFFER, capture->second);
			QT_GL3(GetBufferSubData)(GL_TRANSFORM_FEEDBACK_BUFFER, static_cast<GLintptr>(f->second.offset), static_cast<GLsizeiptr>(size),
				&(*b)[f->second.offset]);
			swapUnits(&(*b)[f->second.offset], size & ~3u, 4);
			Versions& v = versions[f->second.buffer];
			v.version = nextVersion++;
			Change change = {v.version, f->second.offset, f->second.offset + size};
			v.changes.push_back(change);
			if (v.changes.size() > maxChanges) {
				v.dropped = v.changes.front().version;
				v.changes.erase(v.changes.begin());
			}
		}
	}

	// glGetIntegeri_v: the buffer bindings are QuarkTex's own (the fake
	// buffers' names and their ranges), the rest the host's.
	GLint indexedInteger(GLenum target, GLuint index) {
		PerGL& gl = perGL[currentGL()];
		std::map<uint32_t, UniformBinding>* bindings = 0;
		switch (target) {
		case 0x8A28: case 0x8A29: case 0x8A2A: bindings = &gl.uniformBuffers; break; // UNIFORM_BUFFER_BINDING/_START/_SIZE
		case 0x8C8F: case 0x8C84: case 0x8C85: bindings = &gl.feedbackBuffers; break; // TRANSFORM_FEEDBACK_BUFFER_BINDING/_START/_SIZE
		case 0x90D3: case 0x90D4: case 0x90D5: bindings = &gl.storageBuffers; break; // SHADER_STORAGE_BUFFER_BINDING/_START/_SIZE
		case 0x92C1: case 0x92C2: case 0x92C3: bindings = &gl.atomicBuffers; break; // ATOMIC_COUNTER_BUFFER_BINDING/_START/_SIZE
		}
		if (bindings) {
			std::map<uint32_t, UniformBinding>::const_iterator b = bindings->find(index);
			if (b == bindings->end()) return 0;
			if (target == 0x8A28 || target == 0x8C8F || target == 0x90D3 || target == 0x92C1) return static_cast<GLint>(b->second.buffer);
			if (target == 0x8A29 || target == 0x8C84 || target == 0x90D4 || target == 0x92C2) return static_cast<GLint>(b->second.offset);
			return static_cast<GLint>(b->second.size);
		}
		GLint value = 0;
		QT_GL3(GetIntegeri_v)(target, index, &value);
		return value;
	}

	// glCopyBufferSubData between two of the fake buffers, recorded as a
	// change of the one written (as glBufferSubData is).
	bool copyBufferSubData(const Command& c) {
		PerGL& gl = perGL[currentGL()];
		syncBack(gl, c.u(1));
		syncBack(gl, c.u(2));
		std::vector<uint8_t>* from = buffer(c.u(1));
		std::vector<uint8_t>* into = buffer(c.u(2));
		uint32_t readOffset = c.u(3), writeOffset = c.u(4), size = c.u(5);
		if (!from || !into || !size || readOffset > from->size() || size > from->size() - readOffset
				|| writeOffset > into->size() || size > into->size() - writeOffset) return true;
		memmove(&(*into)[writeOffset], &(*from)[readOffset], size);
		recordChange(c.u(2), writeOffset, writeOffset + size);
		return true;
	}

	bool draw(const Command& c) {
		GLenum mode = static_cast<GLenum>(c.u(1));
		uint32_t first = c.u(2), count = c.u(3), indexType = c.u(4), indexBuffer = c.u(5), indexPointer = c.u(6);
		Attrib attribs[QT_OGLES2_MAX_ATTRIBS];
		uint32_t instances = c.u(7);
		for (int i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) attribs[i] = readAttrib(c, 8 + 8 * i);
		if (!count) return true;
		PerGL& gl = perGL[currentGL()];
		bindUniformBuffers(gl);
		if (!gl.orphans.empty()) {
			QT_GL3(DeleteBuffers)(static_cast<GLsizei>(gl.orphans.size()), &gl.orphans[0]);
			gl.orphans.clear();
		}

		// The vertices the draw reads: first to first + count - 1, or the
		// lowest to the highest index (big-endian, converted here, not with
		// everyone else's component()).
		std::vector<uint32_t> indices;
		uint32_t low = first, high = first + count - 1;
		if (indexType) {
			const uint8_t* base;
			uint32_t size = typeSize(indexType);
			if (indexBuffer) {
				syncBack(gl, indexBuffer);
				const std::vector<uint8_t>* buffer = findBuffer(indexBuffer);
				if (!buffer || indexPointer > buffer->size() || count * size > buffer->size() - indexPointer) return true;
				base = &(*buffer)[indexPointer];
			}
			else {
				if (!indexPointer) return true;
				base = static_cast<const uint8_t*>(c.resolve(indexPointer));
				if (!base) return true;
			}
			// first: the base vertex (glDrawElementsBaseVertexOES), 0 otherwise.
			int32_t baseVertex = static_cast<int32_t>(first);
			indices.resize(count);
			low = 0xFFFFFFFFu;
			high = 0;
			for (uint32_t n = 0; n < count; ++n) {
				const uint8_t* q = base + n * size;
				int64_t v = size == 1 ? q[0] : size == 2 ? (uint32_t(q[0]) << 8) | q[1] : word(q);
				v += baseVertex;
				if (v < 0) return true;
				indices[n] = static_cast<uint32_t>(v);
				if (indices[n] < low) low = indices[n];
				if (indices[n] > high) high = indices[n];
			}
		}

		bool on[QT_OGLES2_MAX_ATTRIBS];
		for (int i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) {
			const Attrib& a = attribs[i];
			// A per-instance attribute (divisor) reads elements 0 to
			// (instances - 1) / divisor, whatever the indices.
			uint32_t from = low, to = high;
			if (a.divisor) {
				from = 0;
				to = ((instances ? instances : 1) - 1) / a.divisor;
			}
			on[i] = a.enabled && (a.bufferName ? cachedArray(gl, a, from, to) != 0 : streamArray(c, gl, i, a, from, to));
			if (!on[i]) {
				QT_GL3(DisableVertexAttribArray)(static_cast<GLuint>(i));
				continue;
			}
			// The buffer the attribute's floats (or ints) are in is bound now.
			QT_GL3(EnableVertexAttribArray)(static_cast<GLuint>(i));
			if (a.integer) QT_GL3(VertexAttribIPointer)(static_cast<GLuint>(i), static_cast<GLint>(a.size), GL_INT, 0, 0);
			else QT_GL3(VertexAttribPointer)(static_cast<GLuint>(i), static_cast<GLint>(a.size), GL_FLOAT, GL_FALSE, 0, 0);
			QT_GL3(VertexAttribDivisor)(static_cast<GLuint>(i), a.divisor);
		}
		GLsizei n = static_cast<GLsizei>(instances ? instances : 1);
		if (indexType) {
			if (!gl.elements) QT_GL3(GenBuffers)(1, &gl.elements);
			QT_GL3(BindBuffer)(GL_ELEMENT_ARRAY_BUFFER, gl.elements);
			QT_GL3(BufferData)(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(count * sizeof(uint32_t)), &indices[0], GL_STREAM_DRAW);
			if (instances) QT_GL3(DrawElementsInstanced)(mode, static_cast<GLsizei>(count), GL_UNSIGNED_INT, 0, n);
			else QT_GL(DrawElements)(mode, static_cast<GLsizei>(count), GL_UNSIGNED_INT, 0);
		}
		else if (instances) QT_GL3(DrawArraysInstanced)(mode, static_cast<GLint>(first), static_cast<GLsizei>(count), n);
		else QT_GL(DrawArrays)(mode, static_cast<GLint>(first), static_cast<GLsizei>(count));
		markWritten(gl);
		for (int i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i)
			if (on[i]) QT_GL3(DisableVertexAttribArray)(static_cast<GLuint>(i));
		return true;
	}

	void currentBytes(uint32_t name, bool replaced) {
		PerGL& gl = perGL[currentGL()];
		if (replaced) gl.gpuWritten.erase(name);
		else syncBack(gl, name);
	}

	void forgetBuffer(uint32_t name) {
		uintptr_t current = currentGL();
		for (std::map<uintptr_t, PerGL>::iterator g = perGL.begin(); g != perGL.end(); ++g) {
			g->second.gpuWritten.erase(name);
			std::map<uint32_t, GLuint>::iterator capture = g->second.captures.find(name);
			if (capture != g->second.captures.end()) {
				if (g->first == current) QT_GL3(DeleteBuffers)(1, &capture->second);
				else g->second.orphans.push_back(capture->second);
				g->second.captures.erase(capture);
			}
			std::map<Layout, Cached>& cached = g->second.cached;
			for (std::map<Layout, Cached>::iterator i = cached.begin(); i != cached.end();) {
				if (i->first.buffer != name) {
					++i;
					continue;
				}
				if (i->second.gl) {
					if (g->first == current) QT_GL3(DeleteBuffers)(1, &i->second.gl);
					else g->second.orphans.push_back(i->second.gl);
				}
				cached.erase(i++);
			}
		}
	}

	// The current context's converted buffers and stream buffers, and the
	// shader sources kept for it if no context of its group is left.
	void forgetContext() {
		uintptr_t key = currentGL(), group = currentGroup();
		bool last = true;
		groups.erase(key);
		for (std::map<uintptr_t, uintptr_t>::const_iterator g = groups.begin(); g != groups.end(); ++g)
			if (g->second == group) last = false;
		std::map<uintptr_t, PerGL>::iterator it = perGL.find(key);
		if (it != perGL.end()) {
			PerGL& gl = it->second;
			for (std::map<Layout, Cached>::iterator i = gl.cached.begin(); i != gl.cached.end(); ++i)
				if (i->second.gl) QT_GL3(DeleteBuffers)(1, &i->second.gl);
			for (int i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i)
				if (gl.stream[i]) QT_GL3(DeleteBuffers)(1, &gl.stream[i]);
			if (gl.elements) QT_GL3(DeleteBuffers)(1, &gl.elements);
			if (gl.staging) QT_GL3(DeleteBuffers)(1, &gl.staging);
			for (std::map<uint32_t, GLuint>::iterator i = gl.captures.begin(); i != gl.captures.end(); ++i)
				QT_GL3(DeleteBuffers)(1, &i->second);
			if (!gl.orphans.empty()) QT_GL3(DeleteBuffers)(static_cast<GLsizei>(gl.orphans.size()), &gl.orphans[0]);
			perGL.erase(it);
		}
		for (std::map<ShaderKey, std::string>::iterator i = sources.begin(); last && i != sources.end();) {
			if (i->first.first == group) sources.erase(i++);
			else ++i;
		}
	}

	int32_t simple(const Command& c) {
		uint32_t w0 = c.u(2), w1 = c.u(3);
		switch (c.u(1)) {
		case QT_OGLES2_S_CreateShader: return static_cast<int32_t>(QT_GL3(CreateShader)(static_cast<GLenum>(w0)));
		case QT_OGLES2_S_DeleteShader:
			QT_GL3(DeleteShader)(static_cast<GLuint>(w0));
			sources.erase(ShaderKey(currentGroup(), static_cast<GLuint>(w0)));
			return 0;
		case QT_OGLES2_S_CompileShader: QT_GL3(CompileShader)(static_cast<GLuint>(w0)); return 0;
		case QT_OGLES2_S_AttachShader: QT_GL3(AttachShader)(static_cast<GLuint>(w0), static_cast<GLuint>(w1)); return 0;
		case QT_OGLES2_S_DetachShader: QT_GL3(DetachShader)(static_cast<GLuint>(w0), static_cast<GLuint>(w1)); return 0;
		case QT_OGLES2_S_CreateProgram: return static_cast<int32_t>(QT_GL3(CreateProgram)());
		case QT_OGLES2_S_DeleteProgram: QT_GL3(DeleteProgram)(static_cast<GLuint>(w0)); return 0;
		case QT_OGLES2_S_LinkProgram: QT_GL3(LinkProgram)(static_cast<GLuint>(w0)); return 0;
		case QT_OGLES2_S_UseProgram: QT_GL3(UseProgram)(static_cast<GLuint>(w0)); return 0;
		case QT_OGLES2_S_ValidateProgram: QT_GL3(ValidateProgram)(static_cast<GLuint>(w0)); return 0;
		case QT_OGLES2_S_IsShader: return QT_GL3(IsShader)(static_cast<GLuint>(w0)) ? 1 : 0;
		case QT_OGLES2_S_IsProgram: return QT_GL3(IsProgram)(static_cast<GLuint>(w0)) ? 1 : 0;
		}
		return 0;
	}

	int32_t name(const Command& c) {
		uint32_t program = c.u(2), index = c.u(3);
		const char* text = static_cast<const char*>(c.resolve(c.u(4)));
		static const char empty[] = "";
		if (!text) text = empty;
		switch (c.u(1)) {
		case QT_OGLES2_N_BindAttribLocation:
			QT_GL3(BindAttribLocation)(static_cast<GLuint>(program), static_cast<GLuint>(index), text);
			return 0;
		case QT_OGLES2_N_GetAttribLocation: return QT_GL3(GetAttribLocation)(static_cast<GLuint>(program), text);
		case QT_OGLES2_N_GetUniformLocation: return QT_GL3(GetUniformLocation)(static_cast<GLuint>(program), text);
		case QT_OGLES2_N_GetUniformBlockIndex:
			return static_cast<int32_t>(QT_GL3(GetUniformBlockIndex)(static_cast<GLuint>(program), text));
		case QT_OGLES2_N_GetFragDataLocation: return QT_GL3(GetFragDataLocation)(static_cast<GLuint>(program), text);
		}
		return 0;
	}

	int32_t uniform(const Command& c) {
		GLint location = static_cast<GLint>(c.u(2));
		switch (c.u(1)) {
		case QT_OGLES2_U_1F: QT_GL3(Uniform1f)(location, c.f(3)); break;
		case QT_OGLES2_U_2F: QT_GL3(Uniform2f)(location, c.f(3), c.f(4)); break;
		case QT_OGLES2_U_3F: QT_GL3(Uniform3f)(location, c.f(3), c.f(4), c.f(5)); break;
		case QT_OGLES2_U_4F: QT_GL3(Uniform4f)(location, c.f(3), c.f(4), c.f(5), c.f(6)); break;
		case QT_OGLES2_U_1I: QT_GL3(Uniform1i)(location, static_cast<GLint>(c.u(3))); break;
		case QT_OGLES2_U_2I: QT_GL3(Uniform2i)(location, static_cast<GLint>(c.u(3)), static_cast<GLint>(c.u(4))); break;
		case QT_OGLES2_U_3I:
			QT_GL3(Uniform3i)(location, static_cast<GLint>(c.u(3)), static_cast<GLint>(c.u(4)), static_cast<GLint>(c.u(5)));
			break;
		case QT_OGLES2_U_4I:
			QT_GL3(Uniform4i)(location, static_cast<GLint>(c.u(3)), static_cast<GLint>(c.u(4)), static_cast<GLint>(c.u(5)),
				static_cast<GLint>(c.u(6)));
			break;
		case QT_OGLES2_U_1UI: case QT_OGLES2_U_2UI: case QT_OGLES2_U_3UI: case QT_OGLES2_U_4UI: {
			GLuint v[4] = {c.u(3), c.u(4), c.u(5), c.u(6)};
			switch (c.u(1)) {
			case QT_OGLES2_U_1UI: QT_GL3(Uniform1uiv)(location, 1, v); break;
			case QT_OGLES2_U_2UI: QT_GL3(Uniform2uiv)(location, 1, v); break;
			case QT_OGLES2_U_3UI: QT_GL3(Uniform3uiv)(location, 1, v); break;
			default: QT_GL3(Uniform4uiv)(location, 1, v);
			}
			break;
		}
		}
		return 0;
	}

	int32_t getIv(const Command& c) {
		GLuint id = static_cast<GLuint>(c.u(2));
		GLenum pname = static_cast<GLenum>(c.u(3));
		uint8_t* out = c.p(4);
		GLint value = 0;
		if (c.u(1) == QT_OGLES2_G_Shader) {
			std::map<ShaderKey, std::string>::const_iterator it = sources.find(ShaderKey(currentGroup(), id));
			// The application's own source, not coreSource's.
			if (pname == 0x8B88 && it != sources.end()) value = static_cast<GLint>(it->second.size() + 1); // GL_SHADER_SOURCE_LENGTH
			else QT_GL3(GetShaderiv)(id, pname, &value);
		}
		else QT_GL3(GetProgramiv)(id, pname, &value);
		if (out) {
			uint32_t big = qt_swap32(static_cast<uint32_t>(value));
			memcpy(out, &big, 4);
		}
		return 0;
	}

	int32_t getLog(const Command& c) {
		GLuint id = static_cast<GLuint>(c.u(2));
		GLsizei bufSize = static_cast<GLsizei>(c.u(3));
		uint8_t* lengthOut = c.p(4);
		char* textOut = c.p(5);
		GLsizei length = 0;
		if (textOut && bufSize > 0) {
			if (c.u(1) == QT_OGLES2_G_Shader) QT_GL3(GetShaderInfoLog)(id, bufSize, &length, textOut);
			else if (c.u(1) == QT_OGLES2_G_ShaderSource) {
				std::map<ShaderKey, std::string>::const_iterator it = sources.find(ShaderKey(currentGroup(), id));
				if (it != sources.end()) {
					length = static_cast<GLsizei>(it->second.size() < static_cast<size_t>(bufSize - 1) ? it->second.size() : bufSize - 1);
					memcpy(textOut, it->second.data(), length);
					textOut[length] = 0;
				}
				else QT_GL3(GetShaderSource)(id, bufSize, &length, textOut);
			}
			else if (c.u(1) == QT_OGLES2_G_ProgramPipeline) {
				if (gl3::has43()) QT_GL3(GetProgramPipelineInfoLog)(id, bufSize, &length, textOut);
				else textOut[0] = 0;
			}
			else QT_GL3(GetProgramInfoLog)(id, bufSize, &length, textOut);
		}
		if (lengthOut) {
			uint32_t big = qt_swap32(static_cast<uint32_t>(length));
			memcpy(lengthOut, &big, 4);
		}
		return 0;
	}

	// glGenTextures/glDeleteTextures are real OpenGL 1.1 exports (QT_GL);
	// glGenFramebuffers/DeleteFramebuffers/glGenRenderbuffers/
	// DeleteRenderbuffers are not (QT_GL3). Real host objects throughout,
	// unlike QT_OGLES2_GEN_BUFFERS's fake ones: a texture or framebuffer
	// does not need converting from big-endian at draw time the way buffer
	// data does.
	bool genObjects(const Command& c) {
		uint32_t kind = c.u(1), n = c.u(2);
		uint8_t* out = c.p(3);
		GLuint names[64];
		if (!out || n > 64) return true;
		switch (kind) {
		case QT_OGLES2_K_Texture: QT_GL(GenTextures)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_Framebuffer: QT_GL3(GenFramebuffers)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_Renderbuffer: QT_GL3(GenRenderbuffers)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_Sampler: QT_GL3(GenSamplers)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_Query: QT_GL3(GenQueries)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_TransformFeedback:
			if (!gl3::has43()) return true;
			QT_GL3(GenTransformFeedbacks)(static_cast<GLsizei>(n), names);
			break;
		case QT_OGLES2_K_ProgramPipeline:
			if (!gl3::has43()) return true;
			QT_GL3(GenProgramPipelines)(static_cast<GLsizei>(n), names);
			break;
		default: return true;
		}
		for (uint32_t i = 0; i < n; ++i) {
			uint32_t big = qt_swap32(names[i]);
			memcpy(out + 4 * i, &big, 4);
		}
		return true;
	}

	bool deleteObjects(const Command& c) {
		uint32_t kind = c.u(1), n = c.u(2);
		const uint8_t* in = c.p(3);
		GLuint names[64];
		if (!in || n > 64) return true;
		for (uint32_t i = 0; i < n; ++i) {
			uint32_t big;
			memcpy(&big, in + 4 * i, 4);
			names[i] = qt_swap32(big);
		}
		switch (kind) {
		case QT_OGLES2_K_Texture: QT_GL(DeleteTextures)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_Framebuffer: {
			// Never the context's own object; deleting a bound framebuffer
			// binds 0, which is that object again.
			GLuint own = qt_default_framebuffer();
			for (uint32_t i = 0; i < n; ++i) if (own && names[i] == own) names[i] = 0;
			QT_GL3(DeleteFramebuffers)(static_cast<GLsizei>(n), names);
			if (own) {
				GLint draw = 0, read = 0;
				QT_GL(GetIntegerv)(0x8CA6, &draw);
				QT_GL(GetIntegerv)(0x8CAA, &read);
				if (!draw) QT_GL3(BindFramebuffer)(0x8CA9, own); // GL_DRAW_FRAMEBUFFER
				if (!read) QT_GL3(BindFramebuffer)(0x8CA8, own); // GL_READ_FRAMEBUFFER
			}
			break;
		}
		case QT_OGLES2_K_Renderbuffer: QT_GL3(DeleteRenderbuffers)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_Sampler: QT_GL3(DeleteSamplers)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_Query: QT_GL3(DeleteQueries)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_TransformFeedback: if (gl3::has43()) QT_GL3(DeleteTransformFeedbacks)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_ProgramPipeline: if (gl3::has43()) QT_GL3(DeleteProgramPipelines)(static_cast<GLsizei>(n), names); break;
		}
		return true;
	}

	// The format and type a sized internal format (ES 3.0's glTexStorage*)
	// is allocated with when there is no data: the base format of its
	// components (an _INTEGER one for the integer formats), GL_INT/
	// GL_UNSIGNED_INT for those, the depth and depth-stencil ones' own, and
	// GL_UNSIGNED_BYTE for the rest, which desktop OpenGL takes for any of
	// them without data.
	void storageFormat(GLenum internalformat, GLenum& format, GLenum& type) {
		struct Sized { GLenum internalformat; uint8_t components, kind; }; // kind: 0 normalised/float, 1 signed int, 2 unsigned int
		static const Sized sized[] = {
			{0x8229, 1, 0}, {0x8F94, 1, 0}, {0x822D, 1, 0}, {0x822E, 1, 0}, // R8, R8_SNORM, R16F, R32F
			{0x822B, 2, 0}, {0x8F95, 2, 0}, {0x822F, 2, 0}, {0x8230, 2, 0}, // RG8, RG8_SNORM, RG16F, RG32F
			{0x8051, 3, 0}, {0x8C41, 3, 0}, {0x8D62, 3, 0}, {0x8F96, 3, 0}, // RGB8, SRGB8, RGB565, RGB8_SNORM
			{0x8C3A, 3, 0}, {0x8C3D, 3, 0}, {0x881B, 3, 0}, {0x8815, 3, 0}, // R11F_G11F_B10F, RGB9_E5, RGB16F, RGB32F
			{0x8231, 1, 1}, {0x8232, 1, 2}, {0x8233, 1, 1}, {0x8234, 1, 2}, {0x8235, 1, 1}, {0x8236, 1, 2}, // R8I..R32UI
			{0x8237, 2, 1}, {0x8238, 2, 2}, {0x8239, 2, 1}, {0x823A, 2, 2}, {0x823B, 2, 1}, {0x823C, 2, 2}, // RG8I..RG32UI
			{0x8D8F, 3, 1}, {0x8D7D, 3, 2}, {0x8D89, 3, 1}, {0x8D77, 3, 2}, {0x8D83, 3, 1}, {0x8D71, 3, 2}, // RGB8I..RGB32UI
			{0x8D8E, 4, 1}, {0x8D7C, 4, 2}, {0x8D88, 4, 1}, {0x8D76, 4, 2}, {0x8D82, 4, 1}, {0x8D70, 4, 2}, // RGBA8I..RGBA32UI
			{0x906F, 4, 2}, // RGB10_A2UI
		};
		switch (internalformat) {
		case 0x81A5: case 0x81A6: case 0x8CAC: // DEPTH_COMPONENT16, 24, 32F
			format = 0x1902; type = GL_UNSIGNED_INT; return; // GL_DEPTH_COMPONENT
		case 0x88F0: format = 0x84F9; type = 0x84FA; return; // DEPTH24_STENCIL8: DEPTH_STENCIL, UNSIGNED_INT_24_8
		case 0x8CAD: format = 0x84F9; type = 0x8DAD; return; // DEPTH32F_STENCIL8: FLOAT_32_UNSIGNED_INT_24_8_REV
		}
		static const GLenum plain[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
		static const GLenum integer[] = {0x8D94, 0x8228, 0x8D98, 0x8D99}; // RED/RG/RGB/RGBA_INTEGER
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		for (size_t i = 0; i < sizeof(sized) / sizeof(sized[0]); ++i) {
			if (sized[i].internalformat != internalformat) continue;
			format = sized[i].kind ? integer[sized[i].components - 1] : plain[sized[i].components - 1];
			type = sized[i].kind == 1 ? GL_INT : sized[i].kind == 2 ? GL_UNSIGNED_INT : GL_UNSIGNED_BYTE;
			return;
		}
	}

	void swizzle(GLenum target, GLenum gles2Format);

	// glTexStorage2D/3D as glTexImage2D/3D without data, level by level
	// (OpenGL 4.2 has glTexStorage itself; 3.3 core is what the host
	// asks for), the levels beyond them cut off with GL_TEXTURE_MAX_LEVEL.
	void texStorage(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, bool three) {
		GLenum format, type;
		if (internalformat == 0x8D62) internalformat = 0x8051; // GL_RGB565 -> GL_RGB8, as glRenderbufferStorage's
		storageFormat(internalformat, format, type);
		for (GLsizei level = 0; level < levels; ++level) {
			if (three) QT_GL3(TexImage3D)(target, level, static_cast<GLint>(internalformat), width, height, depth, 0, format, type, 0);
			else if (target == GL_TEXTURE_CUBE_MAP) {
				for (GLenum face = 0; face < 6; ++face)
					QT_GL(TexImage2D)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, static_cast<GLint>(internalformat), width, height, 0, format, type, 0);
			}
			else QT_GL(TexImage2D)(target, level, static_cast<GLint>(internalformat), width, height, 0, format, type, 0);
			if (width > 1) width /= 2;
			if (height > 1 && target != 0x8C1A) height /= 2; // not for GL_TEXTURE_1D_ARRAY's layers
			if (depth > 1 && target == GL_TEXTURE_3D) depth /= 2; // a 2D array keeps its layers
		}
		QT_GL(TexParameteri)(target, GL_TEXTURE_MAX_LEVEL, levels - 1);
		swizzle(target, 0);
	}

	int32_t call(const Command& c) {
		uint32_t w0 = c.u(2), w1 = c.u(3), w2 = c.u(4), w3 = c.u(5), w4 = c.u(6);
		uint32_t w5 = c.u(7), w6 = c.u(8), w7 = c.u(9), w8 = c.u(10), w9 = c.u(11);
		switch (c.u(1)) {
		case QT_OGLES2_C_BindTexture: QT_GL(BindTexture)(static_cast<GLenum>(w0), static_cast<GLuint>(w1)); break;
		case QT_OGLES2_C_ActiveTexture: QT_GL3(ActiveTexture)(static_cast<GLenum>(w0)); break;
		case QT_OGLES2_C_GenerateMipmap: QT_GL3(GenerateMipmap)(static_cast<GLenum>(w0)); break;
		case QT_OGLES2_C_TexParameteri:
			QT_GL(TexParameteri)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), static_cast<GLint>(w2));
			break;
		case QT_OGLES2_C_BindFramebuffer: QT_GL3(BindFramebuffer)(static_cast<GLenum>(w0), hostFramebuffer(static_cast<GLuint>(w1))); break;
		case QT_OGLES2_C_BindRenderbuffer: QT_GL3(BindRenderbuffer)(static_cast<GLenum>(w0), static_cast<GLuint>(w1)); break;
		case QT_OGLES2_C_FramebufferTexture2D:
			QT_GL3(FramebufferTexture2D)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), static_cast<GLenum>(w2),
				static_cast<GLuint>(w3), static_cast<GLint>(w4));
			break;
		case QT_OGLES2_C_FramebufferRenderbuffer:
			QT_GL3(FramebufferRenderbuffer)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), static_cast<GLenum>(w2),
				static_cast<GLuint>(w3));
			break;
		case QT_OGLES2_C_CheckFramebufferStatus: return static_cast<int32_t>(QT_GL3(CheckFramebufferStatus)(static_cast<GLenum>(w0)));
		case QT_OGLES2_C_RenderbufferStorage:
			// GL_RGB565: GLES2's own, desktop OpenGL's only from 4.1 on.
			if (w1 == 0x8D62) w1 = 0x8051; // GL_RGB8
			QT_GL3(RenderbufferStorage)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), static_cast<GLsizei>(w2),
				static_cast<GLsizei>(w3));
			break;
		case QT_OGLES2_C_BlendColor: QT_GL3(BlendColor)(c.f(2), c.f(3), c.f(4), c.f(5)); break;
		case QT_OGLES2_C_BlendEquation: QT_GL3(BlendEquation)(static_cast<GLenum>(w0)); break;
		case QT_OGLES2_C_BlendEquationSeparate:
			QT_GL3(BlendEquationSeparate)(static_cast<GLenum>(w0), static_cast<GLenum>(w1));
			break;
		case QT_OGLES2_C_BlendFuncSeparate:
			QT_GL3(BlendFuncSeparate)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), static_cast<GLenum>(w2),
				static_cast<GLenum>(w3));
			break;
		case QT_OGLES2_C_StencilFuncSeparate:
			QT_GL3(StencilFuncSeparate)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), static_cast<GLint>(w2),
				static_cast<GLuint>(w3));
			break;
		case QT_OGLES2_C_StencilMaskSeparate:
			QT_GL3(StencilMaskSeparate)(static_cast<GLenum>(w0), static_cast<GLuint>(w1));
			break;
		case QT_OGLES2_C_StencilOpSeparate:
			QT_GL3(StencilOpSeparate)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), static_cast<GLenum>(w2),
				static_cast<GLenum>(w3));
			break;
		case QT_OGLES2_C_SampleCoverage: QT_GL3(SampleCoverage)(c.f(2), w1 ? GL_TRUE : GL_FALSE); break;
		case QT_OGLES2_C_IsFramebuffer: return QT_GL3(IsFramebuffer)(static_cast<GLuint>(w0)) ? 1 : 0;
		case QT_OGLES2_C_IsRenderbuffer: return QT_GL3(IsRenderbuffer)(static_cast<GLuint>(w0)) ? 1 : 0;
		case QT_OGLES2_C_IsBuffer: return w0 && buffers.count(w0) ? 1 : 0;
		case QT_OGLES2_C_VertexAttrib4f:
			QT_GL3(VertexAttrib4f)(static_cast<GLuint>(w0), c.f(3), c.f(4), c.f(5), c.f(6));
			break;
		case QT_OGLES2_C_ProvokingVertex: QT_GL3(ProvokingVertex)(static_cast<GLenum>(w0)); break;
		// OpenGL ES 3.0 (ogles3.library).
		case QT_OGLES2_C_UniformBlockBinding: QT_GL3(UniformBlockBinding)(w0, w1, w2); break;
		case QT_OGLES2_C_BindSampler: QT_GL3(BindSampler)(w0, w1); break;
		case QT_OGLES2_C_SamplerParameteri: QT_GL3(SamplerParameteri)(w0, static_cast<GLenum>(w1), static_cast<GLint>(w2)); break;
		case QT_OGLES2_C_SamplerParameterf: QT_GL3(SamplerParameterf)(w0, static_cast<GLenum>(w1), c.f(4)); break;
		case QT_OGLES2_C_IsSampler: return QT_GL3(IsSampler)(w0) ? 1 : 0;
		case QT_OGLES2_C_TexStorage2D:
			texStorage(static_cast<GLenum>(w0), static_cast<GLsizei>(w1), static_cast<GLenum>(w2), static_cast<GLsizei>(w3),
				static_cast<GLsizei>(w4), 1, false);
			break;
		case QT_OGLES2_C_TexStorage3D:
			texStorage(static_cast<GLenum>(w0), static_cast<GLsizei>(w1), static_cast<GLenum>(w2), static_cast<GLsizei>(w3),
				static_cast<GLsizei>(w4), static_cast<GLsizei>(w5), true);
			break;
		case QT_OGLES2_C_FramebufferTextureLayer:
			QT_GL3(FramebufferTextureLayer)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), w2, static_cast<GLint>(w3),
				static_cast<GLint>(w4));
			break;
		case QT_OGLES2_C_RenderbufferStorageMultisample:
			if (w2 == 0x8D62) w2 = 0x8051; // GL_RGB565 -> GL_RGB8
			QT_GL3(RenderbufferStorageMultisample)(static_cast<GLenum>(w0), static_cast<GLsizei>(w1), static_cast<GLenum>(w2),
				static_cast<GLsizei>(w3), static_cast<GLsizei>(w4));
			break;
		case QT_OGLES2_C_BlitFramebuffer:
			QT_GL3(BlitFramebuffer)(static_cast<GLint>(w0), static_cast<GLint>(w1), static_cast<GLint>(w2), static_cast<GLint>(w3),
				static_cast<GLint>(w4), static_cast<GLint>(w5), static_cast<GLint>(w6), static_cast<GLint>(w7), w8,
				static_cast<GLenum>(w9));
			break;
		case QT_OGLES2_C_VertexAttribI4i:
			QT_GL3(VertexAttribI4i)(w0, static_cast<GLint>(w1), static_cast<GLint>(w2), static_cast<GLint>(w3), static_cast<GLint>(w4));
			break;
		case QT_OGLES2_C_VertexAttribI4ui: QT_GL3(VertexAttribI4ui)(w0, w1, w2, w3, w4); break;
		case QT_OGLES2_C_BeginQuery: QT_GL3(BeginQuery)(static_cast<GLenum>(w0), w1); break;
		case QT_OGLES2_C_EndQuery: QT_GL3(EndQuery)(static_cast<GLenum>(w0)); break;
		case QT_OGLES2_C_IsQuery: return QT_GL3(IsQuery)(w0) ? 1 : 0;
		case QT_OGLES2_C_BeginTransformFeedback: beginTransformFeedback(static_cast<GLenum>(w0)); break;
		case QT_OGLES2_C_EndTransformFeedback: endTransformFeedback(); break;
		case QT_OGLES2_C_PauseTransformFeedback: if (gl3::has43()) QT_GL3(PauseTransformFeedback)(); break;
		case QT_OGLES2_C_ResumeTransformFeedback: if (gl3::has43()) QT_GL3(ResumeTransformFeedback)(); break;
		case QT_OGLES2_C_BindTransformFeedback: if (gl3::has43()) QT_GL3(BindTransformFeedback)(static_cast<GLenum>(w0), w1); break;
		case QT_OGLES2_C_IsTransformFeedback: return gl3::has43() && QT_GL3(IsTransformFeedback)(w0) ? 1 : 0;
		case QT_OGLES2_C_DispatchCompute:
			if (gl3::has43()) {
				PerGL& gl = perGL[currentGL()];
				bindUniformBuffers(gl);
				QT_GL3(DispatchCompute)(w0, w1, w2);
				markWritten(gl);
			}
			break;
		case QT_OGLES2_C_Has43: return gl3::has43() ? 1 : 0;
		case QT_OGLES2_C_MemoryBarrier: if (gl3::has43()) QT_GL3(MemoryBarrier)(w0); break;
		case QT_OGLES2_C_BindImageTexture:
			if (gl3::has43())
				QT_GL3(BindImageTexture)(w0, w1, static_cast<GLint>(w2), w3 ? GL_TRUE : GL_FALSE, static_cast<GLint>(w4),
					static_cast<GLenum>(w5), static_cast<GLenum>(w6));
			break;
		case QT_OGLES2_C_CopyTexSubImage3D:
			QT_GL3(CopyTexSubImage3D)(static_cast<GLenum>(w0), static_cast<GLint>(w1), static_cast<GLint>(w2), static_cast<GLint>(w3),
				static_cast<GLint>(w4), static_cast<GLint>(w5), static_cast<GLint>(w6), static_cast<GLsizei>(w7),
				static_cast<GLsizei>(w8));
			break;
		// The rest of OpenGL ES 3.1 (stage 3b).
		case QT_OGLES2_C_ProgramParameteri:
			if (gl3::has43()) QT_GL3(ProgramParameteri)(w0, static_cast<GLenum>(w1), static_cast<GLint>(w2));
			break;
		case QT_OGLES2_C_BindProgramPipeline: if (gl3::has43()) QT_GL3(BindProgramPipeline)(w0); break;
		case QT_OGLES2_C_UseProgramStages: if (gl3::has43()) QT_GL3(UseProgramStages)(w0, w1, w2); break;
		case QT_OGLES2_C_ActiveShaderProgram: if (gl3::has43()) QT_GL3(ActiveShaderProgram)(w0, w1); break;
		case QT_OGLES2_C_ValidateProgramPipeline: if (gl3::has43()) QT_GL3(ValidateProgramPipeline)(w0); break;
		case QT_OGLES2_C_IsProgramPipeline: return gl3::has43() && QT_GL3(IsProgramPipeline)(w0) ? 1 : 0;
		case QT_OGLES2_C_TexStorage2DMultisample:
			if (gl3::has43())
				QT_GL3(TexStorage2DMultisample)(static_cast<GLenum>(w0), static_cast<GLsizei>(w1), static_cast<GLenum>(w2),
					static_cast<GLsizei>(w3), static_cast<GLsizei>(w4), w5 ? GL_TRUE : GL_FALSE);
			break;
		case QT_OGLES2_C_SampleMaski: if (gl3::has43()) QT_GL3(SampleMaski)(w0, w1); break;
		case QT_OGLES2_C_FramebufferParameteri:
			if (gl3::has43()) QT_GL3(FramebufferParameteri)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), static_cast<GLint>(w2));
			break;
		case QT_OGLES2_C_ClipControl:
#ifndef QT_TEST
			if (!gl3::ClipControl) return 0;
#endif
			QT_GL3(ClipControl)(static_cast<GLenum>(w0), static_cast<GLenum>(w1));
			return 1;
		}
		return 0;
	}

	// The program interface queries (ES 3.1, QT_OGLES2_PROGRAM_RESOURCE).
	int32_t programResource(const Command& c) {
		if (!gl3::has43()) return c.u(1) == QT_OGLES2_R_Location ? -1 : c.u(1) == QT_OGLES2_R_Index ? -1 : 0;
		GLuint program = c.u(2);
		GLenum programInterface = static_cast<GLenum>(c.u(3));
		uint32_t index = c.u(4), propCount = c.u(6);
		GLsizei bufSize = static_cast<GLsizei>(c.u(7));
		uint8_t* lengthOut = c.p(8);
		GLsizei length = 0;
		switch (c.u(1)) {
		case QT_OGLES2_R_Interfaceiv: {
			GLint value = 0;
			uint8_t* out = c.p(9);
			QT_GL3(GetProgramInterfaceiv)(program, programInterface, static_cast<GLenum>(index), &value);
			if (out) putWord(out, static_cast<uint32_t>(value));
			return 0;
		}
		case QT_OGLES2_R_Index: case QT_OGLES2_R_Location: {
			const char* name = static_cast<const char*>(c.resolve(c.u(5)));
			if (!c.u(5) || !name) return -1;
			if (c.u(1) == QT_OGLES2_R_Index) return static_cast<int32_t>(QT_GL3(GetProgramResourceIndex)(program, programInterface, name));
			return QT_GL3(GetProgramResourceLocation)(program, programInterface, name);
		}
		case QT_OGLES2_R_Name: {
			char* nameOut = c.p(9);
			GLchar dummy[1];
			if (!nameOut || bufSize <= 0) {
				nameOut = dummy;
				bufSize = 1;
			}
			QT_GL3(GetProgramResourceName)(program, programInterface, index, bufSize, &length, nameOut);
			break;
		}
		case QT_OGLES2_R_iv: {
			const uint8_t* propsIn = c.p(5);
			uint8_t* out = c.p(9);
			if (!propsIn || !out || propCount > 64 || bufSize <= 0 || bufSize > 4096) break;
			// GL_ACTIVE_VARIABLES gives as many values as the resource has:
			// room for what the application said it has room for.
			std::vector<GLenum> props(propCount ? propCount : 1);
			for (uint32_t i = 0; i < propCount; ++i) props[i] = word(propsIn + 4 * i);
			std::vector<GLint> values(static_cast<size_t>(bufSize));
			QT_GL3(GetProgramResourceiv)(program, programInterface, index, static_cast<GLsizei>(propCount), &props[0], bufSize,
				&length, &values[0]);
			for (GLsizei i = 0; i < length && i < bufSize; ++i) putWord(out + 4 * i, static_cast<uint32_t>(values[i]));
			break;
		}
		}
		if (lengthOut) putWord(lengthOut, static_cast<uint32_t>(length));
		return 0;
	}

	// GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA: GLES2 has them, desktop
	// core profiles do not; stored as GL_RED/GL_RG and swizzled back, as
	// Warp3D's own single-channel textures are (host/w3d.cpp). Returns
	// whether format was one of them (and changes it, and internalformat
	// when given, to the core one).
	bool coreFormat(GLenum& format, GLint* internalformat) {
		switch (format) {
		case GL_ALPHA: case GL_LUMINANCE:
			format = GL_RED;
			if (internalformat) *internalformat = GL_R8;
			return true;
		case GL_LUMINANCE_ALPHA:
			format = GL_RG;
			if (internalformat) *internalformat = GL_RG8;
			return true;
		}
		return false;
	}

	void swizzle(GLenum target, GLenum gles2Format) {
		// Cube map faces: the parameters belong to the cube map itself.
		if (target >= 0x8515 && target <= 0x851A) target = 0x8513;
		GLint r = GL_RED, g = GL_GREEN, b = GL_BLUE, a = GL_ALPHA;
		switch (gles2Format) {
		case GL_ALPHA: r = g = b = GL_ZERO; a = GL_RED; break;
		case GL_LUMINANCE: r = g = b = GL_RED; a = GL_ONE; break;
		case GL_LUMINANCE_ALPHA: r = g = b = GL_RED; a = GL_GREEN; break;
		}
		QT_GL(TexParameteri)(target, GL_TEXTURE_SWIZZLE_R, r);
		QT_GL(TexParameteri)(target, GL_TEXTURE_SWIZZLE_G, g);
		QT_GL(TexParameteri)(target, GL_TEXTURE_SWIZZLE_B, b);
		QT_GL(TexParameteri)(target, GL_TEXTURE_SWIZZLE_A, a);
	}

	bool packed16(GLenum type) {
		return type == 0x8363 || type == 0x8033 || type == 0x8034; // GL_UNSIGNED_SHORT_5_6_5, _4_4_4_4, _5_5_5_1
	}

	// The size of the values a pixel type's data is swapped in: 2 or 4 for
	// the multi-byte ones (OES_depth_texture's GL_UNSIGNED_SHORT/INT, OES_
	// packed_depth_stencil's GL_UNSIGNED_INT_24_8 included), 1 for GL_
	// UNSIGNED_BYTE, 0 for a type not supported.
	uint32_t swapUnit(GLenum type) {
		switch (type) {
		case GL_UNSIGNED_BYTE: case GL_BYTE: return 1;
		case GL_UNSIGNED_SHORT: case GL_SHORT: case 0x140B: return 2; // GL_HALF_FLOAT
		case GL_UNSIGNED_INT: case GL_INT: case GL_FLOAT:
		case 0x84FA: case 0x8368: case 0x8C3B: case 0x8C3E: case 0x8DAD:
			// GL_UNSIGNED_INT_24_8, _2_10_10_10_REV, _10F_11F_11F_REV, _5_9_9_9_REV, FLOAT_32_UNSIGNED_INT_24_8_REV
			return 4;
		}
		return packed16(type) ? 2 : 0;
	}

	uint32_t pixelBytes(GLenum format, GLenum type) {
		if (packed16(type)) return 2;
		if (type == 0x8DAD) return 8; // a float depth and a word of stencil
		if (type == 0x84FA || type == 0x8368 || type == 0x8C3B || type == 0x8C3E) return 4; // packed in one word
		uint32_t components;
		switch (format) {
		case GL_LUMINANCE_ALPHA: case GL_RG: case 0x8228: components = 2; break; // GL_RG_INTEGER
		case GL_RGB: case 0x8D98: components = 3; break; // GL_RGB_INTEGER
		case GL_RGBA: case 0x80E1: case 0x8D99: components = 4; break; // GL_BGRA, GL_RGBA_INTEGER
		default: components = 1; // GL_ALPHA, GL_LUMINANCE, GL_RED(_INTEGER), GL_DEPTH_COMPONENT
		}
		return components * swapUnit(type);
	}

	// The bytes a width x height image takes with the given row alignment
	// (GL_UNPACK_/PACK_ALIGNMENT); the last row is not padded.
	size_t imageSize(GLsizei width, GLsizei height, uint32_t bytes, GLint alignment) {
		if (width <= 0 || height <= 0) return 0;
		size_t row = static_cast<size_t>(width) * bytes;
		size_t a = alignment > 0 ? static_cast<size_t>(alignment) : 4;
		size_t stride = (row + a - 1) / a * a;
		return stride * (height - 1) + row;
	}

	// Every row starts on a whole unit: a row of 2- or 4-byte values is a
	// multiple of their size already, and padding only adds to it.
	void swapUnits(uint8_t* p, size_t size, uint32_t unit) {
		if (unit == 2) {
			for (size_t i = 0; i + 1 < size; i += 2) {
				uint8_t t = p[i];
				p[i] = p[i + 1];
				p[i + 1] = t;
			}
		}
		else if (unit == 4) {
			for (size_t i = 0; i + 3 < size; i += 4) {
				uint32_t v;
				memcpy(&v, p + i, 4);
				v = qt_swap32(v);
				memcpy(p + i, &v, 4);
			}
		}
	}

	// Pixel data for glTexImage2D/glTexSubImage2D in host order: as given
	// for GL_UNSIGNED_BYTE (and NULL), a swapped copy (in converted) for the
	// multi-byte types; false for a type not supported.
	bool hostPixels(GLenum format, GLenum type, const uint8_t* data, GLsizei width, GLsizei height,
			std::vector<uint8_t>& converted, const void*& pixels, GLsizei depth = 1) {
		uint32_t unit = swapUnit(type);
		pixels = data;
		if (!unit) return false;
		if (unit == 1 || !data) return true;
		GLint alignment = 4;
		QT_GL(GetIntegerv)(GL_UNPACK_ALIGNMENT, &alignment);
		// A 3D image's slices follow each other (GL_UNPACK_IMAGE_HEIGHT 0),
		// each padded as a 2D one is.
		size_t slice = imageSize(width, height, pixelBytes(format, type), alignment);
		if (depth > 1) {
			size_t row = static_cast<size_t>(width) * pixelBytes(format, type), a = alignment > 0 ? alignment : 4;
			slice = (row + a - 1) / a * a * height;
			slice = slice * (depth - 1) + imageSize(width, height, pixelBytes(format, type), alignment);
		}
		converted.assign(data, data + slice);
		if (!converted.empty()) swapUnits(&converted[0], converted.size(), unit);
		pixels = converted.empty() ? 0 : &converted[0];
		return true;
	}

	// GL_UNSIGNED_BYTE pixels are byte-granular, so forwarded as given; the
	// 16-bit packed types are swapped (hostPixels). data may be NULL, a
	// render target's own storage, allocated by the driver.
	bool texImage2D(const Command& c) {
		GLenum target = static_cast<GLenum>(c.u(1));
		GLint level = static_cast<GLint>(c.u(2)), internalformat = static_cast<GLint>(c.u(3));
		GLsizei width = static_cast<GLsizei>(c.u(4)), height = static_cast<GLsizei>(c.u(5));
		GLint border = static_cast<GLint>(c.u(6));
		GLenum format = static_cast<GLenum>(c.u(7)), type = static_cast<GLenum>(c.u(8));
		std::vector<uint8_t> converted;
		const void* pixels;
		if (!hostPixels(format, type, c.p(9), width, height, converted, pixels)) {
			qt_report("ogles2: GLTexImage2D: pixel type not supported");
			return true;
		}
		GLenum gles2Format = format;
		bool swizzled = coreFormat(format, &internalformat);
		QT_GL(TexImage2D)(target, level, internalformat, width, height, border, format, type, const_cast<void*>(pixels));
		// Level 0 decides it; a texture may be reused with another format.
		if (level == 0) swizzle(target, swizzled ? gles2Format : 0);
		return true;
	}

	bool texSubImage2D(const Command& c) {
		GLenum target = static_cast<GLenum>(c.u(1));
		GLint level = static_cast<GLint>(c.u(2)), x = static_cast<GLint>(c.u(3)), y = static_cast<GLint>(c.u(4));
		GLsizei width = static_cast<GLsizei>(c.u(5)), height = static_cast<GLsizei>(c.u(6));
		GLenum format = static_cast<GLenum>(c.u(7)), type = static_cast<GLenum>(c.u(8));
		std::vector<uint8_t> converted;
		const void* pixels;
		if (!c.u(9)) return true;
		if (!hostPixels(format, type, c.p(9), width, height, converted, pixels)) {
			qt_report("ogles2: GLTexSubImage2D: pixel type not supported");
			return true;
		}
		coreFormat(format, 0);
		QT_GL(TexSubImage2D)(target, level, x, y, width, height, format, type, const_cast<void*>(pixels));
		return true;
	}

	bool readPixels(const Command& c) {
		GLint x = static_cast<GLint>(c.u(1)), y = static_cast<GLint>(c.u(2));
		GLsizei width = static_cast<GLsizei>(c.u(3)), height = static_cast<GLsizei>(c.u(4));
		GLenum format = static_cast<GLenum>(c.u(5)), type = static_cast<GLenum>(c.u(6));
		uint8_t* out = c.p(7);
		if (!out) return true;
		QT_GL(ReadPixels)(x, y, width, height, format, type, out);
		uint32_t unit = swapUnit(type);
		if (unit > 1) {
			GLint alignment = 4;
			QT_GL(GetIntegerv)(GL_PACK_ALIGNMENT, &alignment);
			swapUnits(out, imageSize(width, height, pixelBytes(format, type), alignment), unit);
		}
		return true;
	}

	bool texImage3D(const Command& c) {
		GLenum target = static_cast<GLenum>(c.u(1));
		GLint level = static_cast<GLint>(c.u(2)), internalformat = static_cast<GLint>(c.u(3));
		GLsizei width = static_cast<GLsizei>(c.u(4)), height = static_cast<GLsizei>(c.u(5)), depth = static_cast<GLsizei>(c.u(6));
		GLint border = static_cast<GLint>(c.u(7));
		GLenum format = static_cast<GLenum>(c.u(8)), type = static_cast<GLenum>(c.u(9));
		std::vector<uint8_t> converted;
		const void* pixels;
		if (!hostPixels(format, type, c.p(10), width, height, converted, pixels, depth)) {
			qt_report("ogles3: GLTexImage3D: pixel type not supported");
			return true;
		}
		GLenum gles2Format = format;
		bool swizzled = coreFormat(format, &internalformat);
		QT_GL3(TexImage3D)(target, level, internalformat, width, height, depth, border, format, type, pixels);
		if (level == 0) swizzle(target, swizzled ? gles2Format : 0);
		return true;
	}

	bool texSubImage3D(const Command& c) {
		GLenum target = static_cast<GLenum>(c.u(1));
		GLint level = static_cast<GLint>(c.u(2)), x = static_cast<GLint>(c.u(3)), y = static_cast<GLint>(c.u(4)),
			z = static_cast<GLint>(c.u(5));
		GLsizei width = static_cast<GLsizei>(c.u(6)), height = static_cast<GLsizei>(c.u(7)), depth = static_cast<GLsizei>(c.u(8));
		GLenum format = static_cast<GLenum>(c.u(9)), type = static_cast<GLenum>(c.u(10));
		std::vector<uint8_t> converted;
		const void* pixels;
		if (!c.u(11)) return true;
		if (!hostPixels(format, type, c.p(11), width, height, converted, pixels, depth)) {
			qt_report("ogles3: GLTexSubImage3D: pixel type not supported");
			return true;
		}
		coreFormat(format, 0);
		QT_GL3(TexSubImage3D)(target, level, x, y, z, width, height, depth, format, type, pixels);
		return true;
	}

	// The compressed formats offered (gl/oglescmd.h): OpenGL ES 3.0's ETC2/EAC
	// ones, which OpenGL 4.3 has too (GL_ARB_ES3_compatibility), and ETC1.
	const GLint compressedFormats[] = {
		0x9270, 0x9271, 0x9272, 0x9273, 0x9274, 0x9275, 0x9276, 0x9277, 0x9278, 0x9279, // R11_EAC .. SRGB8_ALPHA8_ETC2_EAC
		0x8D64 // ETC1_RGB8_OES
	};
	uint32_t compressedFormatCount() {
		return gl3::has43() ? sizeof(compressedFormats) / sizeof(compressedFormats[0]) : 0;
	}

	bool compressedTex(const Command& c) {
		GLenum target = static_cast<GLenum>(c.u(2));
		GLint level = static_cast<GLint>(c.u(3));
		GLint x = static_cast<GLint>(c.u(4)), y = static_cast<GLint>(c.u(5)), z = static_cast<GLint>(c.u(6));
		GLsizei width = static_cast<GLsizei>(c.u(7)), height = static_cast<GLsizei>(c.u(8)), depth = static_cast<GLsizei>(c.u(9));
		GLenum format = static_cast<GLenum>(c.u(10));
		GLsizei size = static_cast<GLsizei>(c.u(11));
		const void* data = c.p(12);
		if (size < 0 || size > (256 << 20) || (size && !data)) {
			qt_report("ogles2: glCompressedTex*Image*: data not readable");
			return true;
		}
		// ETC1 is a subset of ETC2's RGB8 (desktop OpenGL has no ETC1 token).
		if (format == 0x8D64) format = 0x9274;
		GLenum internalformat = static_cast<GLenum>(x);
		if (internalformat == 0x8D64) internalformat = 0x9274;
		switch (c.u(1)) {
		case QT_OGLES2_CT_Image2D: QT_GL3(CompressedTexImage2D)(target, level, internalformat, width, height, 0, size, data); break;
		case QT_OGLES2_CT_Sub2D: QT_GL3(CompressedTexSubImage2D)(target, level, x, y, width, height, format, size, data); break;
		case QT_OGLES2_CT_Image3D: QT_GL3(CompressedTexImage3D)(target, level, internalformat, width, height, depth, 0, size, data); break;
		case QT_OGLES2_CT_Sub3D: QT_GL3(CompressedTexSubImage3D)(target, level, x, y, z, width, height, depth, format, size, data); break;
		}
		return true;
	}

	// count names given as big-endian Amiga addresses of NUL-terminated text;
	// false if one cannot be read.
	bool nameArray(const Command& c, uint32_t count, const uint8_t* addresses, std::vector<const GLchar*>& names) {
		if (!addresses || count > 256) return false;
		names.resize(count);
		for (uint32_t i = 0; i < count; ++i) {
			names[i] = static_cast<const GLchar*>(c.resolve(word(addresses + 4 * i)));
			if (!names[i]) return false;
		}
		return true;
	}

	bool transformFeedbackVaryings(const Command& c) {
		std::vector<const GLchar*> names;
		uint32_t count = c.u(2);
		if (!nameArray(c, count, c.p(3), names)) return true;
		QT_GL3(TransformFeedbackVaryings)(c.u(1), static_cast<GLsizei>(count), count ? &names[0] : 0, static_cast<GLenum>(c.u(4)));
		return true;
	}

	bool uniformIndices(const Command& c) {
		std::vector<const GLchar*> names;
		uint32_t count = c.u(2);
		uint8_t* out = c.p(4);
		if (!out || !count || !nameArray(c, count, c.p(3), names)) return true;
		std::vector<GLuint> indices(count);
		QT_GL3(GetUniformIndices)(c.u(1), static_cast<GLsizei>(count), &names[0], &indices[0]);
		for (uint32_t i = 0; i < count; ++i) putWord(out + 4 * i, indices[i]);
		return true;
	}

	bool activeUniformsiv(const Command& c) {
		uint32_t count = c.u(2);
		const uint8_t* in = c.p(3);
		uint8_t* out = c.p(5);
		if (!in || !out || !count || count > 1024) return true;
		std::vector<GLuint> indices(count);
		std::vector<GLint> values(count);
		for (uint32_t i = 0; i < count; ++i) indices[i] = word(in + 4 * i);
		QT_GL3(GetActiveUniformsiv)(c.u(1), static_cast<GLsizei>(count), &indices[0], static_cast<GLenum>(c.u(4)), &values[0]);
		for (uint32_t i = 0; i < count; ++i) putWord(out + 4 * i, static_cast<uint32_t>(values[i]));
		return true;
	}

	// glClearBuffer*: the colour values are four words, depth or stencil one.
	bool clearBuffer(const Command& c) {
		GLenum buffer = static_cast<GLenum>(c.u(2));
		GLint drawbuffer = static_cast<GLint>(c.u(3));
		if (c.u(1) == QT_OGLES2_CB_fi) {
			QT_GL3(ClearBufferfi)(buffer, drawbuffer, c.f(4), static_cast<GLint>(c.u(5)));
			return true;
		}
		const uint8_t* in = c.p(4);
		if (!in) return true;
		uint32_t v[4] = {0, 0, 0, 0};
		for (int i = 0; i < (buffer == GL_COLOR ? 4 : 1); ++i) v[i] = word(in + 4 * i);
		switch (c.u(1)) {
		case QT_OGLES2_CB_iv: QT_GL3(ClearBufferiv)(buffer, drawbuffer, reinterpret_cast<const GLint*>(v)); break;
		case QT_OGLES2_CB_uiv: QT_GL3(ClearBufferuiv)(buffer, drawbuffer, v); break;
		case QT_OGLES2_CB_fv: QT_GL3(ClearBufferfv)(buffer, drawbuffer, reinterpret_cast<const GLfloat*>(v)); break;
		}
		return true;
	}

	// glUniform*v and glUniformMatrix{2,3,4}fv: count vectors/matrices of
	// big-endian words, swapped into host order.
	int32_t setUniformV(uint32_t subop, GLint location, uint32_t count, const uint8_t* in, GLboolean transpose);

	int32_t uniformV(const Command& c) {
		return setUniformV(c.u(1), static_cast<GLint>(c.u(2)), c.u(3), c.p(4), c.u(5) ? GL_TRUE : GL_FALSE);
	}

	// glProgramUniform* (ES 3.1): the program made current for the call, and
	// the one before put back (OpenGL 4.1's glProgramUniform* would need a
	// call each; this keeps setUniformV's one switch). A bound pipeline stays
	// bound: glUseProgram(0) leaves it in charge again.
	int32_t programUniformV(const Command& c) {
		GLint previous = 0;
		QT_GL(GetIntegerv)(0x8B8D, &previous); // GL_CURRENT_PROGRAM
		QT_GL3(UseProgram)(static_cast<GLuint>(c.u(2)));
		setUniformV(c.u(1), static_cast<GLint>(c.u(3)), c.u(4), c.p(5), c.u(6) ? GL_TRUE : GL_FALSE);
		QT_GL3(UseProgram)(static_cast<GLuint>(previous));
		return 0;
	}

	int32_t setUniformV(uint32_t subop, GLint location, uint32_t count, const uint8_t* in, GLboolean transpose) {
		// By subop: 1-4F, 1-4I, M2, M3, M4, 1-4UI, M2x3, M3x2, M2x4, M4x2, M3x4, M4x3.
		static const uint32_t components[] = {1, 2, 3, 4, 1, 2, 3, 4, 4, 9, 16, 1, 2, 3, 4, 6, 6, 8, 8, 12, 12};
		if (!in || subop > QT_OGLES2_U_M4x3 || count > 1024) return 0;
		uint32_t n = count * components[subop];
		std::vector<uint32_t> v(n ? n : 1);
		for (uint32_t i = 0; i < n; ++i) v[i] = word(in + 4 * i);
		GLsizei k = static_cast<GLsizei>(count);
		const GLfloat* f = reinterpret_cast<const GLfloat*>(&v[0]);
		const GLint* iv = reinterpret_cast<const GLint*>(&v[0]);
		const GLuint* uv = &v[0];
		switch (subop) {
		case QT_OGLES2_U_1F: QT_GL3(Uniform1fv)(location, k, f); break;
		case QT_OGLES2_U_2F: QT_GL3(Uniform2fv)(location, k, f); break;
		case QT_OGLES2_U_3F: QT_GL3(Uniform3fv)(location, k, f); break;
		case QT_OGLES2_U_4F: QT_GL3(Uniform4fv)(location, k, f); break;
		case QT_OGLES2_U_1I: QT_GL3(Uniform1iv)(location, k, iv); break;
		case QT_OGLES2_U_2I: QT_GL3(Uniform2iv)(location, k, iv); break;
		case QT_OGLES2_U_3I: QT_GL3(Uniform3iv)(location, k, iv); break;
		case QT_OGLES2_U_4I: QT_GL3(Uniform4iv)(location, k, iv); break;
		case QT_OGLES2_U_M2: QT_GL3(UniformMatrix2fv)(location, k, transpose, f); break;
		case QT_OGLES2_U_M3: QT_GL3(UniformMatrix3fv)(location, k, transpose, f); break;
		case QT_OGLES2_U_M4: QT_GL3(UniformMatrix4fv)(location, k, transpose, f); break;
		case QT_OGLES2_U_1UI: QT_GL3(Uniform1uiv)(location, k, uv); break;
		case QT_OGLES2_U_2UI: QT_GL3(Uniform2uiv)(location, k, uv); break;
		case QT_OGLES2_U_3UI: QT_GL3(Uniform3uiv)(location, k, uv); break;
		case QT_OGLES2_U_4UI: QT_GL3(Uniform4uiv)(location, k, uv); break;
		case QT_OGLES2_U_M2x3: QT_GL3(UniformMatrix2x3fv)(location, k, transpose, f); break;
		case QT_OGLES2_U_M3x2: QT_GL3(UniformMatrix3x2fv)(location, k, transpose, f); break;
		case QT_OGLES2_U_M2x4: QT_GL3(UniformMatrix2x4fv)(location, k, transpose, f); break;
		case QT_OGLES2_U_M4x2: QT_GL3(UniformMatrix4x2fv)(location, k, transpose, f); break;
		case QT_OGLES2_U_M3x4: QT_GL3(UniformMatrix3x4fv)(location, k, transpose, f); break;
		case QT_OGLES2_U_M4x3: QT_GL3(UniformMatrix4x3fv)(location, k, transpose, f); break;
		}
		return 0;
	}

	// How many values glGet* returns for pname (GLES2's list; 1 for the
	// rest).
	uint32_t valueCount(GLenum pname) {
		switch (pname) {
		case 0x0BA2: case 0x0C10: case 0x0C22: case 0x0C23: case 0x8005: // VIEWPORT, SCISSOR_BOX, COLOR_CLEAR_VALUE, COLOR_WRITEMASK, BLEND_COLOR
			return 4;
		case 0x0B70: case 0x846D: case 0x846E: case 0x0D3A: // DEPTH_RANGE, ALIASED_POINT_SIZE_RANGE, ALIASED_LINE_WIDTH_RANGE, MAX_VIEWPORT_DIMS
			return 2;
		case 0x86A3: // COMPRESSED_TEXTURE_FORMATS
			return compressedFormatCount();
		case 0x8DF8: case 0x87FF: // SHADER_BINARY_FORMATS, PROGRAM_BINARY_FORMATS: none offered
			return 0;
		}
		return 1;
	}

	// GLES2-only pnames, answered from desktop OpenGL's; false for the rest.
	bool emulatedGet(GLenum pname, GLint& value) {
		GLint v = 0;
		switch (pname) {
		case 0x8DFB: QT_GL(GetIntegerv)(0x8B4A, &v); value = v / 4; return true; // MAX_VERTEX_UNIFORM_VECTORS
		case 0x8DFD: QT_GL(GetIntegerv)(0x8B49, &v); value = v / 4; return true; // MAX_FRAGMENT_UNIFORM_VECTORS
		case 0x8DFC: QT_GL(GetIntegerv)(0x9122, &v); value = v / 4; return true; // MAX_VARYING_VECTORS (from MAX_VERTEX_OUTPUT_COMPONENTS)
		case 0x86A2: value = static_cast<GLint>(compressedFormatCount()); return true; // NUM_COMPRESSED_TEXTURE_FORMATS
		case 0x8DF9: case 0x87FE: value = 0; return true; // NUM_SHADER_BINARY_FORMATS, NUM_PROGRAM_BINARY_FORMATS
		case 0x8DFA: value = 1; return true; // SHADER_COMPILER
		case 0x8B9A: value = GL_UNSIGNED_BYTE; return true; // IMPLEMENTATION_COLOR_READ_TYPE
		case 0x8B9B: value = GL_RGBA; return true; // IMPLEMENTATION_COLOR_READ_FORMAT
		case 0x8CA6: case 0x8CAA: // DRAW_/READ_FRAMEBUFFER_BINDING: 0 for the context's own object
			QT_GL(GetIntegerv)(pname, &v); value = programFramebuffer(v); return true;
		}
		return false;
	}

	int32_t get(const Command& c) {
		uint32_t kind = c.u(1);
		GLenum pname = static_cast<GLenum>(c.u(2));
		uint8_t* out = c.p(3);
		uint32_t n = valueCount(pname);
		if (!out || !n) return 0;
		GLint emulated;
		if (emulatedGet(pname, emulated)) {
			if (kind == QT_OGLES2_GET_Integer) putWord(out, static_cast<uint32_t>(emulated));
			else if (kind == QT_OGLES2_GET_Float) putFloat(out, static_cast<float>(emulated));
			else out[0] = emulated ? 1 : 0;
			return 0;
		}
		if (pname == 0x86A3) { // COMPRESSED_TEXTURE_FORMATS: ours, not the desktop driver's list
			for (uint32_t i = 0; i < n; ++i) {
				if (kind == QT_OGLES2_GET_Integer) putWord(out + 4 * i, static_cast<uint32_t>(compressedFormats[i]));
				else if (kind == QT_OGLES2_GET_Float) putFloat(out + 4 * i, static_cast<float>(compressedFormats[i]));
				else out[i] = 1;
			}
			return 0;
		}
		if (pname == 0x846D) pname = 0x0B12; // ALIASED_POINT_SIZE_RANGE: core profiles' POINT_SIZE_RANGE
		GLint ints[16] = {0};
		GLfloat floats[16] = {0};
		GLboolean booleans[16] = {0};
		uint32_t i;
		switch (kind) {
		case QT_OGLES2_GET_Integer:
			QT_GL(GetIntegerv)(pname, ints);
			for (i = 0; i < n; ++i) putWord(out + 4 * i, static_cast<uint32_t>(ints[i]));
			break;
		case QT_OGLES2_GET_Float:
			QT_GL(GetFloatv)(pname, floats);
			for (i = 0; i < n; ++i) putFloat(out + 4 * i, floats[i]);
			break;
		case QT_OGLES2_GET_Boolean:
			QT_GL(GetBooleanv)(pname, booleans);
			for (i = 0; i < n; ++i) out[i] = booleans[i];
			break;
		}
		return 0;
	}

	// The number of values glGetUniform*v returns for a uniform type.
	uint32_t typeValues(GLenum type) {
		switch (type) {
		case 0x8B50: case 0x8B53: case 0x8B57: return 2; // vec2, ivec2, bvec2
		case 0x8B51: case 0x8B54: case 0x8B58: return 3;
		case 0x8B52: case 0x8B55: case 0x8B59: case 0x8B5A: return 4; // ..., mat2
		case 0x8B5B: return 9; // mat3
		case 0x8B5C: return 16; // mat4
		}
		return 1; // float, int, bool, samplers
	}

	// The type of the uniform at location in program: the active uniform
	// whose own location it is, or for an array element, the array whose
	// locations it falls in.
	uint32_t uniformValues(GLuint program, GLint location) {
		GLint active = 0;
		QT_GL3(GetProgramiv)(program, 0x8B86, &active); // GL_ACTIVE_UNIFORMS
		for (GLint i = 0; i < active; ++i) {
			GLchar name[256];
			GLsizei length = 0;
			GLint size = 0;
			GLenum type = 0;
			QT_GL3(GetActiveUniform)(program, static_cast<GLuint>(i), sizeof(name), &length, &size, &type, name);
			GLint base = QT_GL3(GetUniformLocation)(program, name);
			if (base >= 0 && location >= base && location < base + (size > 0 ? size : 1)) return typeValues(type);
		}
		return 1;
	}

	int32_t getParam(const Command& c) {
		uint32_t subop = c.u(1), w0 = c.u(2), w1 = c.u(3), w2 = c.u(4);
		uint8_t* out = c.p(5);
		GLint ints[16] = {0};
		GLfloat floats[16] = {0};
		if (!out) return 0;
		switch (subop) {
		case QT_OGLES2_P_TexParameteriv:
			QT_GL(GetTexParameteriv)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), ints);
			putWord(out, static_cast<uint32_t>(ints[0]));
			break;
		case QT_OGLES2_P_TexParameterfv:
			QT_GL(GetTexParameterfv)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), floats);
			putFloat(out, floats[0]);
			break;
		case QT_OGLES2_P_RenderbufferParameteriv:
			QT_GL3(GetRenderbufferParameteriv)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), ints);
			putWord(out, static_cast<uint32_t>(ints[0]));
			break;
		case QT_OGLES2_P_FramebufferAttachmentParameteriv: {
			bool own = defaultBound(static_cast<GLenum>(w0));
			GLenum attachment = static_cast<GLenum>(w1);
			if (own) attachment = defaultAttachment(attachment);
			QT_GL3(GetFramebufferAttachmentParameteriv)(static_cast<GLenum>(w0), attachment, static_cast<GLenum>(w2), ints);
			// GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE of the default framebuffer
			if (own && w2 == 0x8CD0 && ints[0] == 0x8D41) ints[0] = 0x8218; // RENDERBUFFER -> FRAMEBUFFER_DEFAULT
			putWord(out, static_cast<uint32_t>(ints[0]));
			break;
		}
		case QT_OGLES2_P_BufferParameteriv: {
			std::vector<uint8_t>* b = buffer(w0);
			uint32_t value = 0;
			if (w1 == 0x8764) value = b ? static_cast<uint32_t>(b->size()) : 0; // GL_BUFFER_SIZE
			else if (w1 == 0x8765) value = usages.count(w0) ? usages[w0] : 0x88E4; // GL_BUFFER_USAGE, default GL_STATIC_DRAW
			putWord(out, value);
			break;
		}
		case QT_OGLES2_P_Queryiv:
			QT_GL3(GetQueryiv)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), ints);
			putWord(out, static_cast<uint32_t>(ints[0]));
			break;
		case QT_OGLES2_P_QueryObjectuiv: {
			GLuint value = 0;
			QT_GL3(GetQueryObjectuiv)(w0, static_cast<GLenum>(w1), &value);
			putWord(out, value);
			break;
		}
		case QT_OGLES2_P_SamplerParameteriv:
			QT_GL3(GetSamplerParameteriv)(w0, static_cast<GLenum>(w1), ints);
			putWord(out, static_cast<uint32_t>(ints[0]));
			break;
		case QT_OGLES2_P_SamplerParameterfv:
			QT_GL3(GetSamplerParameterfv)(w0, static_cast<GLenum>(w1), floats);
			putFloat(out, floats[0]);
			break;
		case QT_OGLES2_P_ActiveUniformBlockiv: {
			GLint count = 1;
			if (w2 == 0x8A43) QT_GL3(GetActiveUniformBlockiv)(w0, w1, 0x8A42, &count); // ACTIVE_UNIFORM_INDICES: ACTIVE_UNIFORMS of them
			if (count < 1) break;
			std::vector<GLint> values(static_cast<size_t>(count));
			QT_GL3(GetActiveUniformBlockiv)(w0, w1, static_cast<GLenum>(w2), &values[0]);
			for (GLint i = 0; i < count; ++i) putWord(out + 4 * i, static_cast<uint32_t>(values[i]));
			break;
		}
		case QT_OGLES2_P_Integeri_v:
			putWord(out, static_cast<uint32_t>(indexedInteger(static_cast<GLenum>(w0), w1)));
			break;
		case QT_OGLES2_P_ProgramPipelineiv:
			if (gl3::has43()) QT_GL3(GetProgramPipelineiv)(w0, static_cast<GLenum>(w1), ints);
			putWord(out, static_cast<uint32_t>(ints[0]));
			break;
		case QT_OGLES2_P_Multisamplefv:
			if (gl3::has43()) QT_GL3(GetMultisamplefv)(static_cast<GLenum>(w0), w1, floats);
			putFloat(out, floats[0]);
			putFloat(out + 4, floats[1]);
			break;
		case QT_OGLES2_P_TexLevelParameteriv:
			QT_GL(GetTexLevelParameteriv)(static_cast<GLenum>(w0), static_cast<GLint>(w1), static_cast<GLenum>(w2), ints);
			putWord(out, static_cast<uint32_t>(ints[0]));
			break;
		case QT_OGLES2_P_TexLevelParameterfv:
			QT_GL(GetTexLevelParameterfv)(static_cast<GLenum>(w0), static_cast<GLint>(w1), static_cast<GLenum>(w2), floats);
			putFloat(out, floats[0]);
			break;
		case QT_OGLES2_P_FramebufferParameteriv:
			if (gl3::has43()) QT_GL3(GetFramebufferParameteriv)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), ints);
			putWord(out, static_cast<uint32_t>(ints[0]));
			break;
		case QT_OGLES2_P_Uniformfv: case QT_OGLES2_P_Uniformiv: {
			GLuint program = static_cast<GLuint>(w0);
			GLint location = static_cast<GLint>(w1);
			uint32_t n = uniformValues(program, location);
			if (subop == QT_OGLES2_P_Uniformfv) {
				QT_GL3(GetUniformfv)(program, location, floats);
				for (uint32_t i = 0; i < n; ++i) putFloat(out + 4 * i, floats[i]);
			}
			else {
				QT_GL3(GetUniformiv)(program, location, ints);
				for (uint32_t i = 0; i < n; ++i) putWord(out + 4 * i, static_cast<uint32_t>(ints[i]));
			}
			break;
		}
		}
		return 0;
	}

	int32_t getActive(const Command& c) {
		GLuint program = static_cast<GLuint>(c.u(2)), index = static_cast<GLuint>(c.u(3));
		GLsizei bufSize = static_cast<GLsizei>(c.u(4));
		uint8_t* lengthOut = c.p(5);
		uint8_t* sizeOut = c.p(6);
		uint8_t* typeOut = c.p(7);
		char* nameOut = c.p(8);
		GLchar dummy[1];
		GLsizei length = 0;
		GLint size = 0;
		GLenum type = 0;
		if (!nameOut || bufSize <= 0) {
			nameOut = dummy;
			bufSize = 1;
		}
		switch (c.u(1)) {
		case QT_OGLES2_A_Attrib: QT_GL3(GetActiveAttrib)(program, index, bufSize, &length, &size, &type, nameOut); break;
		case QT_OGLES2_A_Uniform: QT_GL3(GetActiveUniform)(program, index, bufSize, &length, &size, &type, nameOut); break;
		case QT_OGLES2_A_TransformFeedbackVarying:
			QT_GL3(GetTransformFeedbackVarying)(program, index, bufSize, &length, &size, &type, nameOut);
			break;
		case QT_OGLES2_A_UniformBlockName: QT_GL3(GetActiveUniformBlockName)(program, index, bufSize, &length, nameOut); break;
		}
		if (lengthOut) putWord(lengthOut, static_cast<uint32_t>(length));
		if (sizeOut) putWord(sizeOut, static_cast<uint32_t>(size));
		if (typeOut) putWord(typeOut, type);
		return 0;
	}

	int32_t getAttachedShaders(const Command& c) {
		GLuint program = static_cast<GLuint>(c.u(1));
		GLsizei maxCount = static_cast<GLsizei>(c.u(2));
		uint8_t* countOut = c.p(3);
		uint8_t* shadersOut = c.p(4);
		GLuint shaders[16];
		GLsizei count = 0;
		if (maxCount > 16) maxCount = 16;
		if (maxCount > 0) QT_GL3(GetAttachedShaders)(program, maxCount, &count, shaders);
		if (shadersOut) for (GLsizei i = 0; i < count; ++i) putWord(shadersOut + 4 * i, shaders[i]);
		if (countOut) putWord(countOut, static_cast<uint32_t>(count));
		return 0;
	}

	bool drawBuffers(const Command& c) {
		uint32_t n = c.u(1);
		const uint8_t* in = c.p(2);
		GLenum bufs[16];
		if (!in || n > 16) return true;
		for (uint32_t i = 0; i < n; ++i) bufs[i] = word(in + 4 * i);
		// GL_BACK names framebuffer 0's colour buffer: the object's.
		if (defaultBound(0x8CA9)) for (uint32_t i = 0; i < n; ++i) if (bufs[i] == GL_BACK) bufs[i] = 0x8CE0;
		QT_GL3(DrawBuffers)(static_cast<GLsizei>(n), bufs);
		return true;
	}

	int32_t uniformMatrix4fv(const Command& c) {
		GLint location = static_cast<GLint>(c.u(1));
		uint32_t count = c.u(2);
		const uint8_t* in = c.p(4);
		std::vector<float> m;
		uint32_t i;
		if (!in || count > 64) return 0;
		m.resize(static_cast<size_t>(count) * 16);
		for (i = 0; i < count * 16; ++i) {
			uint32_t bits = (uint32_t(in[4 * i]) << 24) | (uint32_t(in[4 * i + 1]) << 16) |
				(uint32_t(in[4 * i + 2]) << 8) | in[4 * i + 3];
			memcpy(&m[i], &bits, 4);
		}
		QT_GL3(UniformMatrix4fv)(location, static_cast<GLsizei>(count), GL_FALSE, &m[0]);
		return 0;
	}
}

void qt_ogles2_forget_context() {
	ogles2::forgetContext();
}

void qt_ogles2_share_context(uintptr_t gl, uintptr_t with) {
	std::map<uintptr_t, uintptr_t>::const_iterator it = with ? ogles2::groups.find(with) : ogles2::groups.end();
	ogles2::groups[gl] = it != ogles2::groups.end() ? it->second : ogles2::nextGroup++;
}

bool qt_ogles2_decode(const Command& c, int32_t& result) {
	result = 0;
	switch (c.u(0) >> 16) {
	case QT_OGLES2_SHADER_SOURCE:
		if (c.words != QT_OGLES2_SHADER_SOURCE_WORDS) return false;
		return ogles2::shaderSource(c);
	case QT_OGLES2_GEN_BUFFERS:
		if (c.words != QT_OGLES2_GEN_BUFFERS_WORDS) return false;
		return ogles2::genBuffers(c);
	case QT_OGLES2_DELETE_BUFFERS:
		if (c.words != QT_OGLES2_DELETE_BUFFERS_WORDS) return false;
		return ogles2::deleteBuffers(c);
	case QT_OGLES2_BUFFER_DATA:
		if (c.words != QT_OGLES2_BUFFER_DATA_WORDS) return false;
		return ogles2::bufferData(c);
	case QT_OGLES2_DRAW:
		if (c.words != QT_OGLES2_DRAW_WORDS) return false;
		return ogles2::draw(c);
	case QT_OGLES2_SIMPLE:
		if (c.words != QT_OGLES2_SIMPLE_WORDS) return false;
		result = ogles2::simple(c);
		return true;
	case QT_OGLES2_NAME:
		if (c.words != QT_OGLES2_NAME_WORDS) return false;
		result = ogles2::name(c);
		return true;
	case QT_OGLES2_UNIFORM:
		if (c.words != QT_OGLES2_UNIFORM_WORDS) return false;
		result = ogles2::uniform(c);
		return true;
	case QT_OGLES2_GET_IV:
		if (c.words != QT_OGLES2_GET_IV_WORDS) return false;
		result = ogles2::getIv(c);
		return true;
	case QT_OGLES2_GET_LOG:
		if (c.words != QT_OGLES2_GET_LOG_WORDS) return false;
		result = ogles2::getLog(c);
		return true;
	case QT_OGLES2_GEN:
		if (c.words != QT_OGLES2_GEN_WORDS) return false;
		return ogles2::genObjects(c);
	case QT_OGLES2_DELETE:
		if (c.words != QT_OGLES2_DELETE_WORDS) return false;
		return ogles2::deleteObjects(c);
	case QT_OGLES2_CALL:
		if (c.words != QT_OGLES2_CALL_WORDS) return false;
		result = ogles2::call(c);
		return true;
	case QT_OGLES2_TEX_IMAGE_2D:
		if (c.words != QT_OGLES2_TEX_IMAGE_2D_WORDS) return false;
		return ogles2::texImage2D(c);
	case QT_OGLES2_UNIFORM_MATRIX4FV:
		if (c.words != QT_OGLES2_UNIFORM_MATRIX4FV_WORDS) return false;
		result = ogles2::uniformMatrix4fv(c);
		return true;
	case QT_OGLES2_UNIFORM_V:
		if (c.words != QT_OGLES2_UNIFORM_V_WORDS) return false;
		result = ogles2::uniformV(c);
		return true;
	case QT_OGLES2_BUFFER_SUB_DATA:
		if (c.words != QT_OGLES2_BUFFER_SUB_DATA_WORDS) return false;
		return ogles2::bufferSubData(c);
	case QT_OGLES2_READ_BUFFER:
		if (c.words != QT_OGLES2_READ_BUFFER_WORDS) return false;
		return ogles2::readBuffer(c);
	case QT_OGLES2_GET:
		if (c.words != QT_OGLES2_GET_WORDS) return false;
		result = ogles2::get(c);
		return true;
	case QT_OGLES2_GET_PARAM:
		if (c.words != QT_OGLES2_GET_PARAM_WORDS) return false;
		result = ogles2::getParam(c);
		return true;
	case QT_OGLES2_GET_ACTIVE:
		if (c.words != QT_OGLES2_GET_ACTIVE_WORDS) return false;
		result = ogles2::getActive(c);
		return true;
	case QT_OGLES2_GET_ATTACHED_SHADERS:
		if (c.words != QT_OGLES2_GET_ATTACHED_SHADERS_WORDS) return false;
		result = ogles2::getAttachedShaders(c);
		return true;
	case QT_OGLES2_TEX_SUB_IMAGE_2D:
		if (c.words != QT_OGLES2_TEX_SUB_IMAGE_2D_WORDS) return false;
		return ogles2::texSubImage2D(c);
	case QT_OGLES2_READ_PIXELS:
		if (c.words != QT_OGLES2_READ_PIXELS_WORDS) return false;
		return ogles2::readPixels(c);
	case QT_OGLES2_DRAW_BUFFERS:
		if (c.words != QT_OGLES2_DRAW_BUFFERS_WORDS) return false;
		return ogles2::drawBuffers(c);
	// OpenGL ES 3.0 (ogles3.library).
	case QT_OGLES2_TEX_IMAGE_3D:
		if (c.words != QT_OGLES2_TEX_IMAGE_3D_WORDS) return false;
		return ogles2::texImage3D(c);
	case QT_OGLES2_COMPRESSED_TEX:
		if (c.words != QT_OGLES2_COMPRESSED_TEX_WORDS) return false;
		return ogles2::compressedTex(c);
	case QT_OGLES2_TEX_SUB_IMAGE_3D:
		if (c.words != QT_OGLES2_TEX_SUB_IMAGE_3D_WORDS) return false;
		return ogles2::texSubImage3D(c);
	case QT_OGLES2_CLEAR_BUFFER:
		if (c.words != QT_OGLES2_CLEAR_BUFFER_WORDS) return false;
		return ogles2::clearBuffer(c);
	case QT_OGLES2_BIND_BUFFER_RANGE:
		if (c.words != QT_OGLES2_BIND_BUFFER_RANGE_WORDS) return false;
		return ogles2::bindBufferRange(c);
	case QT_OGLES2_COPY_BUFFER_SUB_DATA:
		if (c.words != QT_OGLES2_COPY_BUFFER_SUB_DATA_WORDS) return false;
		return ogles2::copyBufferSubData(c);
	case QT_OGLES2_TRANSFORM_FEEDBACK_VARYINGS:
		if (c.words != QT_OGLES2_TRANSFORM_FEEDBACK_VARYINGS_WORDS) return false;
		return ogles2::transformFeedbackVaryings(c);
	case QT_OGLES2_UNIFORM_INDICES:
		if (c.words != QT_OGLES2_UNIFORM_INDICES_WORDS) return false;
		return ogles2::uniformIndices(c);
	case QT_OGLES2_ACTIVE_UNIFORMS_IV:
		if (c.words != QT_OGLES2_ACTIVE_UNIFORMS_IV_WORDS) return false;
		return ogles2::activeUniformsiv(c);
	// OpenGL ES 3.1 (stage 3b).
	case QT_OGLES2_PROGRAM_UNIFORM_V:
		if (c.words != QT_OGLES2_PROGRAM_UNIFORM_V_WORDS) return false;
		result = ogles2::programUniformV(c);
		return true;
	case QT_OGLES2_PROGRAM_RESOURCE:
		if (c.words != QT_OGLES2_PROGRAM_RESOURCE_WORDS) return false;
		result = ogles2::programResource(c);
		return true;
	}
	return false;
}
