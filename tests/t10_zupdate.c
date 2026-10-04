/*
 * W3D_ZBUFFERUPDATE switches depth writes and nothing else.
 * Left: with updates off, a near red quad leaves no depth, so the far blue
 * quad drawn after it covers it where they overlap.
 * Right: with updates on and blending off, a half transparent green quad is
 * drawn opaque over white.
 * (QuarkTex 0.53 never turned depth writes off, and the missing break after
 * W3D_ZBUFFERUPDATE in W3D_SetState switched blending instead.)
 */
#include "common.h"

const char test_name[] = "t10_zupdate";

int test_setup(void) {
	ULONG result;
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	W3D_SetState(context, W3D_BLENDING, W3D_DISABLE);
	W3D_SetBlendMode(context, W3D_SRC_ALPHA, W3D_ONE_MINUS_SRC_ALPHA);
	result = W3D_AllocZBuffer(context);
	if (result != W3D_SUCCESS) fail("W3D_AllocZBuffer", result);
	W3D_SetZCompareMode(context, W3D_Z_LESS);
	return 0;
}

static void quad(float x, float y, float z, float r, float g, float b, float a) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], x, y, z, r, g, b, a);
	set_vertex(&v[1], x + 100, y, z, r, g, b, a);
	set_vertex(&v[2], x + 100, y + 100, z, r, g, b, a);
	set_vertex(&v[3], x, y + 100, z, r, g, b, a);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = NULL;
	fan.st_pattern = NULL;
	W3D_DrawTriFan(context, &fan);
}

void test_draw(int frame) {
	W3D_Double clear = 1.0;

	W3D_SetState(context, W3D_ZBUFFER, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF000000);
	W3D_SetState(context, W3D_ZBUFFER, W3D_ENABLE);
	W3D_ClearZBuffer(context, &clear);

	W3D_SetState(context, W3D_ZBUFFERUPDATE, W3D_DISABLE);
	quad(20, 40, 0.2f, 1, 0, 0, 1);
	quad(70, 90, 0.8f, 0, 0, 1, 1);

	W3D_SetState(context, W3D_ZBUFFERUPDATE, W3D_ENABLE);
	quad(170, 40, 0.5f, 1, 1, 1, 1);
	quad(200, 90, 0.4f, 0, 1, 0, 0.5f);
}

void test_cleanup(void) {
	W3D_SetState(context, W3D_ZBUFFER, W3D_DISABLE);
	W3D_FreeZBuffer(context);
}
