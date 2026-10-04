/* Two triangles that cross in depth; the z-buffer has to split them. */
#include "common.h"

const char test_name[] = "t04_zbuffer";

int test_setup(void) {
	ULONG result;
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	result = W3D_AllocZBuffer(context);
	if (result != W3D_SUCCESS) fail("W3D_AllocZBuffer", result);
	W3D_SetState(context, W3D_ZBUFFER, W3D_ENABLE);
	W3D_SetState(context, W3D_ZBUFFERUPDATE, W3D_ENABLE);
	W3D_SetZCompareMode(context, W3D_Z_LESS);
	return 0;
}

void test_draw(int frame) {
	W3D_Triangle tri;
	W3D_Double clear = 1.0;

	/* Clear the colour without depth testing, then the z-buffer. */
	W3D_SetState(context, W3D_ZBUFFER, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF000000);
	W3D_SetState(context, W3D_ZBUFFER, W3D_ENABLE);
	W3D_ClearZBuffer(context, &clear);

	/* Red: near on the left, far on the right. */
	set_vertex(&tri.v1, 20, 40, 0.1f, 1, 0, 0, 1);
	set_vertex(&tri.v2, 300, 120, 0.9f, 1, 0, 0, 1);
	set_vertex(&tri.v3, 20, 200, 0.1f, 1, 0, 0, 1);
	tri.tex = NULL;
	tri.st_pattern = NULL;
	W3D_DrawTriangle(context, &tri);

	/* Blue: far on the left, near on the right. */
	set_vertex(&tri.v1, 300, 40, 0.1f, 0, 0, 1, 1);
	set_vertex(&tri.v2, 20, 120, 0.9f, 0, 0, 1, 1);
	set_vertex(&tri.v3, 300, 200, 0.1f, 0, 0, 1, 1);
	W3D_DrawTriangle(context, &tri);
}

void test_cleanup(void) {
	W3D_SetState(context, W3D_ZBUFFER, W3D_DISABLE);
	W3D_FreeZBuffer(context);
}
