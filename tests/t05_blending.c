/* Overlapping translucent quads with two blend modes. */
#include "common.h"

const char test_name[] = "t05_blending";

int test_setup(void) {
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

static void quad(float x, float y, float w, float h, float r, float g, float b, float a) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], x, y, 0.5f, r, g, b, a);
	set_vertex(&v[1], x + w, y, 0.5f, r, g, b, a);
	set_vertex(&v[2], x + w, y + h, 0.5f, r, g, b, a);
	set_vertex(&v[3], x, y + h, 0.5f, r, g, b, a);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = NULL;
	fan.st_pattern = NULL;
	W3D_DrawTriFan(context, &fan);
}

void test_draw(int frame) {
	W3D_SetState(context, W3D_BLENDING, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF303030);

	W3D_SetState(context, W3D_BLENDING, W3D_ENABLE);

	/* Left: classic alpha blending */
	W3D_SetBlendMode(context, W3D_SRC_ALPHA, W3D_ONE_MINUS_SRC_ALPHA);
	quad(10, 20, 90, 90, 1, 0, 0, 0.5f);
	quad(50, 60, 90, 90, 0, 1, 0, 0.5f);
	quad(30, 110, 90, 90, 0, 0, 1, 0.5f);

	/* Right: additive */
	W3D_SetBlendMode(context, W3D_ONE, W3D_ONE);
	quad(170, 20, 90, 90, 0.8f, 0, 0, 1);
	quad(210, 60, 90, 90, 0, 0.8f, 0, 1);
	quad(190, 110, 90, 90, 0, 0, 0.8f, 1);
}

void test_cleanup(void) {
	W3D_SetState(context, W3D_BLENDING, W3D_DISABLE);
}
