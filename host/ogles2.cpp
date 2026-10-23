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
}
#define QT_GL3(name) gl3test::name
#endif
#include <cstdio>
#include <cstring>
#include <map>
#include <vector>
#include "gldecode.h"
#include "oglescmd.h"

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif

namespace ogles2 {
	// A fake buffer: the bytes glBufferData was given, as they are (big
	// endian). 0 is never a valid name (GLES2's "no buffer").
	std::map<uint32_t, std::vector<uint8_t>> buffers;
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
		}
		return 0;
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

	bool shaderSource(const Command& c) {
		GLuint shader = static_cast<GLuint>(c.u(1));
		const char* source = static_cast<const char*>(c.resolve(c.u(2)));
		if (!source) return true;
		QT_GL3(ShaderSource)(shader, 1, &source, 0);
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
			uint32_t size = typeSize(indexType);
			indices.resize(count);
			elements = 0;
			for (uint32_t n = 0; n < count; ++n) {
				const uint8_t* q = base + n * size;
				uint32_t v = size == 1 ? q[0] : size == 2 ? (uint32_t(q[0]) << 8) | q[1] : q[0];
				indices[n] = v;
				if (v + 1 > elements) elements = v + 1;
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
		case QT_OGLES2_S_DeleteShader: QT_GL3(DeleteShader)(static_cast<GLuint>(w0)); return 0;
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
		if (c.u(1) == QT_OGLES2_G_Shader) QT_GL3(GetShaderiv)(id, pname, &value);
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
			QT_GL3(RenderbufferStorage)(static_cast<GLenum>(w0), static_cast<GLenum>(w1), static_cast<GLsizei>(w2),
				static_cast<GLsizei>(w3));
			break;
		}
		return 0;
	}

	// GL_UNSIGNED_BYTE in every known caller -- byte-granular, so the pixel
	// data needs no order conversion, forwarded as given (data may be NULL,
	// a render target's own storage, allocated by the driver).
	bool texImage2D(const Command& c) {
		GLenum target = static_cast<GLenum>(c.u(1));
		GLint level = static_cast<GLint>(c.u(2)), internalformat = static_cast<GLint>(c.u(3));
		GLsizei width = static_cast<GLsizei>(c.u(4)), height = static_cast<GLsizei>(c.u(5));
		GLint border = static_cast<GLint>(c.u(6));
		GLenum format = static_cast<GLenum>(c.u(7)), type = static_cast<GLenum>(c.u(8));
		const uint8_t* data = c.p(9);
		if (type != GL_UNSIGNED_BYTE) {
			qt_report("ogles2: GLTexImage2D: only GL_UNSIGNED_BYTE pixels are supported so far");
			return true;
		}
		QT_GL(TexImage2D)(target, level, internalformat, width, height, border, format, type, const_cast<uint8_t*>(data));
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
	}
	return false;
}
