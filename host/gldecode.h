// Decoding of the OpenGL command buffer written by the 68k side; the format
// is described in gl/glgen.cpp.
#ifndef QUARKTEX_GLDECODE_H
#define QUARKTEX_GLDECODE_H

#include <cstdint>
#include <cstring>
#ifdef _MSC_VER
#include <stdlib.h>
#endif

#if !defined(_WIN32) && !defined(__cdecl)
#define __cdecl
#endif

// Maps an Amiga address to a host pointer (uni_resolve in the library).
typedef void* (__cdecl *QtResolver)(uint32_t address);

// The buffer holds big-endian words; the host is little endian.
inline uint32_t qt_swap32(uint32_t value) {
#ifdef _MSC_VER
	return _byteswap_ulong(value);
#else
	return __builtin_bswap32(value);
#endif
}

// A pointer argument. Converts to whatever pointer type the OpenGL function
// takes, so glFuncs.txt does not have to match the host headers exactly.
struct QtPointer {
	void* value;
	template <typename T> operator T*() const { return static_cast<T*>(value); }
};

// One command: data points at its header word.
struct Command {
	const uint8_t* data;
	uint32_t words;
	QtResolver resolve;

	uint32_t u(int i) const {
		uint32_t value;
		memcpy(&value, data + 4 * i, 4);
		return qt_swap32(value);
	}
	float f(int i) const {
		uint32_t bits = u(i);
		float value;
		memcpy(&value, &bits, 4);
		return value;
	}
	// High word first.
	double d(int i) const {
		uint64_t bits = (static_cast<uint64_t>(u(i)) << 32) | u(i + 1);
		double value;
		memcpy(&value, &bits, 8);
		return value;
	}
	// Address 0 is NULL, not the start of Amiga memory.
	QtPointer p(int i) const {
		uint32_t address = u(i);
		QtPointer pointer = {address ? resolve(address) : 0};
		return pointer;
	}
};

// Executes all commands in the buffer. Returns the result of the last one
// (the value of a synchronous call that returns something), 0 otherwise.
int32_t qt_decode(const uint8_t* buffer, uint32_t bytes, QtResolver resolve);

// Reports a broken command; provided by the library (log file) or the test.
void qt_report(const char* message);

// Executes a Warp3D command (opcode QT_W3D_FIRST and up, host/w3d.cpp).
// Returns false for an unknown opcode or a wrong word count.
bool qt_w3d_decode(const Command& c, int32_t& result);

// Executes a command of QuarkTex's minigl.library (opcode QT_MGL_FIRST and
// up, host/mgl.cpp). Returns false for an unknown opcode or a wrong word count.
bool qt_mgl_decode(const Command& c, int32_t& result);

// Executes a command of QuarkTex's ogles2.library (opcode QT_OGLES2_FIRST
// and up, host/ogles2.cpp). Returns false for an unknown opcode or a wrong
// word count.
bool qt_ogles2_decode(const Command& c, int32_t& result);

// Forgets what host/ogles2.cpp keeps for the current OpenGL context (its
// converted vertex buffers and shader sources); called while it is still
// current, before it is deleted.
void qt_ogles2_forget_context();

// Multisampling (host/ogles2.cpp, "MSAA"): the current context's stand-in
// resolved into the real framebuffer before its pixels are read (GL 1.1's
// glReadPixels/glCopyTex(Sub)Image here), and before a frame is presented
// (also following a new size of framebuffer 0). Nothing without a stand-in.
void qt_ogles2_msaa_resolve();
void qt_ogles2_msaa_present();
// Before glClear (mask): a whole-picture clear brings back a stand-in a
// blit into the real framebuffer had left behind.
void qt_ogles2_msaa_clear(uint32_t mask);

// A new core OpenGL context (an HGLRC), sharing the objects of with's (0:
// none): host/ogles2.cpp keeps shader sources per group of contexts that
// share objects.
void qt_ogles2_share_context(uintptr_t gl, uintptr_t with);

// The host framebuffer an OpenGL ES program's framebuffer 0 stands for: the
// current offscreen context's framebuffer object, or 0 (host/quarktex.cpp).
unsigned int qt_default_framebuffer();

// Draws what the Warp3D commands have batched (ffp::flush), before an OpenGL
// command runs.
void qt_w3d_sync();

// Tracing of Warp3D commands into the log (host/w3d.cpp).
extern bool qt_w3d_trace_textures, qt_w3d_trace_all;

#endif
