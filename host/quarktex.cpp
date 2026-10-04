// QuarkTex host library. The emulator loads it through uaenative.library
// (native_code=true) as quarktex-windows-x86.dll or quarktex-windows-x86-64.dll
// and the 68k side calls the qt_* functions below.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include "uni.h"
#include "gldecode.h"

// Must match QT_PROTOCOL_VERSION in gl/gl.c.
#define QT_PROTOCOL_VERSION 2

extern "C" {
	__declspec(dllexport) uni_resolve_function uni_resolve = 0;
}

namespace {
	HINSTANCE instance = 0;
	HWND amigaWindow = 0;
	HWND windowHandle = 0;
	HDC deviceContext = 0;
	HGLRC glContext = 0;
	bool registered = false;

	LRESULT CALLBACK windowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_NCCREATE:
			return 1;
		case WM_NCDESTROY:
			return 1;
		case WM_CREATE:
			return 0;
		case WM_DESTROY:
			return 0;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

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

	// The window the emulator draws the Amiga display into. uaenative.library
	// does not pass it, so look it up in our own process.
	HWND findAmigaWindow() {
		WindowSearch search = {GetCurrentProcessId(), 0};
		EnumWindows(checkWindow, reinterpret_cast<LPARAM>(&search));
		return search.found;
	}

	std::ofstream* out;

	void logString(const char* c) {
		if (!out) out = new std::ofstream("QuarkTexLog.txt");
		*out << c << std::endl;
	}
}

// Frame capture for the reference tests in tests/. Off unless the environment
// variable QUARKTEX_CAPTURE_DIR names a directory: every swap then writes
// <label>_<frame>.bmp there. The label is the first line of label.txt in that
// directory when the context is created (the test programs write it).
namespace {
	std::string captureDir;
	std::string captureLabel;
	int captureContexts = 0;
	int captureFrames = 0;

	void startCapture() {
		const char* dir = getenv("QUARKTEX_CAPTURE_DIR");
		captureDir = dir ? dir : "";
		if (captureDir.empty()) return;
		++captureContexts;
		captureFrames = 0;

		std::string line;
		std::ifstream labelFile((captureDir + "\\label.txt").c_str());
		if (labelFile) std::getline(labelFile, line);
		captureLabel.clear();
		for (size_t i = 0; i < line.size(); ++i) {
			char c = line[i];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-') captureLabel += c;
		}
		if (captureLabel.empty()) {
			char name[32];
			sprintf(name, "context%02d", captureContexts);
			captureLabel = name;
		}
	}

	void captureFrame() {
		RECT rect;
		if (!GetClientRect(windowHandle, &rect)) return;
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		if (width <= 0 || height <= 0) return;

		int rowSize = (width * 3 + 3) & ~3;
		std::vector<unsigned char> pixels(rowSize * height);

		// Leave the pixel state of the Amiga application untouched.
		glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
		glPushAttrib(GL_PIXEL_MODE_BIT);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_SKIP_ROWS, 0);
		glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
		glReadBuffer(GL_BACK);
		// Bottom-up BGR rows padded to 4 bytes are exactly what BMP stores.
		glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, &pixels[0]);
		glPopAttrib();
		glPopClientAttrib();

		BITMAPFILEHEADER file;
		BITMAPINFOHEADER info;
		memset(&file, 0, sizeof(file));
		memset(&info, 0, sizeof(info));
		file.bfType = 0x4D42;
		file.bfOffBits = sizeof(file) + sizeof(info);
		file.bfSize = file.bfOffBits + static_cast<DWORD>(pixels.size());
		info.biSize = sizeof(info);
		info.biWidth = width;
		info.biHeight = height;
		info.biPlanes = 1;
		info.biBitCount = 24;
		info.biCompression = BI_RGB;
		info.biSizeImage = static_cast<DWORD>(pixels.size());

		char name[32];
		sprintf(name, "_%03d.bmp", captureFrames++);
		std::ofstream bmp((captureDir + "\\" + captureLabel + name).c_str(), std::ios::binary);
		bmp.write(reinterpret_cast<const char*>(&file), sizeof(file));
		bmp.write(reinterpret_cast<const char*>(&info), sizeof(info));
		bmp.write(reinterpret_cast<const char*>(&pixels[0]), pixels.size());
	}
}

QT_EXPORT int __cdecl uni_init(void) {
	return 0;
}

QT_EXPORT int32_t __cdecl qt_protocol_version(struct uni*) {
	return QT_PROTOCOL_VERSION;
}

// a1 = C string
QT_EXPORT int32_t __cdecl qt_log(struct uni* uni) {
	logString(amiga<const char>(uni->a1));
	return 0;
}

void qt_report(const char* message) {
	logString(message);
}

// a1 = command buffer, d1 = its length in bytes. Returns the result of the
// last command.
QT_EXPORT int32_t __cdecl qt_execute(struct uni* uni) {
	if (!glContext) return 0;
	return qt_decode(amiga<const uint8_t>(uni->a1), static_cast<uint32_t>(uni->d1), uni_resolve);
}

QT_EXPORT int32_t __cdecl qt_free_context(struct uni*) {
	if (glContext) {
		wglMakeCurrent(0, 0);
		wglDeleteContext(glContext);
		glContext = 0;
	}
	if (deviceContext) {
		ReleaseDC(windowHandle, deviceContext);
		deviceContext = 0;
	}
	if (windowHandle) {
		DestroyWindow(windowHandle);
		windowHandle = 0;
	}

	UnregisterClassA("QuarkTex", instance);
	registered = false;
	return 0;
}

// d1 = left, d2 = top, d3 = width, d4 = height of the drawing area inside the
// Amiga display; width 0 means the whole display. Returns 1 on success.
QT_EXPORT int32_t __cdecl qt_create_context(struct uni* uni) {
	int left = uni->d1, top = uni->d2, width = uni->d3, height = uni->d4;

	if (!instance) instance = GetModuleHandleA(0);
	if (registered) UnregisterClassA("QuarkTex", instance);

	amigaWindow = findAmigaWindow();
	if (!amigaWindow) { logString("Warning: Could not find the emulator window"); return 0; }

	RECT rect;
	GetClientRect(amigaWindow, &rect);
	if (!width) {
		left = rect.left;
		top = rect.top;
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	else {
		left += rect.left;
		top += rect.top;
	}

	WNDCLASSA wc;
	memset(&wc, 0, sizeof(WNDCLASSA));
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = windowFunc;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.lpszClassName = "QuarkTex";

	if (!RegisterClassA(&wc)) { logString("Warning: Could not register Window Class"); return 0; }
	registered = true;

	if (!(windowHandle = CreateWindowExA(0, "QuarkTex", "", WS_CHILD | WS_VISIBLE, left, top, width, height, amigaWindow, 0, 0, 0))) logString("Warning: Could not create Window");
	if (!(deviceContext = GetDC(windowHandle))) { logString("Warning: Could not get Device Context"); return 0; }

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.dwLayerMask = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.cStencilBits = 8;
	int pixelformat;
	if ((pixelformat = ChoosePixelFormat(deviceContext, &pfd)) == 0) { logString("Warning: Could not choose pixel format"); return 0; }
	if (!SetPixelFormat(deviceContext, pixelformat, &pfd)) { logString("Warning: Could not set pixel format"); return 0; }

	if (!(glContext = wglCreateContext(deviceContext))) { logString("Warning: Could not create rendering context"); return 0; }
	if (!(wglMakeCurrent(deviceContext, glContext))) { logString("Warning: Could not activate the rendering context"); return 0; }

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(2.0f / static_cast<float>(width), -2.0f / static_cast<float>(height), 1.0f);
	glTranslatef(-(static_cast<float>(width) / 2.0f), -(static_cast<float>(height) / 2.0f), 0.0f);

	startCapture();
	return 1;
}

// d1 = left, d2 = top, d3 = width, d4 = height
QT_EXPORT int32_t __cdecl qt_move_window(struct uni* uni) {
	MoveWindow(windowHandle, uni->d1, uni->d2, uni->d3, uni->d4, FALSE);
	return 0;
}

QT_EXPORT int32_t __cdecl qt_swap_buffers(struct uni*) {
	if (!captureDir.empty()) captureFrame();
	SwapBuffers(deviceContext);

	GLenum code = glGetError();
	while (code != GL_NO_ERROR) {
		logString(reinterpret_cast<const char*>(gluErrorString(code)));
		code = glGetError();
	}
	return 0;
}
