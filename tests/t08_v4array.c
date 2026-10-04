/* Warp3D V4 vertex arrays: separate position and colour arrays. */
#include "common.h"

const char test_name[] = "t08_v4array";

static float positions[8][3];
static float colors[8][4];

int test_setup(void) {
	int i;
	for (i = 0; i < 8; ++i) {
		/* A strip of four quads across the window */
		positions[i][0] = 20 + (i / 2) * 93;
		positions[i][1] = (i & 1) ? 200 : 40;
		positions[i][2] = 0.5f;
		colors[i][0] = (i & 1) ? 1.0f : 0.0f;
		colors[i][1] = i / 7.0f;
		colors[i][2] = (i & 1) ? 0.0f : 1.0f;
		colors[i][3] = 1.0f;
	}
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	W3D_VertexPointer(context, positions, sizeof(positions[0]), W3D_VERTEX_F_F_F, 0);
	W3D_ColorPointer(context, colors, sizeof(colors[0]), W3D_COLOR_FLOAT, W3D_CMODE_RGBA, 0);
	return 0;
}

void test_draw(int frame) {
	ULONG result;
	W3D_ClearDrawRegion(context, 0xFF000000);
	result = W3D_DrawArray(context, W3D_PRIMITIVE_TRISTRIP, 0, 8);
	if (result != W3D_SUCCESS) fail("W3D_DrawArray", result);
}

void test_cleanup(void) {
}
