// Commands of QuarkTex's minigl.library (gl/mglcmd.h), executed in its OpenGL
// compatibility context.
//
// QT_MGL_DRAW: the application's vertex arrays are read from Amiga memory
// here, converted from big-endian into host arrays and drawn with OpenGL's
// own vertex arrays, so the 68k does not touch them vertex by vertex (it did
// until phase 7 stage 4, sending them as immediate mode).
//
// GL_ARB_multitexture, glBlendEquation and glBlendFuncSeparate: the
// functions past OpenGL 1.1 are looked up when first used, in the context
// that executes the command (QT_GL13, whatever their version).
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
#include <cstdio>
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
		typedef void (APIENTRY* FactorsFunction)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
		UnitFunction activeTexture, clientActiveTexture, blendEquation;
		TexCoordFunction multiTexCoord2f;
		FactorsFunction blendFuncSeparate;

		template <typename T> T lookUp(const char* name) {
			return reinterpret_cast<T>(reinterpret_cast<void*>(wglGetProcAddress(name)));
		}

		void load() {
			static bool loaded = false;
			if (loaded) return;
			loaded = true;
			activeTexture = lookUp<UnitFunction>("glActiveTexture");
			clientActiveTexture = lookUp<UnitFunction>("glClientActiveTexture");
			multiTexCoord2f = lookUp<TexCoordFunction>("glMultiTexCoord2f");
			blendEquation = lookUp<UnitFunction>("glBlendEquation");
			blendFuncSeparate = lookUp<FactorsFunction>("glBlendFuncSeparate");
		}

		void ActiveTexture(GLenum unit) { load(); if (activeTexture) activeTexture(unit); }
		void ClientActiveTexture(GLenum unit) { load(); if (clientActiveTexture) clientActiveTexture(unit); }
		void MultiTexCoord2f(GLenum unit, GLfloat s, GLfloat t) { load(); if (multiTexCoord2f) multiTexCoord2f(unit, s, t); }
		void BlendEquation(GLenum mode) { load(); if (blendEquation) blendEquation(mode); }
		void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
			load();
			if (blendFuncSeparate) blendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
		}
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

	// Tracing (QUARKTEX_TRACE_FRAME): what a draw gets, for finding out why
	// something does not show: the arrays, the first vertices as read, the
	// matrices and the state that decides whether fragments are written.
	void traceDraw(const Command& c, const Array* arrays, const Converted* converted, const std::vector<uint32_t>& indices) {
		char line[400];
		int n = snprintf(line, sizeof(line), "  arrays");
		for (int k = 0; k < Arrays; ++k)
			n += snprintf(line + n, sizeof(line) - n, " [%u %u %X %u %X]", arrays[k].enabled ? 1u : 0u, arrays[k].size,
				arrays[k].type, arrays[k].stride, arrays[k].address);
		qt_report(line);
		for (int v = 0; v < 3; ++v) {
			uint32_t i = indices.empty() ? c.u(2) + v : (static_cast<size_t>(v) < indices.size() ? indices[v] : 0);
			n = snprintf(line, sizeof(line), "  vertex %u:", i);
			for (int k = 0; k < Arrays; ++k) {
				const Converted& a = converted[k];
				uint32_t size = arrays[k].size;
				if (!a.floats.empty() && (i + 1) * size <= a.floats.size())
					for (uint32_t j = 0; j < size; ++j) n += snprintf(line + n, sizeof(line) - n, " %g", a.floats[i * size + j]);
				else if (!a.bytes.empty() && (i + 1) * size <= a.bytes.size())
					for (uint32_t j = 0; j < size; ++j) n += snprintf(line + n, sizeof(line) - n, " %u", a.bytes[i * size + j]);
				n += snprintf(line + n, sizeof(line) - n, " |");
			}
			qt_report(line);
		}
		const GLenum matrices[2] = {GL_MODELVIEW_MATRIX, GL_PROJECTION_MATRIX};
		for (int m = 0; m < 2; ++m) {
			GLfloat f[16];
			QT_GL(GetFloatv)(matrices[m], f);
			n = snprintf(line, sizeof(line), m ? "  projection" : "  modelview");
			for (int j = 0; j < 16; ++j) n += snprintf(line + n, sizeof(line) - n, " %g", f[j]);
			qt_report(line);
		}
		GLint depthFunc = 0, blendSrc = 0, blendDst = 0, alphaFunc = 0, depthMask = 0, colorMask[4] = {0, 0, 0, 0};
		GLfloat alphaRef = 0, depthRange[2] = {0, 0}, color[4] = {0, 0, 0, 0};
		QT_GL(GetIntegerv)(GL_DEPTH_FUNC, &depthFunc);
		QT_GL(GetIntegerv)(GL_BLEND_SRC, &blendSrc);
		QT_GL(GetIntegerv)(GL_BLEND_DST, &blendDst);
		QT_GL(GetIntegerv)(GL_ALPHA_TEST_FUNC, &alphaFunc);
		QT_GL(GetFloatv)(GL_ALPHA_TEST_REF, &alphaRef);
		QT_GL(GetIntegerv)(GL_DEPTH_WRITEMASK, &depthMask);
		QT_GL(GetIntegerv)(GL_COLOR_WRITEMASK, colorMask);
		QT_GL(GetFloatv)(GL_DEPTH_RANGE, depthRange);
		QT_GL(GetFloatv)(GL_CURRENT_COLOR, color);
		snprintf(line, sizeof(line), "  state: blend %d (%X %X) alpha test %d (%X %g) depth test %d (%X, mask %d, range %g %g) "
			"cull %d texture %d colour mask %d%d%d%d current colour %g %g %g %g",
			QT_GL(IsEnabled)(GL_BLEND), blendSrc, blendDst, QT_GL(IsEnabled)(GL_ALPHA_TEST), alphaFunc, alphaRef,
			QT_GL(IsEnabled)(GL_DEPTH_TEST), depthFunc, depthMask, depthRange[0], depthRange[1], QT_GL(IsEnabled)(GL_CULL_FACE),
			QT_GL(IsEnabled)(GL_TEXTURE_2D), colorMask[0], colorMask[1], colorMask[2], colorMask[3], color[0], color[1], color[2], color[3]);
		qt_report(line);
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
		if (qt_w3d_trace_all) traceDraw(c, arrays, converted, indices);
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
	case QT_MGL_BLEND_EQUATION:
		if (c.words != QT_MGL_BLEND_EQUATION_WORDS) return false;
		QT_GL13(BlendEquation)(c.u(1));
		return true;
	case QT_MGL_BLEND_FUNC_SEPARATE:
		if (c.words != QT_MGL_BLEND_FUNC_SEPARATE_WORDS) return false;
		QT_GL13(BlendFuncSeparate)(c.u(1), c.u(2), c.u(3), c.u(4));
		return true;
	}
	return false;
}
