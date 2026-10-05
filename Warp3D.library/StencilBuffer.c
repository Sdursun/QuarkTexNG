#include "w3d.h"

/*
 * The stencil buffer is the 8-bit one of the host's OpenGL context; it always
 * exists, W3D_AllocStencilBuffer only marks it as used. The host maps the
 * Warp3D functions and operations to OpenGL (gl/w3dcmd.h, host/w3d.cpp).
 * (0.53 had no stencil buffer: every call returned an error.)
 */

ULONG W3D_AllocStencilBuffer(__REGA0(W3D_Context *context)) {
	LOG;
	context->stbufferalloc = W3D_TRUE;
	return W3D_SUCCESS;
}

ULONG W3D_FreeStencilBuffer(__REGA0(W3D_Context *context)) {
	LOG;
	if (!context->stbufferalloc) return W3D_NOSTENCILBUFFER;
	context->stbufferalloc = W3D_FALSE;
	return W3D_SUCCESS;
}

ULONG W3D_ClearStencilBuffer(__REGA0(W3D_Context *context), __REGA1(ULONG *clearvalue)) {
	ULONG *w;
	LOG;
	if (!context->stbufferalloc) return W3D_NOSTENCILBUFFER;
	w = w3d_command(context, QT_W3D_STENCIL_CLEAR, 1);
	w[0] = clearvalue ? *clearvalue : 0;
	return W3D_SUCCESS;
}

static void writeStencil(W3D_Context *context, ULONG x, ULONG y, ULONG width, ULONG height, ULONG bytes, void *data, UBYTE *mask) {
	ULONG *w = w3d_command(context, QT_W3D_WRITE_STENCIL, 7);
	w[0] = x;
	w[1] = y;
	w[2] = width;
	w[3] = height;
	w[4] = bytes;
	w[5] = (ULONG) data;
	w[6] = (ULONG) mask;
	qt_flush();
}

/* depth: bits per value of data, 8, 16 or 32. */
ULONG W3D_FillStencilBuffer(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGD2(ULONG width), __REGD3(ULONG height), __REGD4(ULONG depth), __REGA1(void *data)) {
	LOG;
	if (!context->stbufferalloc) return W3D_NOSTENCILBUFFER;
	if ((depth != 8 && depth != 16 && depth != 32) || !data) return W3D_ILLEGALINPUT;
	writeStencil(context, x, y, width, height, depth / 8, data, NULL);
	return W3D_SUCCESS;
}

ULONG W3D_WriteStencilSpan(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGD2(ULONG n), __REGA1(ULONG *st), __REGA2(UBYTE *mask)) {
	LOG;
	if (!context->stbufferalloc) return W3D_NOSTENCILBUFFER;
	writeStencil(context, x, y, n, 1, 4, st, mask);
	return W3D_SUCCESS;
}

ULONG W3D_WriteStencilPixel(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGD2(ULONG st)) {
	return W3D_WriteStencilSpan(context, x, y, 1, &st, NULL);
}

ULONG W3D_ReadStencilSpan(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGD2(ULONG n), __REGA1(ULONG *st)) {
	ULONG *w;
	LOG;
	if (!context->stbufferalloc) return W3D_NOSTENCILBUFFER;
	w = w3d_command(context, QT_W3D_READ_STENCIL, 4);
	w[0] = x;
	w[1] = QT(context)->height - 1 - (long) y;
	w[2] = n;
	w[3] = (ULONG) st;
	qt_flush();
	return W3D_SUCCESS;
}

ULONG W3D_ReadStencilPixel(__REGA0(W3D_Context *context), __REGD0(ULONG x), __REGD1(ULONG y), __REGA1(ULONG *st)) {
	return W3D_ReadStencilSpan(context, x, y, 1, st);
}

ULONG W3D_SetStencilFunc(__REGA0(W3D_Context *context), __REGD0(ULONG func), __REGD1(ULONG refvalue), __REGD2(ULONG mask)) {
	ULONG *w;
	LOG;
	if (func < W3D_ST_NEVER || func > W3D_ST_NOTEQUAL) return W3D_UNSUPPORTEDSTTEST;
	w = w3d_command(context, QT_W3D_STENCIL_FUNC, 3);
	w[0] = func;
	w[1] = refvalue;
	w[2] = mask;
	return W3D_SUCCESS;
}

ULONG W3D_SetStencilOp(__REGA0(W3D_Context *context), __REGD0(ULONG sfail), __REGD1(ULONG dpfail), __REGD2(ULONG dppass)) {
	ULONG *w;
	LOG;
	if (sfail < W3D_ST_KEEP || sfail > W3D_ST_INVERT || dpfail < W3D_ST_KEEP || dpfail > W3D_ST_INVERT
			|| dppass < W3D_ST_KEEP || dppass > W3D_ST_INVERT) return W3D_UNSUPPORTEDSTTEST;
	w = w3d_command(context, QT_W3D_STENCIL_OP, 3);
	w[0] = sfail;
	w[1] = dpfail;
	w[2] = dppass;
	return W3D_SUCCESS;
}

ULONG W3D_SetWriteMask(__REGA0(W3D_Context *context), __REGD1(ULONG mask)) {
	ULONG *w;
	LOG;
	w = w3d_command(context, QT_W3D_STENCIL_MASK, 1);
	w[0] = mask;
	return W3D_SUCCESS;
}
