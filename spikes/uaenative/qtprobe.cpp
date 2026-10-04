// Probe for the UAE Native Interface (uaenative.library), the way QuarkTex
// will reach the host in 32- and 64-bit WinUAE. Built as
// qtprobe-windows-x86.dll and qtprobe-windows-x86-64.dll.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>

#define UNIAPI extern "C" __declspec(dllexport)

// Register block passed by uaenative.library (include/uni_common.h in UAE).
struct uni {
	int32_t d1, d2, d3, d4, d5, d6, d7;
	int32_t a1, a2, a3, a4, a5;
	int32_t a7;
};

// Filled in by the emulator after loading the DLL.
typedef void* (__cdecl *uni_resolve_function)(uint32_t ptr);
typedef const char* (__cdecl *uni_uae_version_function)(void);
extern "C" {
	__declspec(dllexport) uni_resolve_function uni_resolve = 0;
	__declspec(dllexport) uni_uae_version_function uni_uae_version = 0;
}

namespace {
	struct WindowSearch {
		DWORD process;
		HWND found;
	};

	BOOL CALLBACK checkWindow(HWND hwnd, LPARAM param) {
		WindowSearch* search = reinterpret_cast<WindowSearch*>(param);
		DWORD process = 0;
		GetWindowThreadProcessId(hwnd, &process);
		char name[64];
		if (process == search->process && GetClassNameA(hwnd, name, sizeof(name)) && strcmp(name, "AmigaPowah") == 0) {
			search->found = hwnd;
			return FALSE;
		}
		EnumChildWindows(hwnd, checkWindow, param);
		return search->found ? FALSE : TRUE;
	}

	// The window WinUAE draws the Amiga display into.
	HWND findAmigaWindow() {
		WindowSearch search = {GetCurrentProcessId(), 0};
		EnumWindows(checkWindow, reinterpret_cast<LPARAM>(&search));
		return search.found;
	}
}

UNIAPI int __cdecl uni_init(void) {
	return 0;
}

// a1 = message (C string), a2 = reply buffer, d2 = reply buffer size,
// d1 = number. Writes a report into the reply buffer, returns d1 + 1.
UNIAPI int32_t __cdecl qt_probe(struct uni* uni) {
	const char* message = static_cast<const char*>(uni_resolve(static_cast<uint32_t>(uni->a1)));
	char* reply = static_cast<char*>(uni_resolve(static_cast<uint32_t>(uni->a2)));
	HWND window = findAmigaWindow();
	RECT rect = {0, 0, 0, 0};
	if (window) GetClientRect(window, &rect);
	snprintf(reply, static_cast<size_t>(uni->d2),
		"%d-bit host, %s, message '%s', d1=%ld, AmigaPowah window %s (%ldx%ld)",
		static_cast<int>(sizeof(void*) * 8), uni_uae_version ? uni_uae_version() : "unknown emulator",
		message, static_cast<long>(uni->d1), window ? "found" : "NOT FOUND",
		static_cast<long>(rect.right - rect.left), static_cast<long>(rect.bottom - rect.top));
	return uni->d1 + 1;
}

// a1 = buffer, d1 = length. Returns the 32-bit sum of the bytes.
UNIAPI int32_t __cdecl qt_sum(struct uni* uni) {
	const uint8_t* data = static_cast<const uint8_t*>(uni_resolve(static_cast<uint32_t>(uni->a1)));
	uint32_t sum = 0;
	for (int32_t i = 0; i < uni->d1; ++i) sum += data[i];
	return static_cast<int32_t>(sum);
}
