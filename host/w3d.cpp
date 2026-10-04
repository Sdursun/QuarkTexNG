// Warp3D on the host (docs/phase4-warp3d-on-host.md). Executes the Warp3D
// commands Warp3D.library writes to the command buffer, with the OpenGL calls
// the 68k code of QuarkTex 0.53 made - including its quirks, so that the
// reference tests keep matching 0.53. Fixes belong to phase 5.
//
// The unit test (tests/host) includes this file with QT_TEST defined and
// QT_GL pointing to recording stubs.
#ifndef QT_TEST
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#define QT_GL(name) gl##name
#endif
#include "gldecode.h"
#include "w3dcmd.h"

// OpenGL 1.2 blend factors and packed pixel types; the Windows headers stop
// at OpenGL 1.1.
#ifndef GL_CONSTANT_COLOR
#define GL_CONSTANT_COLOR 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_CONSTANT_ALPHA 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#endif
#ifndef GL_UNSIGNED_SHORT_5_6_5
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#endif
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

namespace {
	// Warp3D.library/Warp3D.h
	enum {
		W3D_TEXMAPPING = 1 << 8, W3D_PERSPECTIVE = 1 << 9, W3D_GOURAUD = 1 << 10,
		W3D_ZBUFFER = 1 << 11, W3D_ZBUFFERUPDATE = 1 << 12, W3D_BLENDING = 1 << 13,
		W3D_FOGGING = 1 << 14, W3D_LOGICOP = 1 << 20, W3D_ALPHATEST = 1 << 22,
		W3D_SCISSOR = 1 << 25,
		W3D_ENABLE = 1
	};

	// Warp3D enumerations to OpenGL, index = Warp3D value (Effect.c, ZBuffer.c).
	const GLenum w3dalpha[] = {0, GL_NEVER, GL_LESS, GL_GEQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_EQUAL, GL_ALWAYS};
	const GLenum w3dblend[] = {0, GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA_SATURATE,
		GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA};
	const GLint w3dfog[] = {0, GL_LINEAR, GL_EXP, GL_EXP2, GL_EXP2}; // W3D_FOG_INTERPOLATED as EXP2
	const GLenum w3dlogic[] = {0, GL_CLEAR, GL_AND, GL_AND_REVERSE, GL_COPY, GL_AND_INVERTED, GL_NOOP, GL_XOR, GL_OR,
		GL_NOR, GL_EQUIV, GL_INVERT, GL_OR_REVERSE, GL_COPY_INVERTED, GL_OR_INVERTED, GL_NAND, GL_SET};
	const GLenum w3dz[] = {0, GL_NEVER, GL_LESS, GL_GEQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_EQUAL, GL_ALWAYS};

	// Texture.c: W3D texture format to OpenGL format and type, and whether the
	// 16/32-bit texels need their bytes swapped.
	const GLint swapFormat[] = {0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0};
	const GLenum formats[] = {0, GL_COLOR_INDEX, GL_BGRA_EXT, GL_RGB, GL_RGB, GL_BGRA_EXT, GL_BGRA_EXT, GL_ALPHA, GL_LUMINANCE,
		GL_LUMINANCE_ALPHA, GL_INTENSITY, GL_RGBA};
	const GLenum types[] = {0, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_BYTE,
		GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE,
		GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE};
	const GLint envs[] = {0, GL_REPLACE, GL_DECAL, GL_MODULATE, GL_BLEND};
	const uint32_t W3D_BLEND = 4;

	template <typename T, size_t N> bool lookup(const T (&table)[N], uint32_t index, T& value) {
		if (index >= N) return false;
		value = table[index];
		return true;
	}

	// The primitive between DRAW_BEGIN and DRAW_END.
	struct Draw {
		uint32_t state;
		bool textured;
		float width, height; // of the texture
	} draw;

	// One W3D_Vertex starting at word i (Warp3D.h: x, y, z (double), w, u, v,
	// tex3d, color r g b a, spec r g b, l). Same as w3d.c drawVertex in 0.53.
	void drawVertex(const Command& c, int i) {
		if ((draw.state & W3D_TEXMAPPING) && draw.textured) {
			float u = c.f(i + 5), v = c.f(i + 6), w = c.f(i + 4);
			if (draw.state & W3D_PERSPECTIVE) QT_GL(TexCoord4f)(u * w / draw.width, v * w / draw.height, 0.0f, w);
			else QT_GL(TexCoord2f)(static_cast<float>(static_cast<double>(u) / draw.width), static_cast<float>(static_cast<double>(v) / draw.height));
		}
		if (draw.state & W3D_GOURAUD) QT_GL(Color4f)(c.f(i + 8), c.f(i + 9), c.f(i + 10), c.f(i + 11));
		// Fog needs the depth too (0.53 only sent it with the z-buffer). Without
		// either, z may be unset and must not clip the vertex away.
		if (draw.state & (W3D_ZBUFFER | W3D_FOGGING)) QT_GL(Vertex3f)(c.f(i), c.f(i + 1), static_cast<float>(c.d(i + 2)));
		else QT_GL(Vertex2f)(c.f(i), c.f(i + 1));
	}

	// Words 1-6 of DRAW_BEGIN and DRAW: primitive, state, textured, texture
	// name, width, height.
	void beginDraw(const Command& c) {
		draw.state = c.u(2);
		draw.textured = c.u(3) != 0;
		draw.width = static_cast<float>(static_cast<int32_t>(c.u(5)));
		draw.height = static_cast<float>(static_cast<int32_t>(c.u(6)));
		if ((draw.state & W3D_TEXMAPPING) && draw.textured) QT_GL(BindTexture)(GL_TEXTURE_2D, c.u(4));
		QT_GL(Begin)(c.u(1));
	}

	// Big-endian values in Amiga memory; address 0 reads as 0.
	uint32_t readU32(const Command& c, uint32_t address) {
		const uint8_t* p = address ? static_cast<const uint8_t*>(c.resolve(address)) : 0;
		return p ? (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3] : 0;
	}
	uint32_t readU16(const Command& c, uint32_t address) {
		const uint8_t* p = address ? static_cast<const uint8_t*>(c.resolve(address)) : 0;
		return p ? (uint32_t(p[0]) << 8) | p[1] : 0;
	}
	uint32_t readU8(const Command& c, uint32_t address) {
		const uint8_t* p = address ? static_cast<const uint8_t*>(c.resolve(address)) : 0;
		return p ? p[0] : 0;
	}
	float readFloat(const Command& c, uint32_t address) {
		uint32_t bits = readU32(c, address);
		float value;
		memcpy(&value, &bits, 4);
		return value;
	}
	double readDouble(const Command& c, uint32_t address) {
		uint64_t bits = (uint64_t(readU32(c, address)) << 32) | readU32(c, address + 4);
		double value;
		memcpy(&value, &bits, 8);
		return value;
	}

	// The arrays of a DRAW_ARRAY command (words 7-17), V4Array.c in 0.53.
	struct Arrays {
		uint32_t vertex, vertexStride, vertexMode;
		uint32_t color, colorStride, colorMode;
		uint32_t texCoord, texStride, texV, texW, texFlags;
	};

	enum {
		W3D_VERTEX_F_F_F = 0, W3D_VERTEX_F_F_D = 1, W3D_VERTEX_D_D_D = 2,
		W3D_COLOR_FLOAT = 1u << 30, W3D_COLOR_UBYTE = 2u << 30,
		W3D_CMODE_RGB = 0x01, W3D_CMODE_BGR = 0x02, W3D_CMODE_RGBA = 0x04, W3D_CMODE_ARGB = 0x08, W3D_CMODE_BGRA = 0x10,
		W3D_TEXCOORD_NORMALIZED = 1,
		W3D_INDEX_UBYTE = 0, W3D_INDEX_UWORD = 1, W3D_INDEX_ULONG = 2
	};

	const GLenum primitives[] = {GL_TRIANGLES, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP};

	void arrayColor(const Command& c, const Arrays& a, uint32_t i) {
		if (!a.color || !(draw.state & W3D_GOURAUD)) return;
		uint32_t p = a.color + i * a.colorStride;
		if (a.colorMode & W3D_COLOR_FLOAT) {
			float f[4];
			for (int k = 0; k < 4; ++k) f[k] = readFloat(c, p + 4 * k);
			if (a.colorMode & W3D_CMODE_RGB) QT_GL(Color3f)(f[0], f[1], f[2]);
			else if (a.colorMode & W3D_CMODE_BGR) QT_GL(Color3f)(f[2], f[1], f[0]);
			else if (a.colorMode & W3D_CMODE_RGBA) QT_GL(Color4f)(f[0], f[1], f[2], f[3]);
			else if (a.colorMode & W3D_CMODE_ARGB) QT_GL(Color4f)(f[1], f[2], f[3], f[0]);
			else if (a.colorMode & W3D_CMODE_BGRA) QT_GL(Color4f)(f[2], f[1], f[0], f[3]);
		}
		else if (a.colorMode & W3D_COLOR_UBYTE) {
			GLubyte b[4];
			for (int k = 0; k < 4; ++k) b[k] = static_cast<GLubyte>(readU8(c, p + k));
			if (a.colorMode & W3D_CMODE_RGB) QT_GL(Color3ub)(b[0], b[1], b[2]);
			else if (a.colorMode & W3D_CMODE_BGR) QT_GL(Color3ub)(b[2], b[1], b[0]);
			else if (a.colorMode & W3D_CMODE_RGBA) QT_GL(Color4ub)(b[0], b[1], b[2], b[3]);
			else if (a.colorMode & W3D_CMODE_ARGB) QT_GL(Color4ub)(b[1], b[2], b[3], b[0]);
			else if (a.colorMode & W3D_CMODE_BGRA) QT_GL(Color4ub)(b[2], b[1], b[0], b[3]);
		}
	}

	void arrayTexCoord(const Command& c, const Arrays& a, uint32_t i) {
		if (!(draw.state & W3D_TEXMAPPING) || !a.texCoord) return;
		uint32_t p = a.texCoord + i * a.texStride;
		float u = readFloat(c, p), v = readFloat(c, p + a.texV), w = readFloat(c, p + a.texW);
		if (a.texFlags & W3D_TEXCOORD_NORMALIZED) QT_GL(TexCoord4f)(u * w, v * w, 0.0f, w);
		else QT_GL(TexCoord4f)(u * w / draw.width, v * w / draw.height, 0.0f, w);
	}

	void arrayVertex(const Command& c, const Arrays& a, uint32_t i) {
		if (!a.vertex) return;
		uint32_t p = a.vertex + i * a.vertexStride;
		switch (a.vertexMode) {
		case W3D_VERTEX_F_F_F:
			QT_GL(Vertex3f)(readFloat(c, p), readFloat(c, p + 4), readFloat(c, p + 8));
			break;
		case W3D_VERTEX_F_F_D: // x and y as floats, z as the double at offset 8
			QT_GL(Vertex3f)(readFloat(c, p), readFloat(c, p + 4), static_cast<float>(readDouble(c, p + 8)));
			break;
		case W3D_VERTEX_D_D_D:
			QT_GL(Vertex3f)(static_cast<float>(readDouble(c, p)), static_cast<float>(readDouble(c, p + 8)),
				static_cast<float>(readDouble(c, p + 16)));
			break;
		}
	}

	bool drawArray(const Command& c) {
		Arrays a = {c.u(7), c.u(8), c.u(9), c.u(10), c.u(11), c.u(12), c.u(13), c.u(14), c.u(15), c.u(16), c.u(17)};
		uint32_t indexType = c.u(18), indices = c.u(19), first = c.u(20), count = c.u(21);
		GLenum primitive;
		if (!lookup(primitives, c.u(1), primitive)) return true; // 0.53 read past its table
		draw.state = c.u(2);
		draw.textured = c.u(3) != 0;
		draw.width = static_cast<float>(static_cast<int32_t>(c.u(5)));
		draw.height = static_cast<float>(static_cast<int32_t>(c.u(6)));
		if ((draw.state & W3D_TEXMAPPING) && draw.textured) QT_GL(BindTexture)(GL_TEXTURE_2D, c.u(4));
		if (indexType != QT_W3D_NO_INDEX && indexType > W3D_INDEX_ULONG) return true; // 0.53 drew nothing
		QT_GL(Begin)(primitive);
		for (uint32_t n = 0; n < count; ++n) {
			uint32_t i;
			switch (indexType) {
			case W3D_INDEX_UBYTE: i = readU8(c, indices + n); break;
			case W3D_INDEX_UWORD: i = readU16(c, indices + 2 * n); break;
			case W3D_INDEX_ULONG: i = readU32(c, indices + 4 * n); break;
			default: i = first + n; break;
			}
			arrayColor(c, a, i);
			arrayTexCoord(c, a, i);
			arrayVertex(c, a, i);
		}
		QT_GL(End)();
		return true;
	}

	// Context.c W3D_SetState in 0.53, with W3D_ZBUFFERUPDATE switching depth
	// writes (0.53 had a missing break there and switched blending instead).
	void setState(uint32_t state, bool enable) {
		if (state == W3D_ZBUFFERUPDATE) {
			QT_GL(DepthMask)(enable ? GL_TRUE : GL_FALSE);
			return;
		}
		if (enable) {
			switch (state) {
			case W3D_TEXMAPPING: QT_GL(Enable)(GL_TEXTURE_2D); break;
			case W3D_GOURAUD: QT_GL(ShadeModel)(GL_SMOOTH); break;
			case W3D_ZBUFFER: QT_GL(Enable)(GL_DEPTH_TEST); break;
			case W3D_BLENDING: QT_GL(Enable)(GL_BLEND); break;
			case W3D_FOGGING: QT_GL(Enable)(GL_FOG); break;
			case W3D_LOGICOP: QT_GL(Enable)(GL_COLOR_LOGIC_OP); break;
			case W3D_ALPHATEST: QT_GL(Enable)(GL_ALPHA_TEST); break;
			case W3D_SCISSOR: QT_GL(Enable)(GL_SCISSOR_TEST); break;
			}
		}
		else {
			switch (state) {
			case W3D_TEXMAPPING: QT_GL(Disable)(GL_TEXTURE_2D); break;
			case W3D_GOURAUD: QT_GL(ShadeModel)(GL_FLAT); break;
			case W3D_ZBUFFER: QT_GL(Disable)(GL_DEPTH_TEST); break;
			case W3D_BLENDING: QT_GL(Disable)(GL_BLEND); break;
			case W3D_FOGGING: QT_GL(Disable)(GL_FOG); break;
			case W3D_LOGICOP: QT_GL(Disable)(GL_COLOR_LOGIC_OP); break;
			case W3D_ALPHATEST: QT_GL(Disable)(GL_ALPHA_TEST); break;
			case W3D_SCISSOR: QT_GL(Disable)(GL_SCISSOR_TEST); break;
			}
		}
	}

	// One colour channel of an ARGB word, as Drawing.c computed it.
	float channel(uint32_t color, int shift) {
		return static_cast<float>((color >> shift) & 0xFF) / 256;
	}
}

bool qt_w3d_decode(const Command& c, int32_t& result) {
	result = 0;
	switch (c.u(0) >> 16) {
	case QT_W3D_INIT_CONTEXT:
		if (c.words != 1) return false;
		QT_GL(Enable)(GL_TEXTURE_2D);
		QT_GL(ShadeModel)(GL_SMOOTH);
		return true;

	case QT_W3D_DRAW_BEGIN:
		if (c.words != 7) return false;
		beginDraw(c);
		return true;

	case QT_W3D_DRAW: {
		uint32_t count = c.u(7);
		if (c.words < 8 || count > QT_W3D_MAX_VERTICES || c.words != 8 + count * QT_W3D_VERTEX_WORDS) return false;
		beginDraw(c);
		for (uint32_t v = 0; v < count; ++v) drawVertex(c, 8 + v * QT_W3D_VERTEX_WORDS);
		QT_GL(End)();
		return true;
	}

	case QT_W3D_VERTICES: {
		uint32_t count = c.u(1);
		if (count > QT_W3D_MAX_VERTICES || c.words != 2 + count * QT_W3D_VERTEX_WORDS) return false;
		for (uint32_t v = 0; v < count; ++v) drawVertex(c, 2 + v * QT_W3D_VERTEX_WORDS);
		return true;
	}

	case QT_W3D_DRAW_END:
		if (c.words != 1) return false;
		QT_GL(End)();
		return true;

	case QT_W3D_SET_STATE:
		if (c.words != 3) return false;
		setState(c.u(1), c.u(2) == W3D_ENABLE);
		return true;

	case QT_W3D_BLEND_MODE: {
		GLenum source, destination;
		if (c.words != 3) return false;
		if (lookup(w3dblend, c.u(1), source) && lookup(w3dblend, c.u(2), destination)) QT_GL(BlendFunc)(source, destination);
		return true;
	}

	case QT_W3D_ALPHA_MODE: {
		GLenum function;
		if (c.words != 3) return false;
		if (lookup(w3dalpha, c.u(1), function)) QT_GL(AlphaFunc)(function, c.f(2));
		return true;
	}

	case QT_W3D_FOG: {
		// Effect.c kept the colour in a global array whose alpha stays 0.
		GLfloat color[4] = {c.f(5), c.f(6), c.f(7), 0.0f};
		GLint mode;
		if (c.words != 8) return false;
		QT_GL(Fogf)(GL_FOG_DENSITY, c.f(4));
		QT_GL(Fogf)(GL_FOG_START, c.f(2));
		QT_GL(Fogf)(GL_FOG_END, c.f(3));
		QT_GL(Fogfv)(GL_FOG_COLOR, color);
		if (lookup(w3dfog, c.u(1), mode)) QT_GL(Fogi)(GL_FOG_MODE, mode);
		return true;
	}

	case QT_W3D_Z_COMPARE: {
		GLenum function;
		if (c.words != 2) return false;
		if (lookup(w3dz, c.u(1), function)) QT_GL(DepthFunc)(function);
		return true;
	}

	case QT_W3D_LOGIC_OP: {
		GLenum operation;
		if (c.words != 2) return false;
		if (lookup(w3dlogic, c.u(1), operation)) QT_GL(LogicOp)(operation);
		return true;
	}

	case QT_W3D_COLOR_MASK:
		if (c.words != 5) return false;
		QT_GL(ColorMask)((GLboolean) c.u(1), (GLboolean) c.u(2), (GLboolean) c.u(3), (GLboolean) c.u(4));
		return true;

	case QT_W3D_CURRENT_COLOR:
		if (c.words != 5) return false;
		QT_GL(Color4f)(c.f(1), c.f(2), c.f(3), c.f(4));
		return true;

	case QT_W3D_SCISSOR:
		if (c.words != 5) return false;
		QT_GL(Scissor)((GLint) (int32_t) c.u(1), (GLint) (int32_t) c.u(2), (GLsizei) (int32_t) c.u(3), (GLsizei) (int32_t) c.u(4));
		return true;

	case QT_W3D_CLEAR: {
		// Drawing.c: a real clear in fullscreen, a rectangle in the current
		// state (texturing, blending, ...) in a window.
		uint32_t color = c.u(1);
		if (c.words != 5) return false;
		if (c.u(2)) {
			QT_GL(ClearColor)(channel(color, 16), channel(color, 8), channel(color, 0), channel(color, 24));
			QT_GL(Clear)(GL_COLOR_BUFFER_BIT);
		}
		else {
			QT_GL(Color4f)(channel(color, 16), channel(color, 8), channel(color, 0), channel(color, 24));
			QT_GL(Recti)(0, 0, (GLint) (int32_t) c.u(3), (GLint) (int32_t) c.u(4));
		}
		return true;
	}

	case QT_W3D_CLEAR_Z:
		if (c.words != 1) return false;
		QT_GL(Clear)(GL_DEPTH_BUFFER_BIT);
		return true;

	case QT_W3D_TEX_ALLOC: {
		// Texture.c W3D_AllocTexObj in 0.53.
		uint32_t format = c.u(1);
		GLint swap = 0;
		GLenum glFormat = 0, glType = 0;
		GLuint name = 0;
		if (c.words != 5) return false;
		lookup(swapFormat, format, swap);
		lookup(formats, format, glFormat);
		lookup(types, format, glType);
		QT_GL(PixelStorei)(GL_UNPACK_SWAP_BYTES, swap ? GL_TRUE : GL_FALSE);
		QT_GL(PixelStorei)(GL_UNPACK_ALIGNMENT, 1);
		QT_GL(GenTextures)(1, &name);
		QT_GL(BindTexture)(GL_TEXTURE_2D, name);
		QT_GL(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		QT_GL(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		QT_GL(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		QT_GL(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		QT_GL(TexImage2D)(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) (int32_t) c.u(2), (GLsizei) (int32_t) c.u(3), 0,
			glFormat, glType, static_cast<GLvoid*>(c.p(4)));
		result = static_cast<int32_t>(name);
		return true;
	}

	case QT_W3D_TEX_FREE: {
		GLuint name = c.u(1);
		if (c.words != 2) return false;
		if (name) QT_GL(DeleteTextures)(1, &name);
		return true;
	}

	case QT_W3D_TEX_FILTER:
		if (c.words != 4) return false;
		QT_GL(BindTexture)(GL_TEXTURE_2D, c.u(1));
		QT_GL(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint) c.u(2));
		QT_GL(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint) c.u(3));
		return true;

	case QT_W3D_TEX_ENV: {
		// Texture.c passed the colour as r, b, g, a.
		uint32_t environment = c.u(2);
		GLint mode = 0;
		GLfloat color[4] = {c.f(3), c.f(5), c.f(4), c.f(6)};
		if (c.words != 7) return false;
		QT_GL(BindTexture)(GL_TEXTURE_2D, c.u(1));
		if (environment && lookup(envs, environment, mode)) QT_GL(TexEnvi)(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
		if (environment == W3D_BLEND) QT_GL(TexEnvfv)(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
		return true;
	}

	case QT_W3D_TEX_WRAP: {
		// Texture.c passed the border colour as r, b, g, a.
		GLfloat color[4] = {c.f(4), c.f(6), c.f(5), c.f(7)};
		if (c.words != 8) return false;
		QT_GL(BindTexture)(GL_TEXTURE_2D, c.u(1));
		if (c.u(2)) QT_GL(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint) c.u(2));
		if (c.u(3)) QT_GL(TexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint) c.u(3));
		QT_GL(TexParameterfv)(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
		return true;
	}

	case QT_W3D_DRAW_ARRAY:
		if (c.words != 22) return false;
		return drawArray(c);

	case QT_W3D_TEX_UPDATE: {
		GLenum glFormat = 0, glType = 0;
		if (c.words != 8) return false;
		lookup(formats, c.u(2), glFormat);
		lookup(types, c.u(2), glType);
		QT_GL(BindTexture)(GL_TEXTURE_2D, c.u(1));
		QT_GL(TexSubImage2D)(GL_TEXTURE_2D, 0, (GLint) (int32_t) c.u(3), (GLint) (int32_t) c.u(4),
			(GLsizei) (int32_t) c.u(5), (GLsizei) (int32_t) c.u(6), glFormat, glType, static_cast<GLvoid*>(c.p(7)));
		return true;
	}
	}
	return false;
}
