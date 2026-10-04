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

// OpenGL 1.2 blend factors; the Windows headers stop at OpenGL 1.1.
#ifndef GL_CONSTANT_COLOR
#define GL_CONSTANT_COLOR 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_CONSTANT_ALPHA 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
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
		if (draw.state & W3D_ZBUFFER) QT_GL(Vertex3f)(c.f(i), c.f(i + 1), static_cast<float>(c.d(i + 2)));
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

	// Context.c W3D_SetState in 0.53. The missing break after
	// W3D_ZBUFFERUPDATE is kept: it switches blending as well.
	void setState(uint32_t state, bool enable) {
		if (enable) {
			switch (state) {
			case W3D_TEXMAPPING: QT_GL(Enable)(GL_TEXTURE_2D); break;
			case W3D_GOURAUD: QT_GL(ShadeModel)(GL_SMOOTH); break;
			case W3D_ZBUFFER: QT_GL(Enable)(GL_DEPTH_TEST); break;
			case W3D_ZBUFFERUPDATE:
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
			case W3D_ZBUFFERUPDATE:
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
	}
	return false;
}
