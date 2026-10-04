#include "w3d.h"

ULONG W3D_AllocZBuffer(__REGA0(W3D_Context *context)) {
	LOG;
	return W3D_SUCCESS;
}

ULONG W3D_FreeZBuffer(__REGA0(W3D_Context *context)) {
	LOG;
	return W3D_SUCCESS;
}

ULONG W3D_ClearZBuffer(__REGA0(W3D_Context *context), __REGA1(W3D_Double *clearvalue)) {
	LOG;
	w3d_command(QT_W3D_CLEAR_Z, 0);
	return W3D_SUCCESS;
}

/*
 * The depth buffer through the OpenGL command path (synchronous calls). It
 * holds floats; Warp3D hands out doubles. Warp3D counts y from the top of
 * the drawing area, glReadPixels from the bottom. (0.53 read floats into
 * doubles without converting them and did not turn y around.)
 *
 * QuarkTex passes Warp3D z (0..1) to OpenGL unprojected, so the depth buffer
 * holds (z + 1) / 2: reads return 2 * depth - 1, writes store (z + 1) / 2.
 */
ULONG W3D_ReadZSpan(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGD2(ULONG n), __REGA1(W3D_Double *z)) {
	float *depth;
	LOG;
	depth = (float*) malloc(n * sizeof(float));
	if (!depth) return W3D_NOMEMORY;
	_glReadPixels((long) x, height - 1 - (long) y, (long) n, 1, GL_DEPTH_COMPONENT, GL_FLOAT, depth);
	SWAP32(depth, n)
	for (i = 0; i < n; ++i) z[i] = 2.0 * depth[i] - 1.0;
	free(depth);
	return W3D_SUCCESS;
}

ULONG W3D_ReadZPixel(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGA1(W3D_Double *z)) {
	return W3D_ReadZSpan(context, x, y, 1, z);
}

ULONG W3D_SetZCompareMode(__REGA0(W3D_Context *context), __REGD1(ULONG mode)) {
	ULONG *w;
	LOG;
	w = w3d_command(QT_W3D_Z_COMPARE, 1);
	w[0] = mode;
	return W3D_SUCCESS;
}

/* One depth value with glDrawPixels at Warp3D position (x, y). The raster
 * position goes through the model view matrix, which maps Warp3D window
 * coordinates; the + 0.5 puts it on the pixel's centre. */
static void writeDepth(long x, long y, long n, float *depth) {
	_glRasterPos2f(x + 0.5f, y + 0.5f);
	_glDrawPixels(n, 1, GL_DEPTH_COMPONENT, GL_FLOAT, depth);
}

/*
 * Writes depth values as they are: the depth test passes everything and
 * depth writes are on while writing. (0.53 wrote the address of its
 * pointer, passed GL_DOUBLE, which glDrawPixels does not take, and put
 * masked pixels at x + n.)
 */
void W3D_WriteZSpan(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGD2(ULONG n), __REGA1(W3D_Double *z), __REGA2(UBYTE *mask)) {
	float *depth;
	LOG;
	depth = (float*) malloc(n * sizeof(float));
	if (!depth) return;
	for (i = 0; i < n; ++i) depth[i] = (float) ((z[i] + 1.0) / 2.0);
	SWAP32(depth, n)
	_glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
	_glEnable(GL_DEPTH_TEST);
	_glDepthFunc(GL_ALWAYS);
	_glDepthMask(GL_TRUE);
	/* glDrawPixels of depth also writes the current raster colour. */
	_glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	if (mask) {
		for (i = 0; i < n; ++i) if (mask[i]) writeDepth(x + i, y, 1, &depth[i]);
	}
	else writeDepth(x, y, n, depth);
	_glPopAttrib();
	free(depth);
}

void W3D_WriteZPixel(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGA1(W3D_Double *z)) {
	W3D_WriteZSpan(context, x, y, 1, z, NULL);
}
