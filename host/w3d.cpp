// Warp3D on the host (docs/phase4-warp3d-on-host.md). Executes the Warp3D
// commands Warp3D.library writes to the command buffer, with the OpenGL calls
// the 68k code of QuarkTex 0.53 made.
//
// The unit test (tests/host) includes this file with QT_TEST defined and
// QT_GL pointing to recording stubs.
#ifndef QT_TEST
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#define QT_GL(name) gl##name
#endif
#include "gldecode.h"
#include "w3dcmd.h"

bool qt_w3d_decode(const Command& c, int32_t& result) {
	result = 0;
	switch (c.u(0) >> 16) {
	case QT_W3D_INIT_CONTEXT:
		if (c.words != 1) return false;
		QT_GL(Enable)(GL_TEXTURE_2D);
		QT_GL(ShadeModel)(GL_SMOOTH);
		return true;
	}
	return false;
}
