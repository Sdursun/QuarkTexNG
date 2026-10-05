// QuarkTex host library. The emulator loads it through uaenative.library
// (native_code=true) as quartexng-windows-x86.dll or quartexng-windows-x86-64.dll
// and the 68k side calls the qt_* functions below.
//
// Every Warp3D or agl context the Amiga side creates gets its own child
// window of the emulator window, OpenGL context and (for Warp3D) ffp.h state,
// named by the id qt_create_context returns; the other calls pass that id.
#include "gl3.h"
#include <GL/glu.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "uni.h"
#include "gldecode.h"
#include "ffp.h"
#include "present.h"

// Must match QT_PROTOCOL_VERSION in gl/gl.c.
#define QT_PROTOCOL_VERSION 9

extern "C" {
	__declspec(dllexport) uni_resolve_function uni_resolve = 0;
}

namespace {
	HINSTANCE instance = 0;
	int classUsers = 0; // windows of the "QuartexNG" class

	// qt_create_context flags, as QT_CONTEXT_CORE in gl/gl.h.
	const int32_t contextCore = 1;
	// QT_CONTEXT_PLAIN: a compatibility context without the 0.53 model view
	// matrix, with a 24-bit depth buffer (minigl.library).
	const int32_t contextPlain = 2;
	// QT_CONTEXT_OFFSCREEN: draws into a framebuffer object, presented into the
	// Amiga bitmap that qt_swap_buffers names (present.h); its window stays
	// hidden.
	const int32_t contextOffscreen = 4;

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
		if (!out) out = new std::ofstream("QuartexNGLog.txt");
		*out << c << std::endl;
	}

	// Time spent executing command buffers, logged per context when the
	// environment variable QUARKTEX_PROFILE is set.
	struct Profile {
		bool on;
		LONGLONG ticks;
		unsigned long calls;
		unsigned long long bytes;
		// Every reportFrames buffer swaps: the wall clock time, the time
		// executing command buffers and in SwapBuffers; the rest is the
		// emulated 68k's.
		LONGLONG periodStart, periodTicks, swapTicks;
		unsigned long periodCalls;
		unsigned long long periodBytes;
	};
	const unsigned long reportFrames = 300;

	struct Context {
		uint32_t id;
		HWND window;
		HDC deviceContext;
		HGLRC gl;
		ffp::Context* ffp; // Warp3D (OpenGL 3.3 core) only
		bool offscreen;
		present::Framebuffer framebuffer; // offscreen only
		std::string label; // frame capture
		int frames;
		unsigned long swaps;
		Profile profile;
	};

	std::map<uint32_t, Context*> contexts;
	uint32_t nextId = 1;
	Context* active = 0; // whose OpenGL context is current

	Context* find(int32_t id) {
		std::map<uint32_t, Context*>::const_iterator i = contexts.find(static_cast<uint32_t>(id));
		return i == contexts.end() ? 0 : i->second;
	}

	// Makes the context's OpenGL context (and ffp state) current.
	bool activate(Context* c) {
		if (c == active) return true;
		if (!wglMakeCurrent(c->deviceContext, c->gl)) {
			logString("Warning: Could not activate a rendering context");
			return false;
		}
		active = c;
		ffp::makeCurrent(c->ffp);
		return true;
	}

	void logProfile(const Context* c) {
		if (!c->profile.on || !c->profile.calls) return;
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		char line[160];
		snprintf(line, sizeof(line), "Profile %s: %lu buffers, %llu bytes, %.3f s executing",
			c->label.empty() ? "context" : c->label.c_str(), c->profile.calls, c->profile.bytes,
			static_cast<double>(c->profile.ticks) / frequency.QuadPart);
		logString(line);
	}
}

// Frame capture for the reference tests in tests/. Off unless the environment
// variable QUARKTEX_CAPTURE_DIR names a directory: every swap then writes
// <label>_<frame>.bmp there. The label is the first line of label.txt in that
// directory when the context is created (the test programs write it); a
// second context created while one with that label exists gets <label>-2,
// and so on.
namespace {
	std::string captureDir;
	unsigned long captureEvery = 1; // QUARKTEX_CAPTURE_EVERY: only every nth swap
	int captureContexts = 0;

	std::string captureLabel() {
		std::string line, label;
		std::ifstream labelFile((captureDir + "\\label.txt").c_str());
		if (labelFile) std::getline(labelFile, line);
		for (size_t i = 0; i < line.size(); ++i) {
			char c = line[i];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-') label += c;
		}
		char name[32];
		if (label.empty()) {
			sprintf(name, "context%02d", captureContexts);
			return name;
		}
		int same = 0;
		for (std::map<uint32_t, Context*>::const_iterator i = contexts.begin(); i != contexts.end(); ++i) {
			const std::string& other = i->second->label;
			if (other == label || (other.compare(0, label.size() + 1, label + "-") == 0)) ++same;
		}
		if (!same) return label;
		sprintf(name, "-%d", same + 1);
		return label + name;
	}

	void captureFrame(Context* c) {
		RECT rect;
		if (!GetClientRect(c->window, &rect)) return;
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		if (width <= 0 || height <= 0) return;

		int rowSize = (width * 3 + 3) & ~3;
		std::vector<unsigned char> pixels(rowSize * height);

		// Leave the pixel state of the Amiga application untouched. The core
		// profile has no attribute stacks; there only the pack parameters
		// and the read buffer can have changed.
		const GLenum packs[] = {GL_PACK_ALIGNMENT, GL_PACK_ROW_LENGTH, GL_PACK_SKIP_ROWS, GL_PACK_SKIP_PIXELS, GL_PACK_SWAP_BYTES};
		const GLint packValues[] = {4, 0, 0, 0, GL_FALSE};
		GLint saved[5], readBuffer = GL_BACK;
		bool core = c->ffp != 0;
		if (core) {
			for (int i = 0; i < 5; ++i) glGetIntegerv(packs[i], &saved[i]);
			glGetIntegerv(GL_READ_BUFFER, &readBuffer);
		}
		else {
			glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
			glPushAttrib(GL_PIXEL_MODE_BIT);
		}
		for (int i = 0; i < 5; ++i) glPixelStorei(packs[i], packValues[i]);
		glReadBuffer(c->offscreen ? 0x8CE0 /* GL_COLOR_ATTACHMENT0 */ : GL_BACK);
		// Bottom-up BGR rows padded to 4 bytes are exactly what BMP stores.
		glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, &pixels[0]);
		if (core) {
			for (int i = 0; i < 5; ++i) glPixelStorei(packs[i], saved[i]);
			glReadBuffer(static_cast<GLenum>(readBuffer));
		}
		else {
			glPopAttrib();
			glPopClientAttrib();
		}

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
		sprintf(name, "_%03d.bmp", c->frames++);
		std::ofstream bmp((captureDir + "\\" + c->label + name).c_str(), std::ios::binary);
		bmp.write(reinterpret_cast<const char*>(&file), sizeof(file));
		bmp.write(reinterpret_cast<const char*>(&info), sizeof(info));
		bmp.write(reinterpret_cast<const char*>(&pixels[0]), pixels.size());
	}

	// Releases what create got so far; c is not in contexts yet or any more.
	void destroy(Context* c) {
		if (c->gl) {
			if (wglMakeCurrent(c->deviceContext, c->gl)) {
				if (c->ffp) ffp::destroy(c->ffp);
				present::destroy(c->framebuffer);
				ffp::makeCurrent(0);
			}
			wglMakeCurrent(0, 0);
			wglDeleteContext(c->gl);
			active = 0;
		}
		if (c->deviceContext) ReleaseDC(c->window, c->deviceContext);
		if (c->window) {
			DestroyWindow(c->window);
			if (--classUsers == 0) UnregisterClassA("QuartexNG", instance);
		}
		delete c;
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

// a1 = command buffer, d1 = its length in bytes, d2 = context. Returns the
// result of the last command.
// QUARKTEX_TRACE=1 logs every Warp3D texture command; QUARKTEX_TRACE_FRAME=n
// every Warp3D command between the n-th and the next buffer swap of a context.
namespace {
	long traceFrame = -1;
}

QT_EXPORT int32_t __cdecl qt_execute(struct uni* uni) {
	Context* c = find(uni->d2);
	if (!c || !activate(c)) return 0;
	// An offscreen context's last picture is written as soon as the GPU has it.
	if (c->offscreen) present::poll(c->framebuffer, uni_resolve, false);
	qt_w3d_trace_all = traceFrame >= 0 && c->swaps == static_cast<unsigned long>(traceFrame);
	if (!c->profile.on) return qt_decode(amiga<const uint8_t>(uni->a1), static_cast<uint32_t>(uni->d1), uni_resolve);
	LARGE_INTEGER start, end;
	QueryPerformanceCounter(&start);
	int32_t result = qt_decode(amiga<const uint8_t>(uni->a1), static_cast<uint32_t>(uni->d1), uni_resolve);
	QueryPerformanceCounter(&end);
	c->profile.ticks += end.QuadPart - start.QuadPart;
	c->profile.periodTicks += end.QuadPart - start.QuadPart;
	++c->profile.periodCalls;
	c->profile.periodBytes += static_cast<uint32_t>(uni->d1);
	++c->profile.calls;
	c->profile.bytes += static_cast<uint32_t>(uni->d1);
	return result;
}

// d1 = context
QT_EXPORT int32_t __cdecl qt_free_context(struct uni* uni) {
	Context* c = find(uni->d1);
	if (!c) return 0;
	logProfile(c);
	contexts.erase(c->id);
	destroy(c);
	return 0;
}

// d1 = left, d2 = top, d3 = width, d4 = height of the drawing area inside the
// Amiga display; width 0 means the whole display. d5 = flags (contextCore).
// Returns the new context's id, 0 on failure.
QT_EXPORT int32_t __cdecl qt_create_context(struct uni* uni) {
	int left = uni->d1, top = uni->d2, width = uni->d3, height = uni->d4;
	bool core = (uni->d5 & contextCore) != 0;

	if (!instance) instance = GetModuleHandleA(0);
	HWND amigaWindow = findAmigaWindow();
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

	if (!classUsers) {
		WNDCLASSA wc;
		memset(&wc, 0, sizeof(WNDCLASSA));
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = windowFunc;
		wc.hInstance = instance;
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.lpszClassName = "QuartexNG";
		if (!RegisterClassA(&wc)) { logString("Warning: Could not register Window Class"); return 0; }
	}

	Context* c = new Context();
	c->offscreen = (uni->d5 & contextOffscreen) != 0;
	DWORD style = c->offscreen ? WS_CHILD : WS_CHILD | WS_VISIBLE;
	if (!(c->window = CreateWindowExA(0, "QuartexNG", "", style, left, top, width, height, amigaWindow, 0, 0, 0))) {
		logString("Warning: Could not create Window");
		if (!classUsers) UnregisterClassA("QuartexNG", instance);
		delete c;
		return 0;
	}
	++classUsers;
	if (!(c->deviceContext = GetDC(c->window))) { logString("Warning: Could not get Device Context"); destroy(c); return 0; }

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.dwLayerMask = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = (uni->d5 & contextPlain) ? 24 : 16;
	pfd.cStencilBits = 8;
	int pixelformat;
	if ((pixelformat = ChoosePixelFormat(c->deviceContext, &pfd)) == 0) { logString("Warning: Could not choose pixel format"); destroy(c); return 0; }
	if (!SetPixelFormat(c->deviceContext, pixelformat, &pfd)) { logString("Warning: Could not set pixel format"); destroy(c); return 0; }

	if (!(c->gl = core ? gl3::createCoreContext(c->deviceContext) : wglCreateContext(c->deviceContext))) {
		logString(core ? "Warning: Could not create an OpenGL 3.3 core rendering context" : "Warning: Could not create rendering context");
		destroy(c);
		return 0;
	}
	if (!wglMakeCurrent(c->deviceContext, c->gl)) { logString("Warning: Could not activate the rendering context"); destroy(c); return 0; }
	active = c;
	ffp::makeCurrent(0);
	c->framebuffer.core = core;
	if (c->offscreen && !present::resize(c->framebuffer, width, height, uni_resolve)) {
		logString("Warning: Could not create a framebuffer object");
		destroy(c);
		return 0;
	}

	if (core) {
		if (!gl3::load()) { logString("Warning: OpenGL 3.3 functions missing"); destroy(c); return 0; }
		if (!(c->ffp = ffp::create(width, height))) { destroy(c); return 0; }
		static bool logged = false;
		if (!logged) {
			logged = true;
			logString((std::string("OpenGL ") + reinterpret_cast<const char*>(glGetString(GL_VERSION)) + ", " +
				reinterpret_cast<const char*>(glGetString(GL_RENDERER))).c_str());
		}
	}
	else if (!(uni->d5 & contextPlain)) {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glScalef(2.0f / static_cast<float>(width), -2.0f / static_cast<float>(height), 1.0f);
		glTranslatef(-(static_cast<float>(width) / 2.0f), -(static_cast<float>(height) / 2.0f), 0.0f);
	}

	const char* dir = getenv("QUARKTEX_CAPTURE_DIR");
	captureDir = dir ? dir : "";
	if (!captureDir.empty()) {
		++captureContexts;
		c->label = captureLabel();
		const char* every = getenv("QUARKTEX_CAPTURE_EVERY");
		captureEvery = every && atol(every) > 0 ? static_cast<unsigned long>(atol(every)) : 1;
	}
	c->profile.on = getenv("QUARKTEX_PROFILE") != 0;
	const char* trace = getenv("QUARKTEX_TRACE");
	qt_w3d_trace_textures = trace && *trace && *trace != '0';
	const char* frame = getenv("QUARKTEX_TRACE_FRAME");
	traceFrame = frame && *frame ? atol(frame) : -1;
	c->id = nextId++;
	if (!nextId) nextId = 1; // 0 means no context
	contexts[c->id] = c;
	return static_cast<int32_t>(c->id);
}

// d1 = left, d2 = top, d3 = width, d4 = height, d5 = context
QT_EXPORT int32_t __cdecl qt_move_window(struct uni* uni) {
	Context* c = find(uni->d5);
	if (!c) return 0;
	MoveWindow(c->window, uni->d1, uni->d2, uni->d3, uni->d4, FALSE);
	// An offscreen context's picture takes the new size.
	if (c->offscreen && (c->framebuffer.width != uni->d3 || c->framebuffer.height != uni->d4) && activate(c)) {
		ffp::flush();
		present::resize(c->framebuffer, uni->d3, uni->d4, uni_resolve);
	}
	return 0;
}

// d1 = context; a1 = the QtTarget an offscreen context's picture goes to
// (gl/gl.h), 0 for none.
QT_EXPORT int32_t __cdecl qt_swap_buffers(struct uni* uni) {
	Context* c = find(uni->d1);
	if (!c || !activate(c)) return 0;
	ffp::flush();
	if (!captureDir.empty() && c->swaps % captureEvery == 0) captureFrame(c);
	++c->swaps;
	LARGE_INTEGER before, after;
	if (c->profile.on) QueryPerformanceCounter(&before);
	if (!c->offscreen) SwapBuffers(c->deviceContext);
	else if (uni->a1) {
		const uint8_t* words = amiga<const uint8_t>(uni->a1);
		uint32_t w[9];
		for (int i = 0; i < 9; ++i) {
			memcpy(&w[i], words + 4 * i, 4);
			w[i] = qt_swap32(w[i]);
		}
		present::Target t = {w[0], w[1], w[2], w[3], w[4], static_cast<int32_t>(w[5]), static_cast<int32_t>(w[6]),
			static_cast<int32_t>(w[7]), static_cast<int32_t>(w[8])};
		present::start(c->framebuffer, t, uni_resolve);
	}
	if (c->profile.on) {
		QueryPerformanceCounter(&after);
		Profile& p = c->profile;
		p.swapTicks += after.QuadPart - before.QuadPart;
		if (!p.periodStart) p.periodStart = after.QuadPart;
		else if (c->swaps % reportFrames == 0) {
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
			double f = static_cast<double>(frequency.QuadPart), wall = (after.QuadPart - p.periodStart) / f;
			char line[240];
			snprintf(line, sizeof(line), "Profile %s: frames %lu-%lu: %.1f fps; per frame %.2f ms wall, %.2f ms executing "
				"(%lu buffers, %llu bytes), %.2f ms presenting (%.2f starting read backs, %.2f writing pictures), %.2f ms elsewhere (68k)",
				c->label.empty() ? "context" : c->label.c_str(), c->swaps - reportFrames, c->swaps, reportFrames / wall,
				wall * 1000 / reportFrames, p.periodTicks / f * 1000 / reportFrames, p.periodCalls / reportFrames,
				p.periodBytes / reportFrames, p.swapTicks / f * 1000 / reportFrames,
				present::readTicks / f * 1000 / reportFrames, present::writeTicks / f * 1000 / reportFrames,
				(wall - (p.periodTicks + p.swapTicks) / f) * 1000 / reportFrames);
			logString(line);
			p.periodStart = after.QuadPart;
			p.periodTicks = p.swapTicks = 0;
			present::readTicks = present::writeTicks = 0;
			p.periodCalls = 0;
			p.periodBytes = 0;
		}
	}

	GLenum code = glGetError();
	while (code != GL_NO_ERROR) {
		logString(reinterpret_cast<const char*>(gluErrorString(code)));
		code = glGetError();
	}
	return 0;
}

// d1 = context. Writes an offscreen context's pending picture now, waiting
// for the GPU if need be: the 68k side calls it before it waits for input,
// when no command buffer would come to write it.
QT_EXPORT int32_t __cdecl qt_finish_frame(struct uni* uni) {
	Context* c = find(uni->d1);
	if (c && c->offscreen && activate(c)) present::poll(c->framebuffer, uni_resolve, true);
	return 0;
}
