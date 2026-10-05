// Unit test for the OpenGL command buffer: the generated 68k encoders
// (gl/glencode.auto.c) and the host decoder (host/gldecode.cpp) are built into
// one program. Each function is called through encoder, a big-endian buffer
// and decoder into a recording stub, and again directly on the stub; both
// recordings must be the same. Built and run by ./build.sh unittest.
#define QT_TEST
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

// Types and constants as on the 68k side (gl/gl.h).
typedef uint32_t ULONG;
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
#include "../../gl/gldefines.h"

// Stand-in for Amiga memory: address a is arena + a.
static uint8_t arena[0x20000];

static void* resolve(uint32_t address) {
	return arena + address;
}

// --- Recording OpenGL stubs ------------------------------------------------

static std::vector<std::string> records;

struct Record {
	std::string text;
	bool first;
	explicit Record(const char* name) : text(std::string(name) + "("), first(true) {}
	~Record() { records.push_back(text + ")"); }
	void add(const std::string& value) {
		if (!first) text += ",";
		first = false;
		text += value;
	}
};

template <typename T>
typename std::enable_if<std::is_integral<T>::value, Record&>::type operator<<(Record& r, T value) {
	r.add(std::to_string(static_cast<long long>(value)));
	return r;
}

inline Record& operator<<(Record& r, float value) {
	char text[32];
	snprintf(text, sizeof(text), "%.9g", value);
	r.add(text);
	return r;
}

inline Record& operator<<(Record& r, double value) {
	char text[32];
	snprintf(text, sizeof(text), "%.17g", value);
	r.add(text);
	return r;
}

static bool inArena(const void* pointer) {
	return pointer >= arena && pointer < arena + sizeof(arena);
}

// Pointers into Amiga memory as their address; pointers the host made up
// itself (its own variables) as @?.
template <typename T> Record& operator<<(Record& r, T* pointer) {
	char text[32];
	if (!pointer) snprintf(text, sizeof(text), "NULL");
	else if (!inArena(pointer)) snprintf(text, sizeof(text), "@?");
	else snprintf(text, sizeof(text), "@%lx", static_cast<unsigned long>(reinterpret_cast<const uint8_t*>(pointer) - arena));
	r.add(text);
	return r;
}

// A float array the host made up (colours for glFogfv, glTexEnvfv, ...):
// its four values.
inline Record& operator<<(Record& r, GLfloat* pointer) {
	if (!pointer || inArena(pointer)) return operator<< <GLfloat>(r, pointer);
	char text[96];
	snprintf(text, sizeof(text), "{%.9g,%.9g,%.9g,%.9g}", pointer[0], pointer[1], pointer[2], pointer[3]);
	r.add(text);
	return r;
}

#define QT_GL(name) stub_gl##name
#include "glstubs.auto.inc"

// ffp::DepthPoints (host/ffp.h), which only the Warp3D commands call: the
// count and the x, y, depth triples.
GLvoid stub_glDepthPoints(GLsizei count, const GLfloat* xyz) {
	Record r("DepthPoints");
	r << count;
	for (GLsizei i = 0; i < 3 * count; ++i) r << xyz[i];
}

// ffp::ChromaTest, ffp::ChromaBounds (Warp3D's chroma test).
GLvoid stub_glChromaTest(GLboolean enable) {
	Record r("ChromaTest");
	r << enable;
}

GLvoid stub_glChromaBounds(GLuint texture, GLint mode, GLuint lower, GLuint upper) {
	Record r("ChromaBounds");
	r << texture << mode << lower << upper;
}

// ffp::TexturePalette: a palette per name, not recorded.
static std::map<GLuint, std::vector<uint32_t> > stubPalettes;

std::vector<uint32_t>* stub_glTexturePalette(GLuint texture) {
	return &stubPalettes[texture];
}

// ffp::StencilPoints: the count, then x, y, value of each pixel.
GLvoid stub_glStencilPoints(GLsizei count, const GLfloat* xy, const GLuint* values) {
	Record r("StencilPoints");
	r << count;
	for (GLsizei i = 0; i < count; ++i) r << xy[2 * i] << xy[2 * i + 1] << values[i];
}

// The OpenGL 1.3 functions host/mgl.cpp looks up (GL_ARB_multitexture).
#define QT_GL13(name) stub_gl##name

GLvoid stub_glActiveTexture(GLenum unit) {
	Record r("ActiveTexture");
	r << unit;
}

GLvoid stub_glClientActiveTexture(GLenum unit) {
	Record r("ClientActiveTexture");
	r << unit;
}

GLvoid stub_glMultiTexCoord2f(GLenum unit, GLfloat s, GLfloat t) {
	Record r("MultiTexCoord2f");
	r << unit << s << t;
}

GLvoid stub_glBlendEquation(GLenum mode) {
	Record r("BlendEquation");
	r << mode;
}

GLvoid stub_glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
	Record r("BlendFuncSeparate");
	r << srcRGB << dstRGB << srcAlpha << dstAlpha;
}

// --- Host side ---------------------------------------------------------------

static int reports;

void qt_report(const char* message) {
	fprintf(stderr, "report: %s\n", message);
	++reports;
}

#include "../../host/gldecode.cpp"
#include "../../host/w3d.cpp"
#include "../../host/mgl.cpp"
#include "../../host/present.h"

// --- 68k side ----------------------------------------------------------------

static const ULONG capacity = 64;
static ULONG buffer[capacity];
static const uint8_t buffer8[4] = {0, 0, 0, 0}; // a header for hand-made Commands
static ULONG used;
static int flushes;

// Sends the buffer like the 68k side does: as big-endian words.
static ULONG qt_flush(void) {
	std::vector<uint8_t> bytes(used * 4);
	for (ULONG i = 0; i < used; ++i) {
		uint32_t big = qt_swap32(buffer[i]);
		memcpy(&bytes[i * 4], &big, 4);
	}
	used = 0;
	++flushes;
	return static_cast<ULONG>(qt_decode(bytes.data(), static_cast<uint32_t>(bytes.size()), resolve));
}

static ULONG* qt_reserve(ULONG words) {
	if (used + words > capacity) qt_flush();
	ULONG* w = buffer + used;
	used += words;
	return w;
}

static ULONG qt_f2l(float f) {
	ULONG l;
	memcpy(&l, &f, 4);
	return l;
}

static ULONG qt_dhi(double d) {
	uint64_t bits;
	memcpy(&bits, &d, 8);
	return static_cast<ULONG>(bits >> 32);
}

static ULONG qt_dlo(double d) {
	uint64_t bits;
	memcpy(&bits, &d, 8);
	return static_cast<ULONG>(bits);
}

#define QT_ADDRESS(p) static_cast<ULONG>(reinterpret_cast<uintptr_t>(p))

#include "../../gl/glencode.auto.c"

// --- Tests -------------------------------------------------------------------

struct Case {
	const char* name;
	bool synchronous;
	int32_t result; // expected return value, 0 for void
	std::function<int32_t()> encoded;
	std::function<int32_t()> direct;
};

static const Case cases[] = {
#include "glcalls.auto.inc"
};

static int failures;

static void check(bool ok, const std::string& what) {
	if (!ok) {
		printf("FAIL %s\n", what.c_str());
		++failures;
	}
}

static std::string joined() {
	std::string all;
	for (size_t i = 0; i < records.size(); ++i) all += (i ? " " : "") + records[i];
	return all;
}

int main() {
	int count = 0;
	for (const Case& c : cases) {
		records.clear();
		flushes = 0;
		int32_t result = c.encoded();
		if (c.synchronous) check(flushes == 1 && used == 0, std::string(c.name) + ": synchronous call did not flush");
		else {
			check(flushes == 0 && records.empty(), std::string(c.name) + ": queued call ran early");
			qt_flush();
		}
		std::string got = joined();
		records.clear();
		c.direct();
		std::string expected = joined();
		check(got == expected, std::string(c.name) + ": got " + got + ", expected " + expected);
		check(result == c.result, std::string(c.name) + ": returned " + std::to_string(result));
		++count;
	}

	// Calls keep their order across a full buffer.
	records.clear();
	flushes = 0;
	for (int i = 0; i < 40; ++i) _glVertex3f(static_cast<float>(i), 0.5f, -1.0f);
	qt_flush();
	check(records.size() == 40 && flushes == 3, "40 queued calls: " + std::to_string(records.size()) + " records, " + std::to_string(flushes) + " flushes");
	check(!records.empty() && records.front() == "Vertex3f(0,0.5,-1)" && records.back() == "Vertex3f(39,0.5,-1)", "order: " + joined());

	// GetPointerv returns the Amiga address passed to VertexPointer.
	records.clear();
	_glVertexPointer(3, GL_FLOAT, 12, reinterpret_cast<GLvoid*>(0x12340));
	_glGetPointerv(GL_VERTEX_ARRAY_POINTER, reinterpret_cast<GLvoid**>(0x10000));
	uint32_t pointer;
	memcpy(&pointer, arena + 0x10000, 4);
	check(pointer == 0x12340, "GetPointerv returned " + std::to_string(pointer));
	check(joined() == "VertexPointer(3,5126,12,@12340)", "VertexPointer: " + joined());

	// A NULL pointer arrives as NULL.
	records.clear();
	_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	check(joined() == "TexImage2D(3553,0,6408,4,4,0,6408,5121,NULL)", "NULL pointer: " + joined());

	// --- Warp3D commands: the OpenGL calls the 0.53 68k code made ---

	// W3D_CreateContext: texturing on, smooth shading.
	records.clear();
	*qt_reserve(1) = (static_cast<ULONG>(QT_W3D_INIT_CONTEXT) << 16) | 1;
	qt_flush();
	check(joined() == "Enable(3553) ShadeModel(7425)", "QT_W3D_INIT_CONTEXT: " + joined());

	// W3D_DrawTriangle, gouraud shading and z-buffer on, no texture: one DRAW
	// command, drawn as w3d.c drawVertex did.
	{
		const ULONG state = (1 << 10) | (1 << 11); // W3D_GOURAUD | W3D_ZBUFFER
		ULONG* w = qt_reserve(8 + 3 * QT_W3D_VERTEX_WORDS);
		w[0] = (static_cast<ULONG>(QT_W3D_DRAW) << 16) | (8 + 3 * QT_W3D_VERTEX_WORDS);
		w[1] = GL_TRIANGLES;
		w[2] = state;
		w[3] = 0; // no texture
		w[4] = w[5] = w[6] = 0;
		w[7] = 3;
		for (int v = 0; v < 3; ++v) {
			ULONG* vertex = w + 8 + v * QT_W3D_VERTEX_WORDS;
			memset(vertex, 0, QT_W3D_VERTEX_WORDS * 4);
			vertex[0] = qt_f2l(10.0f * (v + 1));  // x
			vertex[1] = qt_f2l(20.0f * (v + 1));  // y
			vertex[2] = qt_dhi(0.25 * (v + 1));   // z (double)
			vertex[3] = qt_dlo(0.25 * (v + 1));
			vertex[4] = qt_f2l(1.0f);             // w
			vertex[8] = qt_f2l(0.5f);             // colour r, g, b, a
			vertex[9] = qt_f2l(0.25f);
			vertex[10] = qt_f2l(0.125f);
			vertex[11] = qt_f2l(1.0f);
		}
		records.clear();
		qt_flush();
		check(joined() == "Begin(4) Color4f(0.5,0.25,0.125,1) Vertex3f(10,20,0.25) Color4f(0.5,0.25,0.125,1) Vertex3f(20,40,0.5) "
			"Color4f(0.5,0.25,0.125,1) Vertex3f(30,60,0.75) End()", "QT_W3D_DRAW: " + joined());
	}

	// Fog without the z-buffer still gets the depth; with neither, it does not.
	for (int fog = 0; fog < 2; ++fog) {
		ULONG* w = qt_reserve(8 + QT_W3D_VERTEX_WORDS);
		memset(w, 0, (8 + QT_W3D_VERTEX_WORDS) * 4);
		w[0] = (static_cast<ULONG>(QT_W3D_DRAW) << 16) | (8 + QT_W3D_VERTEX_WORDS);
		w[1] = GL_POINTS;
		w[2] = fog ? (1 << 14) : 0; // W3D_FOGGING
		w[7] = 1;
		w[8] = qt_f2l(5.0f);
		w[9] = qt_f2l(6.0f);
		w[10] = qt_dhi(0.5);
		w[11] = qt_dlo(0.5);
		records.clear();
		qt_flush();
		check(joined() == (fog ? "Begin(0) Vertex3f(5,6,0.5) End()" : "Begin(0) Vertex2f(5,6) End()"), "QT_W3D_DRAW fog: " + joined());
	}

	// W3D_SetState(W3D_ZBUFFERUPDATE, W3D_ENABLE) switches depth writes, nothing
	// else (0.53 switched blending through a missing break).
	records.clear();
	{
		ULONG* w = qt_reserve(3);
		w[0] = (static_cast<ULONG>(QT_W3D_SET_STATE) << 16) | 3;
		w[1] = 1 << 12;
		w[2] = 1; // W3D_ENABLE
	}
	qt_flush();
	check(joined() == "DepthMask(1)", "QT_W3D_SET_STATE ZBUFFERUPDATE: " + joined());

	// W3D_ClearDrawRegion clears in the colour, also in a window (0.53 drew a
	// rectangle in the current state there); channels / 255.
	records.clear();
	{
		ULONG* w = qt_reserve(5);
		w[0] = (static_cast<ULONG>(QT_W3D_CLEAR) << 16) | 5;
		w[1] = 0xFF204080; // ARGB
		w[2] = 0;          // window
		w[3] = 320;
		w[4] = 240;
	}
	qt_flush();
	check(joined() == "ClearColor(0.125490203,0.250980407,0.501960814,1) Clear(16384)", "QT_W3D_CLEAR: " + joined());

	// W3D_AllocTexObj with an R5G6B5 image at Amiga address 0x14000.
	records.clear();
	{
		ULONG* w = qt_reserve(6);
		w[0] = (static_cast<ULONG>(QT_W3D_TEX_ALLOC) << 16) | 6;
		w[1] = 3; // W3D_R5G6B5
		w[2] = 32;
		w[3] = 16;
		w[4] = 0x14000;
		w[5] = 0;       // no palette
	}
	qt_flush();
	check(joined() == "PixelStorei(3312,1) PixelStorei(3317,1) GenTextures(1,@?) BindTexture(3553,0) "
		"TexParameteri(3553,10242,10497) TexParameteri(3553,10243,10497) TexParameteri(3553,10240,9729) "
		"TexParameteri(3553,10241,9729) TexImage2D(3553,0,6408,32,16,0,6407,33635,@14000)",
		"QT_W3D_TEX_ALLOC: " + joined());

	// CHUNKY: a 2 x 2 rectangle of an image 4 bytes wide goes through the
	// palette (ARGB) to RGBA bytes.
	{
		auto put32 = [](uint32_t address, uint32_t value) {
			uint32_t big = qt_swap32(value);
			memcpy(arena + address, &big, 4);
		};
		for (uint32_t i = 0; i < 256; ++i) put32(0x17000 + 4 * i, 0x80000000u | (i << 16) | ((255 - i) << 8) | (i / 2));
		const uint8_t image[] = {1, 2, 9, 9, 3, 4, 9, 9};
		memcpy(arena + 0x17400, image, sizeof(image));
		Command c = {buffer8, 0, resolve};
		readPalette(c, 99, 0x17000);
		std::vector<GLubyte> rgba = chunkyToRgba(c, paletteOf(99), 0x17400, 2, 2, 4);
		const GLubyte expected[] = {1, 254, 0, 128, 2, 253, 1, 128, 3, 252, 1, 128, 4, 251, 2, 128};
		check(rgba.size() == 16 && memcmp(&rgba[0], expected, 16) == 0, "chunkyToRgba");
		stubPalettes.erase(99);
	}

	// W3D_UpdateTexSubImage of an R8G8B8 rectangle in an image 96 bytes wide:
	// the row length is set for the upload and reset after it.
	records.clear();
	{
		ULONG* w = qt_reserve(10);
		w[0] = (static_cast<ULONG>(QT_W3D_TEX_UPDATE) << 16) | 10;
		w[1] = 7;       // texture name
		w[2] = 4;       // W3D_R8G8B8
		w[3] = 18;
		w[4] = 20;
		w[5] = 12;
		w[6] = 8;
		w[7] = 0x16000; // image
		w[8] = 96;      // bytes per row
		w[9] = 0;       // no palette
	}
	qt_flush();
	check(joined() == "BindTexture(3553,7) PixelStorei(3312,0) PixelStorei(3314,32) TexSubImage2D(3553,0,18,20,12,8,6407,5121,@16000) "
		"PixelStorei(3314,0)", "QT_W3D_TEX_UPDATE: " + joined());

	// W3D_SetTexEnv(W3D_BLEND): the colour goes to OpenGL as r, g, b, a (0.53: r, b, g, a).
	records.clear();
	{
		ULONG* w = qt_reserve(7);
		w[0] = (static_cast<ULONG>(QT_W3D_TEX_ENV) << 16) | 7;
		w[1] = 5;  // texture name
		w[2] = 4;  // W3D_BLEND
		w[3] = qt_f2l(0.25f);
		w[4] = qt_f2l(0.5f);
		w[5] = qt_f2l(0.75f);
		w[6] = qt_f2l(1.0f);
	}
	qt_flush();
	check(joined() == "BindTexture(3553,5) TexEnvi(8960,8704,3042) TexEnvfv(8960,8705,{0.25,0.5,0.75,1})", "QT_W3D_TEX_ENV: " + joined());

	// W3D_DrawElements: big-endian arrays in Amiga memory (the arena), three
	// F_F_F vertices, RGBA float colours, UWORD indices 2, 0, 1.
	{
		auto put32 = [](uint32_t address, uint32_t value) {
			uint32_t big = qt_swap32(value);
			memcpy(arena + address, &big, 4);
		};
		for (int v = 0; v < 3; ++v) {
			for (int k = 0; k < 3; ++k) put32(0x15000 + 12 * v + 4 * k, qt_f2l(static_cast<float>(10 * v + k)));
			for (int k = 0; k < 4; ++k) put32(0x15100 + 16 * v + 4 * k, qt_f2l(0.25f * (k + 1)));
		}
		const uint8_t indices[] = {0, 2, 0, 0, 0, 1};
		memcpy(arena + 0x15200, indices, sizeof(indices));

		ULONG* w = qt_reserve(22);
		memset(w, 0, 22 * 4);
		w[0] = (static_cast<ULONG>(QT_W3D_DRAW_ARRAY) << 16) | 22;
		w[1] = 0;               // W3D_PRIMITIVE_TRIANGLES
		w[2] = 1 << 10;         // W3D_GOURAUD
		w[7] = 0x15000;         // vertices, stride 12, W3D_VERTEX_F_F_F
		w[8] = 12;
		w[9] = 0;
		w[10] = 0x15100;        // colours, stride 16, W3D_COLOR_FLOAT | W3D_CMODE_RGBA
		w[11] = 16;
		w[12] = (1u << 30) | 0x04;
		w[18] = 1;              // W3D_INDEX_UWORD
		w[19] = 0x15200;
		w[21] = 3;
		records.clear();
		qt_flush();
		check(joined() == "Begin(4) Color4f(0.25,0.5,0.75,1) Vertex3f(20,21,22) Color4f(0.25,0.5,0.75,1) Vertex3f(0,1,2) "
			"Color4f(0.25,0.5,0.75,1) Vertex3f(10,11,12) End()", "QT_W3D_DRAW_ARRAY: " + joined());
	}

	// W3D_WriteZSpan at (10, 20): z = -1, 0, 1 as big-endian doubles, the
	// middle one masked out. Depths (z + 1) / 2 at the pixel centres.
	{
		const double z[3] = {-1.0, 0.0, 1.0};
		for (int i = 0; i < 3; ++i) {
			uint64_t bits;
			memcpy(&bits, &z[i], 8);
			for (int k = 0; k < 8; ++k) arena[0x15300 + 8 * i + k] = static_cast<uint8_t>(bits >> (56 - 8 * k));
		}
		const uint8_t mask[3] = {1, 0, 1};
		memcpy(arena + 0x15400, mask, sizeof(mask));
		ULONG* w = qt_reserve(6);
		w[0] = (static_cast<ULONG>(QT_W3D_WRITE_Z) << 16) | 6;
		w[1] = 10;
		w[2] = 20;
		w[3] = 3;
		w[4] = 0x15300;
		w[5] = 0x15400;
		records.clear();
		qt_flush();
		check(joined() == "DepthPoints(2,10.5,20.5,0,12.5,20.5,1)", "QT_W3D_WRITE_Z: " + joined());
	}

	// W3D_FillStencilBuffer, 2 x 2 16-bit values at (5, 6); 0x0102 keeps its
	// low 8 bits. W3D_SetStencilFunc(W3D_ST_EQUAL, 3, 0xFF).
	{
		const uint8_t values[8] = {0, 1, 0, 2, 0x01, 0x02, 0, 0xFF};
		memcpy(arena + 0x15600, values, sizeof(values));
		ULONG* w = qt_reserve(8);
		w[0] = (static_cast<ULONG>(QT_W3D_WRITE_STENCIL) << 16) | 8;
		w[1] = 5;
		w[2] = 6;
		w[3] = 2;
		w[4] = 2;
		w[5] = 2;
		w[6] = 0x15600;
		w[7] = 0;
		w = qt_reserve(4);
		w[0] = (static_cast<ULONG>(QT_W3D_STENCIL_FUNC) << 16) | 4;
		w[1] = 5; // W3D_ST_EQUAL
		w[2] = 3;
		w[3] = 0xFF;
		records.clear();
		qt_flush();
		check(joined() == "StencilPoints(4,5.5,6.5,1,6.5,6.5,2,5.5,7.5,2,6.5,7.5,255) StencilFunc(514,3,255)",
			"QT_W3D_WRITE_STENCIL: " + joined());
	}

	// W3D_SetChromaTestBounds(W3D_CHROMATEST_EXCLUSIVE) on texture 7, then
	// W3D_SetState(W3D_CHROMATEST, W3D_ENABLE).
	{
		ULONG* w = qt_reserve(5);
		w[0] = (static_cast<ULONG>(QT_W3D_CHROMA) << 16) | 5;
		w[1] = 7;
		w[2] = 0x00102030;
		w[3] = 0x00405060;
		w[4] = 3;
		w = qt_reserve(3);
		w[0] = (static_cast<ULONG>(QT_W3D_SET_STATE) << 16) | 3;
		w[1] = 1 << 26;
		w[2] = 1;
		records.clear();
		qt_flush();
		check(joined() == "ChromaBounds(7,2,1056816,4214880) ChromaTest(1)", "QT_W3D_CHROMA: " + joined());
	}

	// W3D_ReadZSpan: the stub leaves the depths 0, which come back as z = -1.
	{
		ULONG* w = qt_reserve(5);
		w[0] = (static_cast<ULONG>(QT_W3D_READ_Z) << 16) | 5;
		w[1] = 3;
		w[2] = 4;
		w[3] = 2;
		w[4] = 0x15500;
		records.clear();
		qt_flush();
		double z[2];
		for (int i = 0; i < 2; ++i) {
			uint64_t bits = 0;
			for (int k = 0; k < 8; ++k) bits = (bits << 8) | arena[0x15500 + 8 * i + k];
			memcpy(&z[i], &bits, 8);
		}
		check(joined() == "ReadPixels(3,4,2,1,6402,5126,@?)" && z[0] == -1.0 && z[1] == -1.0,
			"QT_W3D_READ_Z: " + joined() + " " + std::to_string(z[0]) + " " + std::to_string(z[1]));
	}

	// minigl.library's glDrawElements: big-endian float vertices (size 3,
	// stride 16), unsigned byte colours and texture unit 1 coordinates,
	// unsigned short indices; the host converts the elements the indices
	// reach and draws them.
	{
		const float xyz[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
		for (int i = 0; i < 3; ++i)
			for (int k = 0; k < 3; ++k) {
				uint32_t big = qt_swap32(qt_f2l(xyz[i][k]));
				memcpy(&arena[0x16000 + 16 * i + 4 * k], &big, 4);
			}
		for (int i = 0; i < 12; ++i) arena[0x16100 + i] = static_cast<uint8_t>(10 + i);
		const uint8_t indices[] = {0, 2, 0, 1, 0, 0};
		memcpy(&arena[0x16200], indices, sizeof(indices));
		ULONG* w = qt_reserve(QT_MGL_DRAW_WORDS);
		const ULONG words[QT_MGL_DRAW_WORDS] = {(static_cast<ULONG>(QT_MGL_DRAW) << 16) | QT_MGL_DRAW_WORDS,
			GL_TRIANGLES, 0, 3, GL_UNSIGNED_SHORT, 0x16200,
			1, 3, GL_FLOAT, 16, 0x16000,
			1, 4, GL_UNSIGNED_BYTE, 4, 0x16100,
			0, 2, GL_FLOAT, 8, 0,
			1, 2, GL_FLOAT, 16, 0x16000};
		memcpy(w, words, sizeof(words));
		records.clear();
		qt_flush();
		check(joined() == "PushClientAttrib(2) EnableClientState(32884) VertexPointer(3,5126,0,@?) EnableClientState(32886) "
			"ColorPointer(4,5121,0,@?) ClientActiveTexture(33984) DisableClientState(32888) ClientActiveTexture(33985) "
			"EnableClientState(32888) TexCoordPointer(2,5126,0,@?) DisableClientState(32885) DisableClientState(32887) "
			"DisableClientState(32889) DrawElements(4,3,5125,@?) PopClientAttrib()", "QT_MGL_DRAW: " + joined());

		Command c = {buffer8, 0, resolve};
		using namespace mgl;
		Array vertices = {true, 3, GL_FLOAT, 16, 0x16000}, colours = {true, 4, GL_UNSIGNED_BYTE, 4, 0x16100};
		Converted v, col;
		bool ok = convert(c, vertices, 3, v) && convert(c, colours, 3, col);
		check(ok && v.floats.size() == 9 && v.floats[0] == 1 && v.floats[4] == 5 && v.floats[8] == 9
			&& col.bytes.size() == 12 && col.bytes[0] == 10 && col.bytes[11] == 21, "QT_MGL_DRAW conversion");
	}

	// glActiveTextureARB and glMultiTexCoord2fARB; a unit beyond
	// QT_MGL_TEXTURE_UNITS is ignored.
	{
		const ULONG words[] = {(static_cast<ULONG>(QT_MGL_ACTIVE_TEXTURE) << 16) | QT_MGL_ACTIVE_TEXTURE_WORDS, 0x84C1,
			(static_cast<ULONG>(QT_MGL_MULTI_TEX_COORD) << 16) | QT_MGL_MULTI_TEX_COORD_WORDS, 0x84C1, qt_f2l(0.5f), qt_f2l(0.25f),
			(static_cast<ULONG>(QT_MGL_ACTIVE_TEXTURE) << 16) | QT_MGL_ACTIVE_TEXTURE_WORDS, 0x84C2};
		memcpy(qt_reserve(sizeof(words) / 4), words, sizeof(words));
		records.clear();
		qt_flush();
		check(joined() == "ActiveTexture(33985) MultiTexCoord2f(33985,0.5,0.25)", "QT_MGL_ACTIVE_TEXTURE: " + joined());
	}

	// glBlendEquation and glBlendFuncSeparate.
	{
		const ULONG words[] = {(static_cast<ULONG>(QT_MGL_BLEND_EQUATION) << 16) | QT_MGL_BLEND_EQUATION_WORDS, 0x800A,
			(static_cast<ULONG>(QT_MGL_BLEND_FUNC_SEPARATE) << 16) | QT_MGL_BLEND_FUNC_SEPARATE_WORDS, 0x302, 0x303, 1, 0};
		memcpy(qt_reserve(sizeof(words) / 4), words, sizeof(words));
		records.clear();
		qt_flush();
		check(joined() == "BlendEquation(32778) BlendFuncSeparate(770,771,1,0)", "QT_MGL_BLEND_*: " + joined());
	}

	// glDrawBuffer and glReadBuffer ask for the framebuffer object bound
	// (none here, so GL_BACK stays GL_BACK).
	{
		records.clear();
		_glDrawBuffer(GL_BACK);
		_glReadBuffer(GL_FRONT);
		qt_flush();
		check(joined() == "GetIntegerv(36006,@?) DrawBuffer(1029) GetIntegerv(36010,@?) ReadBuffer(1028)",
			"DrawBuffer/ReadBuffer: " + joined());
	}

	// Presenting into Amiga display memory: one pixel (r 0xF8, g 0xFC, b 0x08)
	// in each format, then a 2 x 2 picture into a 3 x 2 bitmap at (2, 0),
	// clipped at the bitmap's right edge, rows turned upright.
	{
		const uint8_t bgra[4] = {0x08, 0xFC, 0xF8, 0x7F};
		struct { uint32_t format; uint8_t bytes[4]; } formats[] = {
			{present::R8G8B8, {0xF8, 0xFC, 0x08}}, {present::B8G8R8, {0x08, 0xFC, 0xF8}},
			{present::A8R8G8B8, {0, 0xF8, 0xFC, 0x08}}, {present::A8B8G8R8, {0, 0x08, 0xFC, 0xF8}},
			{present::R8G8B8A8, {0xF8, 0xFC, 0x08, 0}}, {present::B8G8R8A8, {0x08, 0xFC, 0xF8, 0}},
			{present::R5G6B5, {0xFF, 0xE1}}, {present::R5G6B5PC, {0xE1, 0xFF}},
			{present::R5G5B5, {0x7F, 0xE1}}, {present::R5G5B5PC, {0xE1, 0x7F}},
			{present::B5G6R5PC, {0xFF, 0x0F}}, {present::B5G5R5PC, {0xFF, 0x07}}};
		for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
			uint8_t out[4] = {0xAA, 0xAA, 0xAA, 0xAA};
			present::convertRow(bgra, out, 1, formats[i].format);
			int n = present::bytesPerPixel(formats[i].format);
			check(n > 0 && memcmp(out, formats[i].bytes, n) == 0, "present format " + std::to_string(formats[i].format));
		}
		check(present::bytesPerPixel(1) == 0, "present: CLUT is not written");

		// Bottom-up picture: bottom row red, green; top row blue, white.
		const uint8_t picture[16] = {0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 255, 255, 0};
		uint8_t* bitmap = arena + 0x17000;
		memset(bitmap, 0x55, 3 * 2 * 3);
		present::Target t = {0x17000, 9, present::R8G8B8, 3, 2, 2, 0, 2, 2};
		bool ok = present::write(picture, 2, 2, t, resolve);
		// Row 0 (top): blue at x 2, x 3 clipped; row 1: red at x 2.
		const uint8_t expected[18] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0, 0, 255,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 255, 0, 0};
		check(ok && memcmp(bitmap, expected, 18) == 0, "present::write");
	}

	check(reports == 0, std::to_string(reports) + " bad commands reported");

	// An unknown Warp3D opcode is reported, not executed.
	*qt_reserve(1) = (static_cast<ULONG>(QT_W3D_FIRST + 0x7fff) << 16) | 1;
	qt_flush();
	check(reports == 1, "unknown Warp3D opcode: " + std::to_string(reports) + " reports");
	reports = 0;
	printf("%d functions checked, %d failures\n", count, failures);
	return failures ? 1 : 0;
}
