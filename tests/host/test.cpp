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

template <typename T> Record& operator<<(Record& r, T* pointer) {
	char text[32];
	if (!pointer) snprintf(text, sizeof(text), "NULL");
	else snprintf(text, sizeof(text), "@%lx", static_cast<unsigned long>(reinterpret_cast<const uint8_t*>(pointer) - arena));
	r.add(text);
	return r;
}

#define QT_GL(name) stub_gl##name
#include "glstubs.auto.inc"

// --- Host side ---------------------------------------------------------------

static int reports;

void qt_report(const char* message) {
	fprintf(stderr, "report: %s\n", message);
	++reports;
}

#include "../../host/gldecode.cpp"
#include "../../host/w3d.cpp"

// --- 68k side ----------------------------------------------------------------

static const ULONG capacity = 64;
static ULONG buffer[capacity];
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

	check(reports == 0, std::to_string(reports) + " bad commands reported");

	// An unknown Warp3D opcode is reported, not executed.
	*qt_reserve(1) = (static_cast<ULONG>(QT_W3D_FIRST + 0x7fff) << 16) | 1;
	qt_flush();
	check(reports == 1, "unknown Warp3D opcode: " + std::to_string(reports) + " reports");
	reports = 0;
	printf("%d functions checked, %d failures\n", count, failures);
	return failures ? 1 : 0;
}
