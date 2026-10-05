// Warp3D on the host (docs/phase4-warp3d-on-host.md). Executes the Warp3D
// commands Warp3D.library writes to the command buffer, with the OpenGL calls
// the 68k code of QuarkTex 0.53 made, minus the bugs phase 5 fixed. Since
// phase 6 those OpenGL 1.1 calls go to the emulation in ffp.h, which draws
// on an OpenGL 3.3 core context.
//
// The unit test (tests/host) includes this file with QT_TEST defined and
// QT_GL pointing to recording stubs.
#ifndef QT_TEST
#include "ffp.h"
#define QT_GL(name) ffp::name
#endif
#include <cstdio>
#include <map>
#include <string>
#include <vector>
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
		W3D_FOGGING = 1 << 14, W3D_LOGICOP = 1 << 20, W3D_STENCILBUFFER = 1 << 21, W3D_ALPHATEST = 1 << 22,
		W3D_SCISSOR = 1 << 25, W3D_CHROMATEST = 1 << 26,
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
	const GLenum w3dstencil[] = {0, GL_NEVER, GL_ALWAYS, GL_LESS, GL_LEQUAL, GL_EQUAL, GL_GEQUAL, GL_GREATER, GL_NOTEQUAL};
	const GLenum w3dstencilop[] = {0, GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_DECR, GL_INVERT};

	// Texture.c: W3D texture format to OpenGL format and type, and whether the
	// 16/32-bit texels need their bytes swapped.
	const GLint swapFormat[] = {0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0};
	const GLenum formats[] = {0, GL_COLOR_INDEX, GL_BGRA_EXT, GL_RGB, GL_RGB, GL_BGRA_EXT, GL_BGRA_EXT, GL_ALPHA, GL_LUMINANCE,
		GL_LUMINANCE_ALPHA, GL_INTENSITY, GL_RGBA};
	const GLenum types[] = {0, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_BYTE,
		GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE,
		GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE};
	const GLint bytesPerPixel[] = {0, 1, 2, 2, 3, 2, 4, 1, 1, 2, 1, 4};
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
	void writeU32(const Command& c, uint32_t address, uint32_t value) {
		uint8_t* p = address ? static_cast<uint8_t*>(c.resolve(address)) : 0;
		if (p) for (int i = 0; i < 4; ++i) p[i] = static_cast<uint8_t>(value >> (24 - 8 * i));
	}
	void writeDouble(const Command& c, uint32_t address, double value) {
		uint8_t* p = address ? static_cast<uint8_t*>(c.resolve(address)) : 0;
		uint64_t bits;
		memcpy(&bits, &value, 8);
		if (p) for (int i = 0; i < 8; ++i) p[i] = static_cast<uint8_t>(bits >> (56 - 8 * i));
	}

	// CHUNKY textures: 8-bit indices into a palette of 256 ARGB words. OpenGL
	// gets them as RGBA bytes (0.53 sent the indices as GL_COLOR_INDEX and
	// ignored the palette). The palette is kept with the texture (ffp, per
	// context), so updates without a palette use the last one; empty if
	// there was none yet.
	const uint32_t W3D_CHUNKY = 1;
	const std::vector<uint32_t> noPalette;

	const std::vector<uint32_t>& paletteOf(GLuint name) {
		const std::vector<uint32_t>* palette = QT_GL(TexturePalette)(name);
		return palette ? *palette : noPalette;
	}

	void readPalette(const Command& c, GLuint name, uint32_t address) {
		std::vector<uint32_t>* palette = QT_GL(TexturePalette)(name);
		if (!palette) return;
		palette->resize(256);
		for (uint32_t i = 0; i < 256; ++i) (*palette)[i] = readU32(c, address + 4 * i);
	}

	std::vector<GLubyte> chunkyToRgba(const Command& c, const std::vector<uint32_t>& palette, uint32_t image,
			int32_t width, int32_t height, uint32_t bytesPerRow) {
		std::vector<GLubyte> rgba(width > 0 && height > 0 ? static_cast<size_t>(width) * height * 4 : 0);
		if (!bytesPerRow) bytesPerRow = static_cast<uint32_t>(width);
		size_t out = 0;
		for (int32_t y = 0; y < height; ++y) {
			for (int32_t x = 0; x < width; ++x) {
				uint32_t argb = palette.size() == 256 ? palette[readU8(c, image + y * bytesPerRow + x)] : 0;
				rgba[out++] = static_cast<GLubyte>(argb >> 16);
				rgba[out++] = static_cast<GLubyte>(argb >> 8);
				rgba[out++] = static_cast<GLubyte>(argb);
				rgba[out++] = static_cast<GLubyte>(argb >> 24);
			}
		}
		return rgba;
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
			if (qt_w3d_trace_all && n < 4) {
				uint32_t v = a.vertex + i * a.vertexStride, t = a.texCoord + i * a.texStride;
				char line[200];
				snprintf(line, sizeof(line), "  vertex %u: %g %g %g  uvw %g %g %g  color %08X", i, readFloat(c, v), readFloat(c, v + 4),
					readFloat(c, v + 8), readFloat(c, t), readFloat(c, t + a.texV), readFloat(c, t + a.texW), readU32(c, a.color + i * a.colorStride));
				qt_report(line);
				if (n == 0) { // the first vertex record as words, to see its layout
					std::string words;
					for (uint32_t k = 0; k < a.vertexStride && k < 128; k += 4) {
						char word[12];
						snprintf(word, sizeof(word), " %08X", readU32(c, v + k));
						words += word;
					}
					qt_report(("  record:" + words).c_str());
				}
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
			case W3D_STENCILBUFFER: QT_GL(Enable)(GL_STENCIL_TEST); break;
			case W3D_CHROMATEST: QT_GL(ChromaTest)(GL_TRUE); break;
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
			case W3D_STENCILBUFFER: QT_GL(Disable)(GL_STENCIL_TEST); break;
			case W3D_CHROMATEST: QT_GL(ChromaTest)(GL_FALSE); break;
			}
		}
	}

	// One colour channel of an ARGB word, as Drawing.c computed it.
	float channel(uint32_t color, int shift) {
		return static_cast<float>((color >> shift) & 0xFF) / 256;
	}
}

void qt_w3d_sync() {
#ifndef QT_TEST
	ffp::flush();
#endif
}

// Tracing for finding out how an application draws (QUARKTEX_TRACE, see
// host/quarktex.cpp): the texture commands, or all commands, are logged with
// their first argument words and their result.
bool qt_w3d_trace_textures = false;
bool qt_w3d_trace_all = false;

namespace {
	bool decode(const Command& c, int32_t& result);

	bool textureCommand(uint32_t opcode) {
		return (opcode >= QT_W3D_TEX_ALLOC && opcode <= QT_W3D_TEX_UPDATE && opcode != QT_W3D_DRAW_ARRAY) || opcode == QT_W3D_CHROMA;
	}

	void trace(const Command& c, bool ok, int32_t result) {
		char line[320];
		int length = snprintf(line, sizeof(line), "W3D %04X:", c.u(0) >> 16);
		for (uint32_t i = 1; i < c.words && i <= 21 && length < 300; ++i) length += snprintf(line + length, sizeof(line) - length, " %X", c.u(i));
		snprintf(line + length, sizeof(line) - length, "%s -> %d%s", c.words > 22 ? " ..." : "", result, ok ? "" : " (bad)");
		qt_report(line);
	}
}

bool qt_w3d_decode(const Command& c, int32_t& result) {
	bool ok = decode(c, result);
	if (qt_w3d_trace_all || (qt_w3d_trace_textures && textureCommand(c.u(0) >> 16))) trace(c, ok, result);
	return ok;
}

namespace {
bool decode(const Command& c, int32_t& result) {
	result = 0;
#ifndef QT_TEST
	if (!ffp::active()) return false; // a Warp3D command in an agl context
#endif
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
		uint32_t palette = c.u(5);
		if (c.words != 6) return false;
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
		if (format == W3D_CHUNKY && palette) {
			int32_t width = static_cast<int32_t>(c.u(2)), height = static_cast<int32_t>(c.u(3));
			readPalette(c, name, palette);
			std::vector<GLubyte> rgba = chunkyToRgba(c, paletteOf(name), c.u(4), width, height, 0);
			QT_GL(TexImage2D)(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
				rgba.empty() ? 0 : &rgba[0]);
		}
		else {
			QT_GL(TexImage2D)(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) (int32_t) c.u(2), (GLsizei) (int32_t) c.u(3), 0,
				glFormat, glType, static_cast<GLvoid*>(c.p(4)));
		}
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
		// The colour as r, g, b, a (0.53 passed it as r, b, g, a).
		uint32_t environment = c.u(2);
		GLint mode = 0;
		GLfloat color[4] = {c.f(3), c.f(4), c.f(5), c.f(6)};
		if (c.words != 7) return false;
		QT_GL(BindTexture)(GL_TEXTURE_2D, c.u(1));
		if (environment && lookup(envs, environment, mode)) QT_GL(TexEnvi)(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
		if (environment == W3D_BLEND) QT_GL(TexEnvfv)(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
		return true;
	}

	case QT_W3D_TEX_WRAP: {
		// The border colour as r, g, b, a (0.53 passed it as r, b, g, a).
		GLfloat color[4] = {c.f(4), c.f(5), c.f(6), c.f(7)};
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

	case QT_W3D_POINT_SIZE: {
		// 0.53 ignored the size; sizes below one pixel draw as one pixel.
		float size = c.f(1);
		if (c.words != 2) return false;
		QT_GL(PointSize)(size >= 1.0f ? size : 1.0f);
		return true;
	}

	case QT_W3D_LINE_WIDTH: {
		float width = c.f(1);
		if (c.words != 2) return false;
		QT_GL(LineWidth)(width >= 1.0f ? width : 1.0f);
		return true;
	}

	case QT_W3D_TEX_UPDATE: {
		GLenum glFormat = 0, glType = 0;
		GLint pixelSize = 0;
		uint32_t bytesPerRow = c.u(8), palette = c.u(9);
		GLuint name = c.u(1);
		if (c.words != 10) return false;
		lookup(formats, c.u(2), glFormat);
		lookup(types, c.u(2), glType);
		lookup(bytesPerPixel, c.u(2), pixelSize);
		QT_GL(BindTexture)(GL_TEXTURE_2D, name);
		if (c.u(2) == W3D_CHUNKY && (palette || !paletteOf(name).empty())) {
			int32_t width = static_cast<int32_t>(c.u(5)), height = static_cast<int32_t>(c.u(6));
			if (palette) readPalette(c, name, palette);
			std::vector<GLubyte> rgba = chunkyToRgba(c, paletteOf(name), c.u(7), width, height, bytesPerRow);
			QT_GL(TexSubImage2D)(GL_TEXTURE_2D, 0, (GLint) (int32_t) c.u(3), (GLint) (int32_t) c.u(4), width, height,
				GL_RGBA, GL_UNSIGNED_BYTE, rgba.empty() ? 0 : &rgba[0]);
			return true;
		}
		// The byte order of this format (0.53 left it as the last allocation
		// had set it).
		GLint swap = 0;
		lookup(swapFormat, c.u(2), swap);
		QT_GL(PixelStorei)(GL_UNPACK_SWAP_BYTES, swap ? GL_TRUE : GL_FALSE);
		bool rows = bytesPerRow && pixelSize && bytesPerRow % pixelSize == 0;
		if (rows) QT_GL(PixelStorei)(GL_UNPACK_ROW_LENGTH, static_cast<GLint>(bytesPerRow / pixelSize));
		QT_GL(TexSubImage2D)(GL_TEXTURE_2D, 0, (GLint) (int32_t) c.u(3), (GLint) (int32_t) c.u(4),
			(GLsizei) (int32_t) c.u(5), (GLsizei) (int32_t) c.u(6), glFormat, glType, static_cast<GLvoid*>(c.p(7)));
		if (rows) QT_GL(PixelStorei)(GL_UNPACK_ROW_LENGTH, 0);
		return true;
	}

	case QT_W3D_READ_Z: {
		// The depth buffer holds (z + 1) / 2 (Warp3D z goes to OpenGL
		// unprojected); Warp3D gets 2 * depth - 1 as big-endian doubles.
		uint32_t count = c.u(3), address = c.u(4);
		if (c.words != 5) return false;
		if (!count || count > QT_W3D_MAX_DEPTH_SPAN) return true;
		std::vector<GLfloat> depth(count);
		QT_GL(ReadPixels)((GLint) (int32_t) c.u(1), (GLint) (int32_t) c.u(2), (GLsizei) count, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[0]);
		for (uint32_t i = 0; i < count; ++i) writeDouble(c, address + 8 * i, 2.0 * depth[i] - 1.0);
		return true;
	}

	case QT_W3D_WRITE_Z: {
		// One point per pixel at its centre; with a mask only where it is set.
		int32_t x = static_cast<int32_t>(c.u(1)), y = static_cast<int32_t>(c.u(2));
		uint32_t count = c.u(3), address = c.u(4), mask = c.u(5);
		if (c.words != 6) return false;
		if (count > QT_W3D_MAX_DEPTH_SPAN) return true;
		std::vector<GLfloat> points;
		for (uint32_t i = 0; i < count; ++i) {
			if (mask && !readU8(c, mask + i)) continue;
			points.push_back(static_cast<GLfloat>(x + static_cast<int32_t>(i)) + 0.5f);
			points.push_back(static_cast<GLfloat>(y) + 0.5f);
			points.push_back(static_cast<GLfloat>((readDouble(c, address + 8 * i) + 1.0) / 2.0));
		}
		if (!points.empty()) QT_GL(DepthPoints)(static_cast<GLsizei>(points.size() / 3), &points[0]);
		return true;
	}

	case QT_W3D_STENCIL_FUNC: {
		GLenum function;
		if (c.words != 4) return false;
		if (lookup(w3dstencil, c.u(1), function)) QT_GL(StencilFunc)(function, (GLint) c.u(2), c.u(3));
		return true;
	}

	case QT_W3D_STENCIL_OP: {
		GLenum sfail, dpfail, dppass;
		if (c.words != 4) return false;
		if (lookup(w3dstencilop, c.u(1), sfail) && lookup(w3dstencilop, c.u(2), dpfail) && lookup(w3dstencilop, c.u(3), dppass)) {
			QT_GL(StencilOp)(sfail, dpfail, dppass);
		}
		return true;
	}

	case QT_W3D_CHROMA: {
		// Warp3D modes: W3D_CHROMATEST_NONE 1, _INCLUSIVE 2, _EXCLUSIVE 3.
		uint32_t mode = c.u(4);
		if (c.words != 5) return false;
		if (mode >= 1 && mode <= 3) QT_GL(ChromaBounds)(c.u(1), static_cast<GLint>(mode - 1), c.u(2), c.u(3));
		return true;
	}

	case QT_W3D_STENCIL_MASK:
		if (c.words != 2) return false;
		QT_GL(StencilMask)(c.u(1));
		return true;

	case QT_W3D_STENCIL_CLEAR:
		if (c.words != 2) return false;
		QT_GL(ClearStencil)((GLint) c.u(1));
		QT_GL(Clear)(GL_STENCIL_BUFFER_BIT);
		return true;

	case QT_W3D_READ_STENCIL: {
		uint32_t count = c.u(3), address = c.u(4);
		if (c.words != 5) return false;
		if (!count || count > QT_W3D_MAX_DEPTH_SPAN) return true;
		std::vector<GLuint> values(count);
		QT_GL(ReadPixels)((GLint) (int32_t) c.u(1), (GLint) (int32_t) c.u(2), (GLsizei) count, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &values[0]);
		for (uint32_t i = 0; i < count; ++i) writeU32(c, address + 4 * i, values[i]);
		return true;
	}

	case QT_W3D_WRITE_STENCIL: {
		// One point per pixel at its centre. As with glDrawPixels, values are
		// taken modulo the 8 stencil bits.
		int32_t x = static_cast<int32_t>(c.u(1)), y = static_cast<int32_t>(c.u(2));
		uint32_t width = c.u(3), height = c.u(4), bytes = c.u(5), data = c.u(6), mask = c.u(7);
		if (c.words != 8) return false;
		if ((bytes != 1 && bytes != 2 && bytes != 4) || static_cast<uint64_t>(width) * height > QT_W3D_MAX_STENCIL_PIXELS) return true;
		std::vector<GLfloat> points;
		std::vector<GLuint> values;
		for (uint32_t row = 0; row < height; ++row) {
			for (uint32_t column = 0; column < width; ++column) {
				if (mask && !readU8(c, mask + column)) continue;
				uint32_t address = data + (row * width + column) * bytes;
				uint32_t value = bytes == 1 ? readU8(c, address) : bytes == 2 ? readU16(c, address) : readU32(c, address);
				points.push_back(static_cast<GLfloat>(x + static_cast<int32_t>(column)) + 0.5f);
				points.push_back(static_cast<GLfloat>(y + static_cast<int32_t>(row)) + 0.5f);
				values.push_back(value & 0xFF);
			}
		}
		if (!values.empty()) QT_GL(StencilPoints)(static_cast<GLsizei>(values.size()), &points[0], &values[0]);
		return true;
	}
	}
	return false;
}
}
