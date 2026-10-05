#include "w3d.h"


ULONG W3D_VertexPointer(__REGA0(W3D_Context* context), __REGA1(void *pointer), __REGD0(int stride), __REGD1(ULONG mode), __REGD2(ULONG flags)) {
	LOG;
	context->VertexPointer = pointer;
	context->VPStride = stride;
	context->VPMode = mode;
	context->VPFlags = flags;
	return W3D_SUCCESS;
}
ULONG W3D_TexCoordPointer(__REGA0(W3D_Context* context), __REGA1(void *pointer), __REGD0(int stride), __REGD1(int unit), __REGD2(int off_v), __REGD3(int off_w), __REGD4(ULONG flags)) {
	LOG;
	context->TexCoordPointer[0] = pointer;
	context->TPStride[0] = stride;
	context->TPVOffs[0] = off_v;
	context->TPWOffs[0] = off_w;
	context->TPFlags[0] = flags;
	return W3D_SUCCESS;
}
ULONG W3D_ColorPointer(__REGA0(W3D_Context* context), __REGA1(void *pointer), __REGD0(int stride), __REGD1(ULONG format), __REGD2(ULONG mode), __REGD3(ULONG flags)) {
	LOG;
	context->ColorPointer = (UBYTE*) pointer;
	context->CPStride = stride;
	context->CPMode = mode | format;
	context->CPFlags = flags;
	return W3D_SUCCESS;
}
ULONG W3D_BindTexture(__REGA0(W3D_Context* context), __REGD0(ULONG tmu), __REGA1(W3D_Texture *texture)) {
	LOG;
	context->CurrentTex[0] = texture;
	return W3D_SUCCESS;
}

/*
 * The host reads the arrays and draws (host/w3d.cpp). The command is
 * synchronous: the arrays only have to stay valid during the call.
 */
static void drawArray(W3D_Context* context, ULONG primitive, ULONG indexType, void *indices, ULONG first, ULONG count) {
	W3D_Texture *tex = (context->state & W3D_TEXMAPPING) ? context->CurrentTex[0] : NULL;
	ULONG *w = w3d_command(context, QT_W3D_DRAW_ARRAY, 21);
	w[0] = primitive;
	w[1] = context->state;
	w[2] = tex != NULL;
	w[3] = tex ? ((Texture*) tex->driver)->glID : 0;
	/* 0.53 divided by the texture's size even without a bound texture. */
	w[4] = context->CurrentTex[0] ? context->CurrentTex[0]->texwidth : 0;
	w[5] = context->CurrentTex[0] ? context->CurrentTex[0]->texheight : 0;
	w[6] = (ULONG) context->VertexPointer;
	w[7] = context->VPStride;
	w[8] = context->VPMode;
	w[9] = (ULONG) context->ColorPointer;
	w[10] = context->CPStride;
	w[11] = context->CPMode;
	w[12] = (ULONG) context->TexCoordPointer[0];
	w[13] = context->TPStride[0];
	w[14] = context->TPVOffs[0];
	w[15] = context->TPWOffs[0];
	w[16] = context->TPFlags[0];
	w[17] = indexType;
	w[18] = (ULONG) indices;
	w[19] = first;
	w[20] = count;
	qt_flush();
}

ULONG W3D_DrawArray(__REGA0(W3D_Context* context), __REGD0(ULONG primitive), __REGD1(ULONG base), __REGD2(ULONG count)) {
	LOG;
	drawArray(context, primitive, QT_W3D_NO_INDEX, NULL, base, count);
	return W3D_SUCCESS;
}

ULONG W3D_DrawElements(__REGA0(W3D_Context* context), __REGD0(ULONG primitive), __REGD1(ULONG type), __REGD2(ULONG count), __REGA1(void *indices)) {
	LOG;
	drawArray(context, primitive, type, indices, 0, count);
	return W3D_SUCCESS;
}

GLenum face[] = { GL_CW, GL_CCW };

void W3D_SetFrontFace(__REGA0(W3D_Context* context), __REGD0(ULONG direction)) {
	LOG;
	w3d_select(context);
	_glFrontFace(face[direction]);
}