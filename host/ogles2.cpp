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
// shares that file's component()-style conversion.
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
}
#define QT_GL3(name) gl3test::name
#endif
#include <cstdio>
#include <cstring>
#include <map>
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
		bool enabled;
		uint32_t size, type, stride, bufferName, pointer; // pointer: offset into the buffer, or an Amiga address
		bool normalised;
	};

	Attrib readAttrib(const Command& c, int word) {
		Attrib a;
		a.enabled = c.u(word) != 0;
		a.size = c.u(word + 1);
		a.type = c.u(word + 2);
		a.normalised = c.u(word + 3) != 0;
		a.stride = c.u(word + 4);
		a.bufferName = c.u(word + 5);
		a.pointer = c.u(word + 6);
		return a;
	}

	// The attribute's data as a flat host float array, elements 0 to count - 1,
	// size components each; false if it names a buffer QuarkTex does not
	// know, or an Amiga address of 0.
	bool convert(const Command& c, const Attrib& a, uint32_t count, std::vector<float>& out) {
		const uint8_t* base;
		uint32_t stride = a.stride ? a.stride : a.size * typeSize(a.type);
		if (a.bufferName) {
			const std::vector<uint8_t>* buffer = findBuffer(a.bufferName);
			if (!buffer || a.pointer + stride * (count ? count - 1 : 0) + a.size * typeSize(a.type) > buffer->size()) return false;
			base = &(*buffer)[a.pointer];
		}
		else {
			if (!a.pointer) return false;
			base = static_cast<const uint8_t*>(c.resolve(a.pointer));
			if (!base) return false;
		}
		out.resize(static_cast<size_t>(count) * a.size);
		for (uint32_t i = 0; i < count; ++i)
			for (uint32_t k = 0; k < a.size; ++k) out[i * a.size + k] = component(base, a.type, stride, a.normalised, i, k);
		return true;
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
	std::map<GLuint, std::string> sources;

	std::string coreSource(const std::string& source, bool fragment) {
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
		sources[shader] = source;
		std::string core = coreSource(source, type == 0x8B30); // GL_FRAGMENT_SHADER
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
		}
		return true;
	}

	// The buffer currently bound to GL_ARRAY_BUFFER/GL_ELEMENT_ARRAY_BUFFER,
	// so glBufferData knows which one gets the data; glVertexAttribPointer's
	// and glDrawElements' own buffer names travel with each QT_OGLES2_DRAW
	// instead (host/mgl.cpp's minigl.library keeps no state like this at all,
	// since its arrays are never buffer-backed).
	uint32_t arrayBuffer, elementArrayBuffer;

	bool bindBuffer(const Command& c) {
		uint32_t target = c.u(1), buffer = c.u(2);
		if (target == GL_ARRAY_BUFFER) arrayBuffer = buffer;
		else if (target == GL_ELEMENT_ARRAY_BUFFER) elementArrayBuffer = buffer;
		return true;
	}

	bool bufferData(const Command& c) {
		uint32_t target = c.u(1), size = c.u(2);
		const uint8_t* data = c.p(3);
		uint32_t name = target == GL_ELEMENT_ARRAY_BUFFER ? elementArrayBuffer : arrayBuffer;
		if (!name) return true;
		std::vector<uint8_t>& buffer = buffers[name];
		buffer.resize(size);
		if (data && size) memcpy(&buffer[0], data, size);
		usages[name] = c.u(4);
		return true;
	}

	// The bound fake buffer of target, or 0.
	std::vector<uint8_t>* boundBuffer(uint32_t target) {
		uint32_t name = target == GL_ELEMENT_ARRAY_BUFFER ? elementArrayBuffer : arrayBuffer;
		std::map<uint32_t, std::vector<uint8_t>>::iterator it = buffers.find(name);
		return name && it != buffers.end() ? &it->second : 0;
	}

	bool bufferSubData(const Command& c) {
		std::vector<uint8_t>* buffer = boundBuffer(c.u(1));
		uint32_t offset = c.u(2), size = c.u(3);
		const uint8_t* data = c.p(4);
		if (!buffer || !data || offset > buffer->size() || size > buffer->size() - offset) return true;
		if (size) memcpy(&(*buffer)[offset], data, size);
		return true;
	}

	bool readBuffer(const Command& c) {
		std::vector<uint8_t>* buffer = boundBuffer(c.u(1));
		uint32_t offset = c.u(2), size = c.u(3);
		uint8_t* out = c.p(4);
		if (!buffer || !out || offset > buffer->size() || size > buffer->size() - offset) return true;
		if (size) memcpy(out, &(*buffer)[offset], size);
		return true;
	}

	// A real, temporary host buffer object for the converted data: plain
	// client-side pointers (no GL_ARRAY_BUFFER bound) reliably hung this
	// Intel driver's glDrawArrays under a core profile, even with a VAO
	// bound and the pointer and size both correct; uploading into a real
	// buffer object, the ordinary modern-GL way, does not.
	GLuint tempBuffers[QT_OGLES2_MAX_ATTRIBS];

	void setArray(int index, const Attrib& a, bool on, const std::vector<float>& data) {
		if (!on) {
			QT_GL3(DisableVertexAttribArray)(static_cast<GLuint>(index));
			return;
		}
		QT_GL3(GenBuffers)(1, &tempBuffers[index]);
		QT_GL3(BindBuffer)(GL_ARRAY_BUFFER, tempBuffers[index]);
		QT_GL3(BufferData)(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size() * sizeof(float)), &data[0], GL_STREAM_DRAW);
		QT_GL3(EnableVertexAttribArray)(static_cast<GLuint>(index));
		QT_GL3(VertexAttribPointer)(static_cast<GLuint>(index), static_cast<GLint>(a.size), GL_FLOAT, GL_FALSE, 0, 0);
	}

	bool draw(const Command& c) {
		GLenum mode = static_cast<GLenum>(c.u(1));
		uint32_t first = c.u(2), count = c.u(3), indexType = c.u(4), indexBuffer = c.u(5), indexPointer = c.u(6);
		Attrib attribs[QT_OGLES2_MAX_ATTRIBS];
		for (int i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) attribs[i] = readAttrib(c, 7 + 7 * i);
		if (!count) return true;

		// Elements: read the indices (big-endian, converted here, not with
		// everyone else's component()) and find how many array elements they
		// reach, as minigl.library's draw does (host/mgl.cpp).
		std::vector<uint32_t> indices;
		uint32_t elements = first + count;
		if (indexType) {
			const uint8_t* base;
			if (indexBuffer) {
				const std::vector<uint8_t>* buffer = findBuffer(indexBuffer);
				uint32_t size = typeSize(indexType);
				if (!buffer || indexPointer + count * size > buffer->size()) return true;
				base = &(*buffer)[indexPointer];
			}
			else {
				if (!indexPointer) return true;
				base = static_cast<const uint8_t*>(c.resolve(indexPointer));
				if (!base) return true;
			}
			// first: the base vertex (glDrawElementsBaseVertexOES), 0 otherwise.
			int32_t baseVertex = static_cast<int32_t>(first);
			uint32_t size = typeSize(indexType);
			indices.resize(count);
			elements = 0;
			for (uint32_t n = 0; n < count; ++n) {
				const uint8_t* q = base + n * size;
				int64_t v = size == 1 ? q[0] : size == 2 ? (uint32_t(q[0]) << 8) | q[1] : word(q);
				v += baseVertex;
				if (v < 0) return true;
				indices[n] = static_cast<uint32_t>(v);
				if (indices[n] + 1 > elements) elements = indices[n] + 1;
			}
		}

		std::vector<float> converted[QT_OGLES2_MAX_ATTRIBS];
		bool on[QT_OGLES2_MAX_ATTRIBS];
		for (int i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i)
			on[i] = attribs[i].enabled && elements && convert(c, attribs[i], elements, converted[i]);
		for (int i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) setArray(i, attribs[i], on[i], converted[i]);
		if (indexType) {
			std::vector<uint32_t> indices32(indices.begin(), indices.end());
			QT_GL(DrawElements)(mode, static_cast<GLsizei>(count), GL_UNSIGNED_INT, &indices32[0]);
		}
		else QT_GL(DrawArrays)(mode, static_cast<GLint>(first), static_cast<GLsizei>(count));
		for (int i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) {
			if (!on[i]) continue;
			QT_GL3(DisableVertexAttribArray)(static_cast<GLuint>(i));
			QT_GL3(DeleteBuffers)(1, &tempBuffers[i]);
		}
		return true;
	}

	int32_t simple(const Command& c) {
		uint32_t w0 = c.u(2), w1 = c.u(3);
		switch (c.u(1)) {
		case QT_OGLES2_S_CreateShader: return static_cast<int32_t>(QT_GL3(CreateShader)(static_cast<GLenum>(w0)));
		case QT_OGLES2_S_DeleteShader:
			QT_GL3(DeleteShader)(static_cast<GLuint>(w0));
			sources.erase(static_cast<GLuint>(w0));
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
		}
		return 0;
	}

	int32_t getIv(const Command& c) {
		GLuint id = static_cast<GLuint>(c.u(2));
		GLenum pname = static_cast<GLenum>(c.u(3));
		uint8_t* out = c.p(4);
		GLint value = 0;
		if (c.u(1) == QT_OGLES2_G_Shader) {
			std::map<GLuint, std::string>::const_iterator it = sources.find(id);
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
				std::map<GLuint, std::string>::const_iterator it = sources.find(id);
				if (it != sources.end()) {
					length = static_cast<GLsizei>(it->second.size() < static_cast<size_t>(bufSize - 1) ? it->second.size() : bufSize - 1);
					memcpy(textOut, it->second.data(), length);
					textOut[length] = 0;
				}
				else QT_GL3(GetShaderSource)(id, bufSize, &length, textOut);
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
		case QT_OGLES2_K_Framebuffer: QT_GL3(DeleteFramebuffers)(static_cast<GLsizei>(n), names); break;
		case QT_OGLES2_K_Renderbuffer: QT_GL3(DeleteRenderbuffers)(static_cast<GLsizei>(n), names); break;
		}
		return true;
	}

	int32_t call(const Command& c) {
		uint32_t w0 = c.u(2), w1 = c.u(3), w2 = c.u(4), w3 = c.u(5), w4 = c.u(6);
		switch (c.u(1)) {
		case QT_OGLES2_C_BindTexture: QT_GL(BindTexture)(static_cast<GLenum>(w0), static_cast<GLuint>(w1)); break;
		case QT_OGLES2_C_ActiveTexture: QT_GL3(ActiveTexture)(static_cast<GLenum>(w0)); break;
		case QT_OGLES2_C_GenerateMipmap: QT_GL3(GenerateMipmap)(static_cast<GLenum>(w0)); break;
		case QT_OGLES2_C_TexParameteri:
			QT_GL(TexParameteri)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), static_cast<GLint>(w2));
			break;
		case QT_OGLES2_C_BindFramebuffer: QT_GL3(BindFramebuffer)(static_cast<GLenum>(w0), static_cast<GLuint>(w1)); break;
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
		}
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
		if (type == GL_UNSIGNED_BYTE) return 1;
		if (packed16(type) || type == GL_UNSIGNED_SHORT) return 2;
		if (type == GL_UNSIGNED_INT || type == 0x84FA) return 4; // GL_UNSIGNED_INT_24_8
		return 0;
	}

	uint32_t pixelBytes(GLenum format, GLenum type) {
		if (packed16(type)) return 2;
		if (type == 0x84FA) return 4;
		uint32_t components;
		switch (format) {
		case GL_LUMINANCE_ALPHA: case GL_RG: components = 2; break;
		case GL_RGB: components = 3; break;
		case GL_RGBA: case 0x80E1: components = 4; break; // GL_BGRA
		default: components = 1; // GL_ALPHA, GL_LUMINANCE, GL_RED, GL_DEPTH_COMPONENT
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
			std::vector<uint8_t>& converted, const void*& pixels) {
		uint32_t unit = swapUnit(type);
		pixels = data;
		if (!unit) return false;
		if (unit == 1 || !data) return true;
		GLint alignment = 4;
		QT_GL(GetIntegerv)(GL_UNPACK_ALIGNMENT, &alignment);
		converted.assign(data, data + imageSize(width, height, pixelBytes(format, type), alignment));
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

	// glUniform*v and glUniformMatrix{2,3,4}fv: count vectors/matrices of
	// big-endian words, swapped into host order.
	int32_t uniformV(const Command& c) {
		uint32_t subop = c.u(1);
		GLint location = static_cast<GLint>(c.u(2));
		uint32_t count = c.u(3);
		const uint8_t* in = c.p(4);
		static const uint32_t components[] = {1, 2, 3, 4, 1, 2, 3, 4, 4, 9, 16};
		if (!in || subop > QT_OGLES2_U_M4 || count > 1024) return 0;
		uint32_t n = count * components[subop];
		std::vector<uint32_t> v(n ? n : 1);
		for (uint32_t i = 0; i < n; ++i) v[i] = word(in + 4 * i);
		GLsizei k = static_cast<GLsizei>(count);
		const GLfloat* f = reinterpret_cast<const GLfloat*>(&v[0]);
		const GLint* iv = reinterpret_cast<const GLint*>(&v[0]);
		switch (subop) {
		case QT_OGLES2_U_1F: QT_GL3(Uniform1fv)(location, k, f); break;
		case QT_OGLES2_U_2F: QT_GL3(Uniform2fv)(location, k, f); break;
		case QT_OGLES2_U_3F: QT_GL3(Uniform3fv)(location, k, f); break;
		case QT_OGLES2_U_4F: QT_GL3(Uniform4fv)(location, k, f); break;
		case QT_OGLES2_U_1I: QT_GL3(Uniform1iv)(location, k, iv); break;
		case QT_OGLES2_U_2I: QT_GL3(Uniform2iv)(location, k, iv); break;
		case QT_OGLES2_U_3I: QT_GL3(Uniform3iv)(location, k, iv); break;
		case QT_OGLES2_U_4I: QT_GL3(Uniform4iv)(location, k, iv); break;
		case QT_OGLES2_U_M2: QT_GL3(UniformMatrix2fv)(location, k, GL_FALSE, f); break;
		case QT_OGLES2_U_M3: QT_GL3(UniformMatrix3fv)(location, k, GL_FALSE, f); break;
		case QT_OGLES2_U_M4: QT_GL3(UniformMatrix4fv)(location, k, GL_FALSE, f); break;
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
		case 0x86A3: case 0x8DF8: // COMPRESSED_TEXTURE_FORMATS, SHADER_BINARY_FORMATS: none offered
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
		case 0x86A2: case 0x8DF9: value = 0; return true; // NUM_COMPRESSED_TEXTURE_FORMATS, NUM_SHADER_BINARY_FORMATS
		case 0x8DFA: value = 1; return true; // SHADER_COMPILER
		case 0x8B9A: value = GL_UNSIGNED_BYTE; return true; // IMPLEMENTATION_COLOR_READ_TYPE
		case 0x8B9B: value = GL_RGBA; return true; // IMPLEMENTATION_COLOR_READ_FORMAT
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
		case QT_OGLES2_P_FramebufferAttachmentParameteriv:
			QT_GL3(GetFramebufferAttachmentParameteriv)(static_cast<GLenum>(w0), static_cast<GLenum>(w1),
				static_cast<GLenum>(w2), ints);
			putWord(out, static_cast<uint32_t>(ints[0]));
			break;
		case QT_OGLES2_P_BufferParameteriv: {
			uint32_t name = w0 == GL_ELEMENT_ARRAY_BUFFER ? elementArrayBuffer : arrayBuffer;
			std::vector<uint8_t>* buffer = boundBuffer(w0);
			uint32_t value = 0;
			if (w1 == 0x8764) value = buffer ? static_cast<uint32_t>(buffer->size()) : 0; // GL_BUFFER_SIZE
			else if (w1 == 0x8765) value = usages.count(name) ? usages[name] : 0x88E4; // GL_BUFFER_USAGE, default GL_STATIC_DRAW
			putWord(out, value);
			break;
		}
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
		if (c.u(1) == QT_OGLES2_A_Attrib) QT_GL3(GetActiveAttrib)(program, index, bufSize, &length, &size, &type, nameOut);
		else QT_GL3(GetActiveUniform)(program, index, bufSize, &length, &size, &type, nameOut);
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
	case QT_OGLES2_BIND_BUFFER:
		if (c.words != QT_OGLES2_BIND_BUFFER_WORDS) return false;
		return ogles2::bindBuffer(c);
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
	}
	return false;
}
