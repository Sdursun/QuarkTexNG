#include "w3d.h"

/* The host maps the Warp3D values to OpenGL (host/w3d.cpp). */

static inline ULONG f2l(float f) {
	union { float f; ULONG l; } u;
	u.f = f;
	return u.l;
}

ULONG W3D_SetAlphaMode(__REGA0(W3D_Context *context), __REGD1(ULONG mode), __REGA1(W3D_Float *refval)) {
	ULONG *w;
	LOG;
	w = w3d_command(context, QT_W3D_ALPHA_MODE, 2);
	w[0] = mode;
	w[1] = f2l(*refval);
	return W3D_SUCCESS;
}
ULONG W3D_SetBlendMode(__REGA0(W3D_Context *context), __REGD0(ULONG srcfunc), __REGD1(ULONG dstfunc)) {
	ULONG *w;
	LOG;
	w = w3d_command(context, QT_W3D_BLEND_MODE, 2);
	w[0] = srcfunc;
	w[1] = dstfunc;
	return W3D_SUCCESS;
}
/* A new draw region: an offscreen context's frame drawn so far goes into the
 * old one (Context.c), the next into this one; a host window context
 * presents, as in 0.53. */
ULONG W3D_SetDrawRegion(__REGA0(W3D_Context *context), __REGA1(struct BitMap *bm), __REGD1(int yoffset), __REGA2(W3D_Scissor *scissor)) {
	QtContext *qt = QT(context);
	LOG;
	if (qt->offscreen) {
		w3d_present(qt);
		if (!qt->window && presentable(bm)) {
			qt->bitmap = bm;
			qt->yoffset = yoffset;
		}
	}
	else if (qt->fullscreen) {
		w3d_select(context);
		swapBuffers();
	}
	w3d_describe(context, bm, yoffset);
	return W3D_SUCCESS;
}
ULONG W3D_SetDrawRegionWBM(__REGA0(W3D_Context *context), __REGA1(W3D_Bitmap *bm), __REGA2(W3D_Scissor *scissor)) {
	QtContext *qt = QT(context);
	LOG;
	if (qt->offscreen) w3d_present(qt);
	else if (qt->fullscreen) {
		w3d_select(context);
		swapBuffers();
	}
	return W3D_SUCCESS;
}

ULONG W3D_SetFogParams(__REGA0(W3D_Context *context), __REGA1(W3D_Fog *fogparams), __REGD1(ULONG fogmode)) {
	ULONG *w;
	LOG;
	w = w3d_command(context, QT_W3D_FOG, 7);
	w[0] = fogmode;
	w[1] = f2l(fogparams->fog_start);
	w[2] = f2l(fogparams->fog_end);
	w[3] = f2l(fogparams->fog_density);
	w[4] = f2l(fogparams->fog_color.r);
	w[5] = f2l(fogparams->fog_color.g);
	w[6] = f2l(fogparams->fog_color.b);
	return W3D_SUCCESS;
}
ULONG W3D_SetLogicOp(__REGA0(W3D_Context *context), __REGD1(ULONG operation)) {
	ULONG *w;
	LOG;
	w = w3d_command(context, QT_W3D_LOGIC_OP, 1);
	w[0] = operation;
	return W3D_SUCCESS;
}
ULONG W3D_SetColorMask(__REGA0(W3D_Context *context), __REGD0(W3D_Bool red), __REGD1(W3D_Bool green), __REGD2(W3D_Bool blue), __REGD3(W3D_Bool alpha)) {
	ULONG *w;
	LOG;
	w = w3d_command(context, QT_W3D_COLOR_MASK, 4);
	w[0] = (GLboolean) red;
	w[1] = (GLboolean) green;
	w[2] = (GLboolean) blue;
	w[3] = (GLboolean) alpha;
	return W3D_SUCCESS;
}

ULONG W3D_SetPenMask(__REGA0(W3D_Context *context), __REGD1(ULONG pen)) {
	LOG;
	return W3D_SUCCESS;
}

ULONG W3D_SetCurrentColor(__REGA0(W3D_Context *context), __REGA1(W3D_Color *color)) {
	ULONG *w;
	LOG;
	w = w3d_command(context, QT_W3D_CURRENT_COLOR, 4);
	w[0] = f2l(color->r);
	w[1] = f2l(color->g);
	w[2] = f2l(color->b);
	w[3] = f2l(color->a);
	return W3D_SUCCESS;
}

ULONG W3D_SetCurrentPen(__REGA0(W3D_Context *context), __REGD1(ULONG pen)) {
	LOG;
	return W3D_SUCCESS;
}

/* OpenGL counts y from the bottom of the drawing area. */
void W3D_SetScissor(__REGA0(W3D_Context *context), __REGA1(W3D_Scissor *scissor)) {
	ULONG *w;
	LOG;
	w = w3d_command(context, QT_W3D_SCISSOR, 4);
	w[0] = scissor->left;
	w[1] = QT(context)->height - (scissor->top + scissor->height);
	w[2] = scissor->width;
	w[3] = scissor->height;
}

/* The frame is complete: an offscreen context's goes into display memory. */
void W3D_FlushFrame(__REGA0(W3D_Context *context)) {
	LOG;
	w3d_select(context);
	_glFinish();
	w3d_present(QT(context));
}
