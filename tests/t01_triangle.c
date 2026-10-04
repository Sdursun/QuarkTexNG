/* Gouraud shaded triangle on a cleared background. */
#include "common.h"

const char test_name[] = "t01_triangle";

int test_setup(void) {
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

void test_draw(int frame) {
	W3D_Triangle tri;
	W3D_ClearDrawRegion(context, 0xFF202040);
	set_vertex(&tri.v1, 160, 20, 0.5f, 1, 0, 0, 1);
	set_vertex(&tri.v2, 300, 220, 0.5f, 0, 1, 0, 1);
	set_vertex(&tri.v3, 20, 220, 0.5f, 0, 0, 1, 1);
	tri.tex = NULL;
	tri.st_pattern = NULL;
	W3D_DrawTriangle(context, &tri);
}

void test_cleanup(void) {
}
