#include "gl3.h"

namespace gl3 {
#define QT_GL3_DEFINE(result, name, parameters) result (APIENTRY* name) parameters = 0;
	QT_GL3_FUNCTIONS(QT_GL3_DEFINE)
#undef QT_GL3_DEFINE

	bool load() {
		bool all = true;
#define QT_GL3_LOAD(result, name, parameters) \
		name = reinterpret_cast<result (APIENTRY*) parameters>(reinterpret_cast<void*>(wglGetProcAddress("gl" #name))); \
		if (!name) all = false;
		QT_GL3_FUNCTIONS(QT_GL3_LOAD)
#undef QT_GL3_LOAD
		return all;
	}

	HGLRC createCoreContext(HDC deviceContext) {
		typedef HGLRC (WINAPI* CreateContextAttribs)(HDC, HGLRC, const int*);
		// wglCreateContextAttribsARB needs a current context to be looked up.
		HGLRC legacy = wglCreateContext(deviceContext);
		if (!legacy) return 0;
		HGLRC core = 0;
		if (wglMakeCurrent(deviceContext, legacy)) {
			CreateContextAttribs create = reinterpret_cast<CreateContextAttribs>(
				reinterpret_cast<void*>(wglGetProcAddress("wglCreateContextAttribsARB")));
			const int attributes[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0
			};
			if (create) core = create(deviceContext, 0, attributes);
			wglMakeCurrent(0, 0);
		}
		wglDeleteContext(legacy);
		return core;
	}
}
