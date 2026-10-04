#include "w3d.h"

/* The host draws (host/w3d.cpp); see drawPrimitive in w3d.c. */

ULONG W3D_DrawLine(__REGA0(W3D_Context *context), __REGA1(W3D_Line *line)) {
	LOG;
	drawPrimitive(context, GL_LINES, line->tex, &line->v1, 2);
	return W3D_SUCCESS;
}
ULONG W3D_DrawPoint(__REGA0(W3D_Context *context), __REGA1(W3D_Point *point)) {
	LOG;
	drawPrimitive(context, GL_POINTS, point->tex, &point->v1, 1);
	return W3D_SUCCESS;
}
ULONG W3D_DrawTriangle(__REGA0(W3D_Context *context), __REGA1(W3D_Triangle *triangle)) {
	LOG;
	drawPrimitive(context, GL_TRIANGLES, triangle->tex, &triangle->v1, 3);
	return W3D_SUCCESS;
}
ULONG W3D_DrawTriFan(__REGA0(W3D_Context *context), __REGA1(W3D_Triangles *triangles)) {
	LOG;
	drawPrimitive(context, GL_TRIANGLE_FAN, triangles->tex, triangles->v, triangles->vertexcount);
	return W3D_SUCCESS;
}
ULONG W3D_DrawTriStrip(__REGA0(W3D_Context *context), __REGA1(W3D_Triangles *triangles)) {
	LOG;
	drawPrimitive(context, GL_TRIANGLE_STRIP, triangles->tex, triangles->v, triangles->vertexcount);
	return W3D_SUCCESS;
}
ULONG W3D_Flush(__REGA0(W3D_Context *context)) {
	LOG;
	_glFinish();
	return W3D_SUCCESS;
}
ULONG W3D_DrawLineStrip(__REGA0(W3D_Context *context), __REGA1(W3D_Lines *lines)) {
	LOG;
	drawPrimitive(context, GL_LINE_STRIP, lines->tex, lines->v, lines->vertexcount);
	return W3D_SUCCESS;
}

ULONG W3D_DrawLineLoop(__REGA0(W3D_Context *context), __REGA1(W3D_Lines *lines)) {
	LOG;
	drawPrimitive(context, GL_LINE_LOOP, lines->tex, lines->v, lines->vertexcount);
	return W3D_SUCCESS;
}

ULONG W3D_ClearDrawRegion(__REGA0(W3D_Context *context), __REGD0(ULONG color)) {
	ULONG *w;
	LOG;
	w = w3d_command(QT_W3D_CLEAR, 4);
	w[0] = color;
	w[1] = fullscreen != 0;
	w[2] = width;
	w[3] = height;
	return W3D_SUCCESS;
}
