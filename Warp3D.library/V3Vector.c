#include "w3d.h"

ULONG W3D_DrawTriangleV(__REGA0(W3D_Context *context), __REGA1(W3D_TriangleV *triangle)) {
	W3D_Vertex *v[3];
	LOG;
	v[0] = triangle->v1;
	v[1] = triangle->v2;
	v[2] = triangle->v3;
	drawPrimitiveList(context, GL_TRIANGLES, triangle->tex, v, 3);
	return W3D_SUCCESS;
}

ULONG W3D_DrawTriFanV(__REGA0(W3D_Context *context), __REGA1(W3D_TrianglesV *triangles)) {
	LOG;
	drawPrimitiveList(context, GL_TRIANGLE_FAN, triangles->tex, triangles->v, triangles->vertexcount);
	return W3D_SUCCESS;
}

ULONG W3D_DrawTriStripV(__REGA0(W3D_Context *context), __REGA1(W3D_TrianglesV *triangles)) {
	LOG;
	drawPrimitiveList(context, GL_TRIANGLE_STRIP, triangles->tex, triangles->v, triangles->vertexcount);
	return W3D_SUCCESS;
}
