// Commands of QuarkTex's minigl.library (gl/mglcmd.h), executed in its OpenGL
// compatibility context.
//
// QT_MGL_DRAW: the application's vertex arrays are read from Amiga memory
// here, converted from big-endian into host arrays and drawn with OpenGL's
// own vertex arrays, so the 68k does not touch them vertex by vertex (it did
// until phase 7 stage 4, sending them as immediate mode).
//
// GL_ARB_multitexture: the OpenGL 1.3 functions are looked up when first
// used, in the context that executes the command.
//
// The unit test (tests/host) includes this file with QT_TEST defined and
// QT_GL and QT_GL13 pointing to recording stubs.
#ifndef QT_TEST
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#define QT_GL(name) gl##name
#define QT_GL13(name) mgl::gl13::name
#endif
#include <vector>
#include "gldecode.h"
#include "mglcmd.h"

#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif

namespace mgl {
#ifndef QT_TEST
	namespace gl13 {
		typedef void (APIENTRY* UnitFunction)(GLenum unit);
		typedef void (APIENTRY* TexCoordFunction)(GLenum unit, GLfloat s, GLfloat t);
		UnitFunction activeTexture, clientActiveTexture;
		TexCoordFunction multiTexCoord2f;

		bool load() {
			static bool loaded = false;
			if (!loaded) {
				loaded = true;
				activeTexture = reinterpret_cast<UnitFunction>(reinterpret_cast<void*>(wglGetProcAddress("glActiveTexture")));
				clientActiveTexture = reinterpret_cast<UnitFunction>(reinterpret_cast<void*>(wglGetProcAddress("glClientActiveTexture")));
				multiTexCoord2f = reinterpret_cast<TexCoordFunction>(reinterpret_cast<void*>(wglGetProcAddress("glMultiTexCoord2f")));
			}
			return activeTexture && clientActiveTexture && multiTexCoord2f;
		}

		void ActiveTexture(GLenum unit) { if (load()) activeTexture(unit); }
		void ClientActiveTexture(GLenum unit) { if (load()) clientActiveTexture(unit); }
		void MultiTexCoord2f(GLenum unit, GLfloat s, GLfloat t) { if (load()) multiTexCoord2f(unit, s, t); }
	}
#endif

	struct Array {
		bool enabled;
		uint32_t size, type, stride, address;
	};

	uint32_t typeSize(uint32_t type) {
		switch (type) {
		case GL_UNSIGNED_BYTE: case GL_BYTE: return 1;
		case GL_UNSIGNED_SHORT: case GL_SHORT: return 2;
		case GL_DOUBLE: return 8;
		}
		return 4;
	}

	// Component k of element i, big-endian in Amiga memory, as a float.
	float component(const uint8_t* base, const Array& a, uint32_t i, uint32_t k) {
		const uint8_t* p = base + i * a.stride + k * typeSize(a.type);
		switch (a.type) {
		case GL_FLOAT: case GL_INT: case GL_UNSIGNED_INT: {
			uint32_t bits = (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
			if (a.type == GL_INT) return static_cast<float>(static_cast<int32_t>(bits));
			if (a.type == GL_UNSIGNED_INT) return static_cast<float>(bits);
			float value;
			memcpy(&value, &bits, 4);
			return value;
		}
		case GL_DOUBLE: {
			uint64_t bits = 0;
			for (int b = 0; b < 8; ++b) bits = (bits << 8) | p[b];
			double value;
			memcpy(&value, &bits, 8);
			return static_cast<float>(value);
		}
		case GL_SHORT: return static_cast<float>(static_cast<int16_t>((p[0] << 8) | p[1]));
		case GL_UNSIGNED_SHORT: return static_cast<float>((p[0] << 8) | p[1]);
		case GL_BYTE: return static_cast<float>(static_cast<int8_t>(p[0]));
		}
		return p[0];
	}

	// Elements 0 to count - 1 of the array as floats (colours of unsigned
	// bytes stay bytes), size components each.
	struct Converted {
		std::vector<float> floats;
		std::vector<uint8_t> bytes;
	};

	bool convert(const Command& c, const Array& a, uint32_t count, Converted& out) {
		const uint8_t* base = static_cast<const uint8_t*>(c.resolve(a.address));
		if (!base || a.size < 1 || a.size > 4) return false;
		if (a.type == GL_UNSIGNED_BYTE) {
			out.bytes.resize(static_cast<size_t>(count) * a.size);
			for (uint32_t i = 0; i < count; ++i) memcpy(&out.bytes[i * a.size], base + i * a.stride, a.size);
			return true;
		}
		out.floats.resize(static_cast<size_t>(count) * a.size);
		for (uint32_t i = 0; i < count; ++i)
			for (uint32_t k = 0; k < a.size; ++k) out.floats[i * a.size + k] = component(base, a, i, k);
		return true;
	}

	Array array(const Command& c, int word) {
		Array a = {c.u(word) != 0, c.u(word + 1), c.u(word + 2), c.u(word + 3), c.u(word + 4)};
		if (a.enabled && !a.address) a.enabled = false;
		return a;
	}

	// The arrays in QT_MGL_DRAW's order.
	enum { Vertices, Colours, TexCoords0, TexCoords1, Arrays };

	void setArray(int k, bool on, GLint size, GLenum type, const void* data) {
		static const GLenum states[Arrays] = {GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY, GL_TEXTURE_COORD_ARRAY};
		if (k >= TexCoords0) QT_GL13(ClientActiveTexture)(GL_TEXTURE0 + (k - TexCoords0));
		if (!on) {
			QT_GL(DisableClientState)(states[k]);
			return;
		}
		QT_GL(EnableClientState)(states[k]);
		void* pointer = const_cast<void*>(data);
		if (k == Vertices) QT_GL(VertexPointer)(size, type, 0, pointer);
		else if (k == Colours) QT_GL(ColorPointer)(size, type, 0, pointer);
		else QT_GL(TexCoordPointer)(size, type, 0, pointer);
	}

	bool draw(const Command& c) {
		GLenum mode = c.u(1);
		uint32_t first = c.u(2), count = c.u(3), indexType = c.u(4), indexAddress = c.u(5);
		Array arrays[Arrays];
		for (int k = 0; k < Arrays; ++k) arrays[k] = array(c, 6 + 5 * k);
		if (!count || !arrays[Vertices].enabled) return true;

		// The indices, and how many elements of the arrays they reach.
		std::vector<uint32_t> indices;
		uint32_t elements = first + count;
		if (indexType) {
			const uint8_t* p = static_cast<const uint8_t*>(c.resolve(indexAddress));
			uint32_t size = typeSize(indexType);
			if (!p || count > QT_MGL_MAX_VERTICES) return true;
			indices.resize(count);
			elements = 0;
			for (uint32_t n = 0; n < count; ++n) {
				const uint8_t* q = p + n * size;
				uint32_t i = size == 1 ? q[0] : size == 2 ? (uint32_t(q[0]) << 8) | q[1]
					: (uint32_t(q[0]) << 24) | (uint32_t(q[1]) << 16) | (uint32_t(q[2]) << 8) | q[3];
				indices[n] = i;
				if (i + 1 > elements) elements = i + 1;
			}
		}
		if (elements > QT_MGL_MAX_VERTICES) return true;

		Converted converted[Arrays];
		bool on[Arrays];
		for (int k = 0; k < Arrays; ++k) on[k] = arrays[k].enabled && convert(c, arrays[k], elements, converted[k]);
		if (!on[Vertices]) return true;
		QT_GL(PushClientAttrib)(GL_CLIENT_VERTEX_ARRAY_BIT);
		for (int k = 0; k < Arrays; ++k) {
			bool bytes = !converted[k].bytes.empty();
			const void* data = !on[k] ? 0 : bytes ? static_cast<const void*>(&converted[k].bytes[0]) : static_cast<const void*>(&converted[k].floats[0]);
			setArray(k, on[k], static_cast<GLint>(arrays[k].size), bytes ? GL_UNSIGNED_BYTE : GL_FLOAT, data);
		}
		QT_GL(DisableClientState)(GL_NORMAL_ARRAY);
		QT_GL(DisableClientState)(GL_INDEX_ARRAY);
		QT_GL(DisableClientState)(GL_EDGE_FLAG_ARRAY);
		if (indexType) QT_GL(DrawElements)(mode, static_cast<GLsizei>(count), GL_UNSIGNED_INT, &indices[0]);
		else QT_GL(DrawArrays)(mode, static_cast<GLint>(first), static_cast<GLsizei>(count));
		QT_GL(PopClientAttrib)();
		return true;
	}

	bool validUnit(uint32_t unit) {
		return unit >= GL_TEXTURE0 && unit < GL_TEXTURE0 + QT_MGL_TEXTURE_UNITS;
	}
}

bool qt_mgl_decode(const Command& c, int32_t& result) {
	result = 0;
	switch (c.u(0) >> 16) {
	case QT_MGL_DRAW:
		if (c.words != QT_MGL_DRAW_WORDS) return false;
		return mgl::draw(c);
	case QT_MGL_ACTIVE_TEXTURE:
		if (c.words != QT_MGL_ACTIVE_TEXTURE_WORDS) return false;
		if (mgl::validUnit(c.u(1))) QT_GL13(ActiveTexture)(c.u(1));
		return true;
	case QT_MGL_MULTI_TEX_COORD:
		if (c.words != QT_MGL_MULTI_TEX_COORD_WORDS) return false;
		if (mgl::validUnit(c.u(1))) QT_GL13(MultiTexCoord2f)(c.u(1), c.f(2), c.f(3));
		return true;
	}
	return false;
}
