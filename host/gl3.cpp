#include "gl3.h"

namespace gl3 {
#define QT_GL3_DEFINE(result, name, parameters) result (APIENTRY* name) parameters = 0;
	QT_GL3_FUNCTIONS(QT_GL3_DEFINE)
	QT_GL43_FUNCTIONS(QT_GL3_DEFINE)
	QT_GL_OPTIONAL_FUNCTIONS(QT_GL3_DEFINE)
#undef QT_GL3_DEFINE

	namespace {
		bool all43 = false;
	}

	bool load() {
		bool all = true;
#define QT_GL3_LOAD(result, name, parameters) \
		name = reinterpret_cast<result (APIENTRY*) parameters>(reinterpret_cast<void*>(wglGetProcAddress("gl" #name))); \
		if (!name) all = false;
		QT_GL3_FUNCTIONS(QT_GL3_LOAD)
		bool required = all;
		all = true;
		QT_GL43_FUNCTIONS(QT_GL3_LOAD)
		all43 = all;
		QT_GL_OPTIONAL_FUNCTIONS(QT_GL3_LOAD)
#undef QT_GL3_LOAD
		return required;
	}

	bool has43() { return all43; }

	HGLRC createCoreContext(HDC deviceContext, int* version, HGLRC share) {
		typedef HGLRC (WINAPI* CreateContextAttribs)(HDC, HGLRC, const int*);
		// wglCreateContextAttribsARB needs a current context to be looked up.
		HGLRC legacy = wglCreateContext(deviceContext);
		if (!legacy) return 0;
		HGLRC core = 0;
		if (wglMakeCurrent(deviceContext, legacy)) {
			CreateContextAttribs create = reinterpret_cast<CreateContextAttribs>(
				reinterpret_cast<void*>(wglGetProcAddress("wglCreateContextAttribsARB")));
			static const int versions[][2] = {{4, 3}, {3, 3}};
			for (int i = 0; create && !core && i < 2; ++i) {
				const int attributes[] = {
					WGL_CONTEXT_MAJOR_VERSION_ARB, versions[i][0],
					WGL_CONTEXT_MINOR_VERSION_ARB, versions[i][1],
					WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
					0
				};
				core = create(deviceContext, share, attributes);
				if (core && version) *version = versions[i][0] * 10 + versions[i][1];
			}
			wglMakeCurrent(0, 0);
		}
		wglDeleteContext(legacy);
		return core;
	}
}
