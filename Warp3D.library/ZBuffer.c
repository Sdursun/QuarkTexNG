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

ULONG W3D_ReadZPixel(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGA1(W3D_Double *z)) {
	LOG;
	_glReadPixels((long) x, (long) y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, (GLvoid*) (memoffset + (long) z));
	SWAP64(z, 1)
	return W3D_SUCCESS;
}
ULONG W3D_ReadZSpan(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGD2(ULONG n), __REGA1(W3D_Double *z)) {
	LOG;
	_glReadPixels((long) x, (long) y, (long) n, 1, GL_DEPTH_COMPONENT, GL_FLOAT, (GLvoid*) (memoffset + (long) z));
	SWAP64(z, n)
	return W3D_SUCCESS;
}

ULONG W3D_SetZCompareMode(__REGA0(W3D_Context *context), __REGD1(ULONG mode)) {
	ULONG *w;
	LOG;
	w = w3d_command(QT_W3D_Z_COMPARE, 1);
	w[0] = mode;
	return W3D_SUCCESS;
}
void W3D_WriteZPixel(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGA1(W3D_Double *z)) {
	float temp;
	LOG;
	temp = *z;
	SWAP32((&temp), 1)
	_glRasterPos2i((long) x, (long) y);
	_glDrawPixels(1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, (GLvoid*) (memoffset + (long) &z));
}
void W3D_WriteZSpan(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGD2(ULONG n), __REGA1(W3D_Double *z), __REGA2(UBYTE *mask)) {
	double* temp;
	LOG;
	temp = (double*) malloc(n * sizeof(double));
	for (i = 0; i < n; ++i) temp[i] = z[i];
	SWAP64(temp, n)
	if (mask) {
		for (i = 0; i < n; ++i) if (mask[i]) {
			_glRasterPos2i((long) (x + n), (long) y);
			_glDrawPixels(1, 1, GL_DEPTH_COMPONENT, GL_DOUBLE, (GLvoid*) (memoffset + (long) z));
		}
	}
	else {
		_glRasterPos2i((long) x, (long) y);
		_glDrawPixels((long) n, 1, GL_DEPTH_COMPONENT, GL_DOUBLE, (GLvoid*) (memoffset + (long) z));
	}
	free(temp);
}