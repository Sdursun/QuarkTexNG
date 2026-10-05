#include "w3d.h"

/* The host draws (host/w3d.cpp); see drawPrimitive in w3d.c. */

ULONG W3D_DrawLine(__REGA0(W3D_Context *context), __REGA1(W3D_Line *line)) {
	LOG;
	*w3d_command(context, QT_W3D_LINE_WIDTH, 1) = w3d_float(line->linewidth);
	drawPrimitive(context, GL_LINES, line->tex, &line->v1, 2);
	return W3D_SUCCESS;
}
ULONG W3D_DrawPoint(__REGA0(W3D_Context *context), __REGA1(W3D_Point *point)) {
	LOG;
	*w3d_command(context, QT_W3D_POINT_SIZE, 1) = w3d_float(point->pointsize);
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
	w3d_select(context);
	_glFinish();
	return W3D_SUCCESS;
}
ULONG W3D_DrawLineStrip(__REGA0(W3D_Context *context), __REGA1(W3D_Lines *lines)) {
	LOG;
	*w3d_command(context, QT_W3D_LINE_WIDTH, 1) = w3d_float(lines->linewidth);
	drawPrimitive(context, GL_LINE_STRIP, lines->tex, lines->v, lines->vertexcount);
	return W3D_SUCCESS;
}

ULONG W3D_DrawLineLoop(__REGA0(W3D_Context *context), __REGA1(W3D_Lines *lines)) {
	LOG;
	*w3d_command(context, QT_W3D_LINE_WIDTH, 1) = w3d_float(lines->linewidth);
	drawPrimitive(context, GL_LINE_LOOP, lines->tex, lines->v, lines->vertexcount);
	return W3D_SUCCESS;
}

ULONG W3D_ClearDrawRegion(__REGA0(W3D_Context *context), __REGD0(ULONG color)) {
	ULONG *w;
	LOG;
	w = w3d_command(context, QT_W3D_CLEAR, 4);
	w[0] = color;
	w[1] = QT(context)->fullscreen != 0;
	w[2] = QT(context)->width;
	w[3] = QT(context)->height;
	return W3D_SUCCESS;
}
