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

// Draws what the Warp3D commands have batched (ffp::flush), before an OpenGL
// command runs.
void qt_w3d_sync();

#endif
