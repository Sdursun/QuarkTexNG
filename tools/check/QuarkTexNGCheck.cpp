// QuarkTexNGCheck: tells a user whether this PC's graphics driver can run
// QuarkTex NG, and shows what it has: the graphics card, its OpenGL and
// GLSL versions and the highest core profile it creates.
//
// QuarkTex NG's host library draws with an OpenGL 4.3 core profile context
// (host/gl3.cpp), so that decides it: "QuarkTex NG support = PASS" when one
// can be made. With only 3.3 the host still runs everything but ogles3.
// library's OpenGL ES 3.1 (compute shaders, docs/phase10-ogles3.md), which
// the FAIL says.
//
// Started from Explorer (a console of its own) it shows a window, with the
// result in colour and a button that copies the report; from a command line
// it prints the report there. /gui opens the window from a command line too.
// Exit code 0 for PASS, 1 for FAIL.
//
// Build:  cl /O2 /EHsc tools\check\QuarkTexNGCheck.cpp opengl32.lib gdi32.lib user32.lib
//         (or the CMake target QuarkTexNGCheck; build.sh host with MinGW)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <cstdio>
#include <cstring>
#include <string>

namespace {
	typedef HGLRC (WINAPI* CreateContextAttribs)(HDC, HGLRC, const int*);

	const int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
	const int WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
	const int WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
	const int WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x0001;
	const GLenum GL_SHADING_LANGUAGE_VERSION_ = 0x8B8C;

	struct Result {
		bool pass;
		std::string details; // the table, one "name  value" line each
		std::string reason;  // why not, for a FAIL
	};

	const char* text(const GLubyte* s) { return s ? reinterpret_cast<const char*>(s) : "(none)"; }

	// The driver's own part of a GL_VERSION string, after the version
	// number: "Build 30.0.101.1934" (Intel), "NVIDIA 560.94", ...
	const char* driverPart(const char* version) {
		const char* p = version;
		while (*p && *p != ' ') ++p;
		while (*p == ' ' || *p == '-') ++p;
		return *p ? p : version;
	}

	void row(std::string& out, const char* name, const char* value) {
		char line[512];
		std::snprintf(line, sizeof(line), "%-24s %s\r\n", name, value);
		out += line;
	}

	// The highest core profile from 4.6 down to 3.0 the driver creates;
	// false if none (0.0).
	bool highestCore(HDC dc, CreateContextAttribs create, int& major, int& minor) {
		static const int versions[][2] = {{4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0}, {3, 3}, {3, 2}, {3, 1}, {3, 0}};
		for (size_t i = 0; create && i < sizeof(versions) / sizeof(versions[0]); ++i) {
			const int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, versions[i][0], WGL_CONTEXT_MINOR_VERSION_ARB, versions[i][1],
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0};
			HGLRC core = create(dc, 0, attribs);
			if (core) {
				wglDeleteContext(core);
				major = versions[i][0];
				minor = versions[i][1];
				return true;
			}
		}
		major = minor = 0;
		return false;
	}

	Result check() {
		Result r;
		r.pass = false;
		WNDCLASSA wc = {};
		wc.lpfnWndProc = DefWindowProcA;
		wc.hInstance = GetModuleHandleA(0);
		wc.lpszClassName = "QuarkTexNGCheckGL";
		RegisterClassA(&wc);
		HWND window = CreateWindowA(wc.lpszClassName, "", WS_OVERLAPPEDWINDOW, 0, 0, 64, 64, 0, 0, wc.hInstance, 0);
		HDC dc = window ? GetDC(window) : 0;
		PIXELFORMATDESCRIPTOR pfd = {};
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		int format = dc ? ChoosePixelFormat(dc, &pfd) : 0;
		HGLRC legacy = 0;
		if (format && SetPixelFormat(dc, format, &pfd)) legacy = wglCreateContext(dc);
		if (!legacy || !wglMakeCurrent(dc, legacy)) r.reason = "no OpenGL rendering context could be made";
		else {
			const char* renderer = text(glGetString(GL_RENDERER));
			const char* version = text(glGetString(GL_VERSION));
			row(r.details, "Graphics card", renderer);
			row(r.details, "Vendor", text(glGetString(GL_VENDOR)));
			row(r.details, "Driver", driverPart(version));
			row(r.details, "OpenGL (compatibility)", version);
			row(r.details, "GLSL", text(glGetString(GL_SHADING_LANGUAGE_VERSION_)));
			int major = 0, minor = 0;
			CreateContextAttribs create =
				reinterpret_cast<CreateContextAttribs>(reinterpret_cast<void*>(wglGetProcAddress("wglCreateContextAttribsARB")));
			char core[32];
			if (highestCore(dc, create, major, minor)) std::snprintf(core, sizeof(core), "%d.%d", major, minor);
			else std::snprintf(core, sizeof(core), "none");
			row(r.details, "Highest core profile", core);
			row(r.details, "Needed", "OpenGL 4.3 core profile");
			r.pass = major > 4 || (major == 4 && minor >= 3);
			if (!r.pass) {
				if (!std::strncmp(renderer, "GDI Generic", 11))
					r.reason = "only Windows' own software OpenGL 1.1: install the graphics card's driver";
				else if (major > 3 || (major == 3 && minor >= 3))
					r.reason = "only OpenGL 3.3: Warp3D, MiniGL, StormMESA and OpenGL ES 2.0/3.0 run, OpenGL ES 3.1 does not; "
						"a newer graphics driver may have 4.3";
				else r.reason = "the driver creates no OpenGL 3.3 core profile context: update the graphics driver";
			}
			wglMakeCurrent(0, 0);
		}
		if (legacy) wglDeleteContext(legacy);
		if (dc) ReleaseDC(window, dc);
		if (window) DestroyWindow(window);
		return r;
	}

	std::string verdict(const Result& r) { return std::string("QuarkTex NG support = ") + (r.pass ? "PASS" : "FAIL"); }

	// The whole report as text, for the console and the clipboard.
	std::string report(const Result& r) {
		std::string out = "QuarkTex NG system check\r\n\r\n" + r.details + "\r\n" + verdict(r) + "\r\n";
		if (!r.reason.empty()) out += "(" + r.reason + ")\r\n";
		return out;
	}

	// --- The window ---------------------------------------------------------

	enum { ID_COPY = 1, ID_CLOSE = 2 };

	const Result* shown;
	HFONT textFont, verdictFont;
	HBRUSH background;
	HWND verdictLabel;

	int scaled(int pixels) {
		HDC dc = GetDC(0);
		int dpi = GetDeviceCaps(dc, LOGPIXELSY);
		ReleaseDC(0, dc);
		return MulDiv(pixels, dpi, 96);
	}

	void copyReport(HWND window) {
		std::string text = report(*shown);
		if (!OpenClipboard(window)) return;
		EmptyClipboard();
		HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
		if (memory) {
			memcpy(GlobalLock(memory), text.c_str(), text.size() + 1);
			GlobalUnlock(memory);
			if (!SetClipboardData(CF_TEXT, memory)) GlobalFree(memory);
		}
		CloseClipboard();
	}

	LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_CREATE: {
			const int margin = scaled(12), width = scaled(560), lineHeight = scaled(18);
			HWND details = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", shown->details.c_str(),
				WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL, margin, margin, width, lineHeight * 9,
				window, 0, 0, 0);
			SendMessageA(details, WM_SETFONT, reinterpret_cast<WPARAM>(textFont), TRUE);
			int y = margin * 2 + lineHeight * 9;
			verdictLabel = CreateWindowA("STATIC", verdict(*shown).c_str(), WS_CHILD | WS_VISIBLE | SS_CENTER, margin, y, width,
				scaled(34), window, 0, 0, 0);
			SendMessageA(verdictLabel, WM_SETFONT, reinterpret_cast<WPARAM>(verdictFont), TRUE);
			y += scaled(36);
			if (!shown->reason.empty()) {
				HWND reason = CreateWindowA("STATIC", shown->reason.c_str(), WS_CHILD | WS_VISIBLE | SS_CENTER, margin, y, width,
					scaled(36), window, 0, 0, 0);
				SendMessageA(reason, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
				y += scaled(38);
			}
			const int buttonWidth = scaled(110), buttonHeight = scaled(28);
			HWND copy = CreateWindowA("BUTTON", "Copy report", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
				margin + width - 2 * buttonWidth - margin, y, buttonWidth, buttonHeight, window,
				reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_COPY)), 0, 0);
			HWND close = CreateWindowA("BUTTON", "Close", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				margin + width - buttonWidth, y, buttonWidth, buttonHeight, window,
				reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_CLOSE)), 0, 0);
			SendMessageA(copy, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
			SendMessageA(close, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
			SetFocus(close);
			// The window's outer size for that client area.
			RECT r = {0, 0, margin * 2 + width, y + buttonHeight + margin};
			AdjustWindowRect(&r, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);
			SetWindowPos(window, 0, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER);
			return 0;
		}
		case WM_CTLCOLORSTATIC:
			// The verdict in green or red; the read-only details stay black.
			SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
			if (reinterpret_cast<HWND>(lParam) == verdictLabel)
				SetTextColor(reinterpret_cast<HDC>(wParam), shown->pass ? RGB(0, 128, 0) : RGB(192, 0, 0));
			return reinterpret_cast<LRESULT>(background);
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_COPY) copyReport(window);
			else if (LOWORD(wParam) == ID_CLOSE || LOWORD(wParam) == IDCANCEL) DestroyWindow(window);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProcA(window, message, wParam, lParam);
	}

	void showWindow(const Result& r) {
		shown = &r;
		background = GetSysColorBrush(COLOR_BTNFACE);
		textFont = CreateFontA(-scaled(13), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FIXED_PITCH,
			"Consolas");
		verdictFont = CreateFontA(-scaled(22), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI");
		WNDCLASSA wc = {};
		wc.lpfnWndProc = windowProc;
		wc.hInstance = GetModuleHandleA(0);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);
		wc.hbrBackground = background;
		wc.lpszClassName = "QuarkTexNGCheck";
		RegisterClassA(&wc);
		HWND window = CreateWindowA(wc.lpszClassName, "QuarkTex NG system check", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, 0, 0, wc.hInstance, 0);
		ShowWindow(window, SW_SHOW);
		MSG message;
		while (GetMessageA(&message, 0, 0, 0) > 0) {
			if (IsDialogMessageA(window, &message)) continue;
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}
		DeleteObject(textFont);
		DeleteObject(verdictFont);
	}

	// Whether this process has the console to itself (started from
	// Explorer): then the window, not the console, shows the report.
	bool ownConsole() {
		DWORD processes[2];
		return GetConsoleProcessList(processes, 2) == 1;
	}
}

int main(int argc, char** argv) {
	bool gui = ownConsole();
	for (int i = 1; i < argc; ++i) {
		if (!_stricmp(argv[i], "/gui")) gui = true;
		else if (!_stricmp(argv[i], "/console")) gui = false;
	}
	if (gui) {
		// The console Explorer opened for it goes at once.
		if (ownConsole()) FreeConsole();
		SetProcessDPIAware();
	}
	Result r = check();
	if (gui) showWindow(r);
	else {
		std::string text = report(r);
		// The console takes \n; the clipboard and the window \r\n.
		std::string plain;
		for (size_t i = 0; i < text.size(); ++i) if (text[i] != '\r') plain += text[i];
		std::fputs(plain.c_str(), stdout);
	}
	return r.pass ? 0 : 1;
}
