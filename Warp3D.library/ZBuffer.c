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
 * The depth buffer, read and written by the host (READ_Z and WRITE_Z in
 * gl/w3dcmd.h), which converts between Warp3D z and the (z + 1) / 2 in the
 * OpenGL depth buffer. Warp3D counts y from the top of the drawing area,
 * glReadPixels from the bottom. (0.53 read floats into doubles without
 * converting them, did not turn y around, wrote the address of its pointer
 * and put masked pixels at x + n.)
 */
ULONG W3D_ReadZSpan(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGD2(ULONG n), __REGA1(W3D_Double *z)) {
	ULONG *w;
	LOG;
	w = w3d_command(QT_W3D_READ_Z, 4);
	w[0] = x;
	w[1] = height - 1 - (long) y;
	w[2] = n;
	w[3] = (ULONG) z;
	qt_flush();
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

void W3D_WriteZSpan(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGD2(ULONG n), __REGA1(W3D_Double *z), __REGA2(UBYTE *mask)) {
	ULONG *w;
	LOG;
	w = w3d_command(QT_W3D_WRITE_Z, 5);
	w[0] = x;
	w[1] = y;
	w[2] = n;
	w[3] = (ULONG) z;
	w[4] = (ULONG) mask;
	qt_flush();
}

void W3D_WriteZPixel(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGA1(W3D_Double *z)) {
	W3D_WriteZSpan(context, x, y, 1, z, NULL);
}
