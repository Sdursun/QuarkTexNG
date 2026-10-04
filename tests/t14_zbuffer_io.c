/*
 * Reading and writing the depth buffer directly.
 * A grey quad at depth 0.25 is drawn on the left. On the right, depth 0 is
 * written as a full row (W3D_WriteZSpan), a dashed row (with a mask) and a
 * single pixel (W3D_WriteZPixel); a blue quad at depth 0.5 drawn over them
 * leaves those pixels black. Then W3D_ReadZPixel and W3D_ReadZSpan read the
 * depth at and around the grey quad: the values are logged and drawn as bars
 * (length = depth) at the bottom.
 * (QuarkTex 0.53 read 4-byte floats into 8-byte doubles, did not turn y
 * upside down for OpenGL, wrote the address of its pointer as the depth,
 * passed GL_DOUBLE, which glDrawPixels does not take, and wrote masked
 * pixels at x + n.)
 */
#include <stdio.h>
#include "common.h"

const char test_name[] = "t14_zbuffer_io";

#define SPAN 120

static W3D_Double zeros[SPAN];
static UBYTE mask[SPAN];

int test_setup(void) {
	ULONG result;
	int i;
	for (i = 0; i < SPAN; ++i) {
		zeros[i] = 0.0;
		mask[i] = (i / 8) & 1;
	}
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	result = W3D_AllocZBuffer(context);
	if (result != W3D_SUCCESS) fail("W3D_AllocZBuffer", result);
	W3D_SetZCompareMode(context, W3D_Z_LESS);
	return 0;
}

static void quad(float x0, float y0, float x1, float y1, float z, float r, float g, float b) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], x0, y0, z, r, g, b, 1);
	set_vertex(&v[1], x1, y0, z, r, g, b, 1);
	set_vertex(&v[2], x1, y1, z, r, g, b, 1);
	set_vertex(&v[3], x0, y1, z, r, g, b, 1);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = NULL;
	fan.st_pattern = NULL;
	W3D_DrawTriFan(context, &fan);
}

/* A depth read as a bar; values outside 0..1 (or NaN) draw no bar. */
static void bar(int row, W3D_Double value) {
	float length = (value >= 0.0 && value <= 1.0) ? (float) value * 280 : 0;
	if (length > 0) quad(20, 130 + row * 16, 20 + length, 142 + row * 16, 0.5f, 1, 1, 0);
}

void test_draw(int frame) {
	W3D_Double clear = 1.0, depth[2], span[4];
	int i;

	W3D_SetState(context, W3D_ZBUFFER, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF000000);
	W3D_SetState(context, W3D_ZBUFFER, W3D_ENABLE);
	W3D_ClearZBuffer(context, &clear);

	quad(20, 20, 140, 100, 0.25f, 0.6f, 0.6f, 0.6f);

	W3D_WriteZSpan(context, 160, 50, SPAN, zeros, NULL);
	W3D_WriteZSpan(context, 160, 70, SPAN, zeros, mask);
	W3D_WriteZPixel(context, 220, 90, zeros);
	quad(150, 30, 290, 110, 0.5f, 0, 0, 1);

	for (i = 0; i < 2; ++i) depth[i] = -1;
	for (i = 0; i < 4; ++i) span[i] = -1;
	W3D_ReadZPixel(context, 80, 60, &depth[0]);  /* inside the grey quad */
	W3D_ReadZPixel(context, 80, 110, &depth[1]); /* below it */
	W3D_ReadZSpan(context, 18, 60, 4, span);     /* across its left edge */

	W3D_SetState(context, W3D_ZBUFFER, W3D_DISABLE);
	bar(0, depth[0]);
	bar(1, depth[1]);
	for (i = 0; i < 4; ++i) bar(2 + i, span[i]);

	if (frame == FRAMES - 1) {
		printf("%s: ReadZPixel %ld %ld, ReadZSpan %ld %ld %ld %ld (x1000)\n", test_name,
			(long) (depth[0] * 1000 + 0.5), (long) (depth[1] * 1000 + 0.5),
			(long) (span[0] * 1000 + 0.5), (long) (span[1] * 1000 + 0.5),
			(long) (span[2] * 1000 + 0.5), (long) (span[3] * 1000 + 0.5));
	}
}

void test_cleanup(void) {
	W3D_FreeZBuffer(context);
}
