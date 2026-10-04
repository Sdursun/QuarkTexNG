/* Alpha ramp drawn with the alpha test at three reference values. */
#include "common.h"

const char test_name[] = "t07_alphatest";

int test_setup(void) {
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

static void ramp(float y) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], 10, y, 0.5f, 1, 0.8f, 0, 0);
	set_vertex(&v[1], 310, y, 0.5f, 0, 0.8f, 1, 1);
	set_vertex(&v[2], 310, y + 60, 0.5f, 0, 0.8f, 1, 1);
	set_vertex(&v[3], 10, y + 60, 0.5f, 1, 0.8f, 0, 0);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = NULL;
	fan.st_pattern = NULL;
	W3D_DrawTriFan(context, &fan);
}

void test_draw(int frame) {
	W3D_Float ref;
	W3D_SetState(context, W3D_ALPHATEST, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF000000);

	W3D_SetState(context, W3D_ALPHATEST, W3D_ENABLE);
	ref = 0.25f;
	W3D_SetAlphaMode(context, W3D_A_GREATER, &ref);
	ramp(10);
	ref = 0.5f;
	W3D_SetAlphaMode(context, W3D_A_GREATER, &ref);
	ramp(90);
	ref = 0.75f;
	W3D_SetAlphaMode(context, W3D_A_GREATER, &ref);
	ramp(170);
}

void test_cleanup(void) {
	W3D_SetState(context, W3D_ALPHATEST, W3D_DISABLE);
}
