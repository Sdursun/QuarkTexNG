// The parts of the UAE Native Interface (include/uni_common.h in WinUAE and
// FS-UAE) that the host library uses.
#ifndef QUARKTEX_UNI_H
#define QUARKTEX_UNI_H

#include <cstdint>

#define QT_EXPORT extern "C" __declspec(dllexport)

// Registers passed by uaenative.library call_function.
struct uni {
	int32_t d1, d2, d3, d4, d5, d6, d7;
	int32_t a1, a2, a3, a4, a5;
	int32_t a7;
};

// Set by the emulator after it has loaded the library.
typedef void* (__cdecl *uni_resolve_function)(uint32_t ptr);
extern "C" __declspec(dllexport) uni_resolve_function uni_resolve;

// Host pointer for an Amiga address.
template <typename T> inline T* amiga(int32_t address) {
	return static_cast<T*>(uni_resolve(static_cast<uint32_t>(address)));
}

#endif
