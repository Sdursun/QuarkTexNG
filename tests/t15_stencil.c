/*
 * The stencil buffer.
 * Top left: a diamond is drawn into the stencil buffer only (colour mask
 * off, W3D_ST_ALWAYS, REPLACE with 1); a red quad with W3D_ST_EQUAL 1 and a
 * blue one with W3D_ST_NOTEQUAL 1 then show it.
 * Top right: two overlapping quads increment the stencil (W3D_ST_INCR); the
 * overlap (2) is drawn bright green, the rest (1) dark green.
 * Bottom left: values written directly - W3D_WriteStencilSpan rows (one
 * full, one dashed with a mask), W3D_WriteStencilPixel and a checkered
 * W3D_FillStencilBuffer block, all with 3 - shown by a yellow quad with
 * W3D_ST_EQUAL 3.
 * Bottom right: W3D_ReadStencilPixel/Span read back the stencil values
 * (bars, 40 pixels per unit), the last bar after drawing with a write mask
 * of 0, which must leave the stencil at 0.
 * (QuarkTex 0.53 had no stencil buffer: every stencil call returned an
 * error and did nothing.)
 */
#include <stdio.h>
#include "common.h"

const char test_name[] = "t15_stencil";

#define SPAN 120

static ULONG threes[SPAN];
static UBYTE mask[SPAN];
static UBYTE block[48 * 64];

int test_setup(void) {
	ULONG result;
	int i, x, y;
	for (i = 0; i < SPAN; ++i) {
		threes[i] = 3;
		mask[i] = (i / 8) & 1;
	}
	for (y = 0; y < 48; ++y) for (x = 0; x < 64; ++x) block[y * 64 + x] = ((x / 8 + y / 8) & 1) ? 3 : 0;
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	printf("%s: W3D_Query stencil buffer %ld, func %ld, write mask %ld\n", test_name,
		(long) W3D_Query(context, W3D_Q_STENCILBUFFER, 0), (long) W3D_Query(context, W3D_Q_STENCIL_FUNC, 0),
		(long) W3D_Query(context, W3D_Q_STENCIL_WRMASK, 0));
	result = W3D_AllocStencilBuffer(context);
	printf("%s: W3D_AllocStencilBuffer %ld\n", test_name, (long) (LONG) result);
	return 0;
}

static void fan(W3D_Vertex *v, int count) {
	W3D_Triangles triangles;
	triangles.vertexcount = count;
	triangles.v = v;
	triangles.tex = NULL;
	triangles.st_pattern = NULL;
	W3D_DrawTriFan(context, &triangles);
}

static void quad(float x0, float y0, float x1, float y1, float r, float g, float b) {
	W3D_Vertex v[4];
	set_vertex(&v[0], x0, y0, 0.5f, r, g, b, 1);
	set_vertex(&v[1], x1, y0, 0.5f, r, g, b, 1);
	set_vertex(&v[2], x1, y1, 0.5f, r, g, b, 1);
	set_vertex(&v[3], x0, y1, 0.5f, r, g, b, 1);
	fan(v, 4);
}

static void diamond(void) {
	W3D_Vertex v[4];
	set_vertex(&v[0], 80, 15, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[1], 140, 60, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[2], 80, 105, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[3], 20, 60, 0.5f, 1, 1, 1, 1);
	fan(v, 4);
}

/* A stencil value as a bar, 40 pixels per unit, at most 3.5 units. */
static void bar(int row, ULONG value) {
	float length = value <= 3 ? value * 40.0f + 4 : 140;
	quad(170, 130 + row * 14, 170 + length, 140 + row * 14, 1, 0.5f, 0);
}

void test_draw(int frame) {
	ULONG zero = 0, pixel = 99, span[4] = {99, 99, 99, 99}, masked = 99;
	int i;

	W3D_SetState(context, W3D_STENCILBUFFER, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF000000);
	W3D_ClearStencilBuffer(context, &zero);
	W3D_SetState(context, W3D_STENCILBUFFER, W3D_ENABLE);
	W3D_SetStencilOp(context, W3D_ST_KEEP, W3D_ST_KEEP, W3D_ST_KEEP);

	/* Top left: a diamond shape in the stencil buffer only. */
	W3D_SetColorMask(context, W3D_FALSE, W3D_FALSE, W3D_FALSE, W3D_FALSE);
	W3D_SetStencilFunc(context, W3D_ST_ALWAYS, 1, 0xFF);
	W3D_SetStencilOp(context, W3D_ST_KEEP, W3D_ST_KEEP, W3D_ST_REPLACE);
	diamond();
	W3D_SetColorMask(context, W3D_TRUE, W3D_TRUE, W3D_TRUE, W3D_TRUE);
	W3D_SetStencilOp(context, W3D_ST_KEEP, W3D_ST_KEEP, W3D_ST_KEEP);
	W3D_SetStencilFunc(context, W3D_ST_EQUAL, 1, 0xFF);
	quad(10, 10, 150, 110, 1, 0, 0);
	W3D_SetStencilFunc(context, W3D_ST_NOTEQUAL, 1, 0xFF);
	quad(10, 10, 150, 110, 0, 0, 0.6f);

	/* Top right: overlapping increments. */
	W3D_SetColorMask(context, W3D_FALSE, W3D_FALSE, W3D_FALSE, W3D_FALSE);
	W3D_SetStencilFunc(context, W3D_ST_ALWAYS, 0, 0xFF);
	W3D_SetStencilOp(context, W3D_ST_KEEP, W3D_ST_KEEP, W3D_ST_INCR);
	quad(170, 10, 270, 80, 1, 1, 1);
	quad(210, 40, 310, 110, 1, 1, 1);
	W3D_SetColorMask(context, W3D_TRUE, W3D_TRUE, W3D_TRUE, W3D_TRUE);
	W3D_SetStencilOp(context, W3D_ST_KEEP, W3D_ST_KEEP, W3D_ST_KEEP);
	W3D_SetStencilFunc(context, W3D_ST_EQUAL, 2, 0xFF);
	quad(170, 10, 310, 110, 0, 1, 0);
	W3D_SetStencilFunc(context, W3D_ST_EQUAL, 1, 0xFF);
	quad(170, 10, 310, 110, 0, 0.4f, 0);

	/* Bottom left: values written directly. */
	for (i = 0; i < 6; ++i) W3D_WriteStencilSpan(context, 20, 132 + i, SPAN, threes, NULL);
	for (i = 0; i < 6; ++i) W3D_WriteStencilSpan(context, 20, 144 + i, SPAN, threes, mask);
	W3D_WriteStencilPixel(context, 120, 170, 3);
	W3D_WriteStencilPixel(context, 124, 170, 3);
	W3D_FillStencilBuffer(context, 20, 160, 64, 48, 8, block);
	W3D_SetStencilFunc(context, W3D_ST_EQUAL, 3, 0xFF);
	quad(10, 125, 150, 230, 1, 1, 0);

	/* Bottom right: read back, then a write mask of 0. */
	W3D_ReadStencilPixel(context, 80, 60, &pixel);      /* diamond centre: 1 */
	W3D_ReadStencilSpan(context, 208, 60, 4, span);     /* 1, 1, 2, 2 */
	W3D_SetWriteMask(context, 0);
	W3D_SetColorMask(context, W3D_FALSE, W3D_FALSE, W3D_FALSE, W3D_FALSE);
	W3D_SetStencilFunc(context, W3D_ST_ALWAYS, 3, 0xFF);
	W3D_SetStencilOp(context, W3D_ST_REPLACE, W3D_ST_REPLACE, W3D_ST_REPLACE);
	quad(290, 220, 310, 235, 1, 1, 1);
	W3D_SetColorMask(context, W3D_TRUE, W3D_TRUE, W3D_TRUE, W3D_TRUE);
	W3D_SetWriteMask(context, 0xFF);
	W3D_SetStencilOp(context, W3D_ST_KEEP, W3D_ST_KEEP, W3D_ST_KEEP);
	W3D_ReadStencilPixel(context, 300, 228, &masked);   /* 0 */

	W3D_SetState(context, W3D_STENCILBUFFER, W3D_DISABLE);
	bar(0, pixel);
	for (i = 0; i < 4; ++i) bar(1 + i, span[i]);
	bar(5, masked);

	if (frame == FRAMES - 1) {
		printf("%s: ReadStencilPixel %lu, ReadStencilSpan %lu %lu %lu %lu, after write mask 0: %lu\n", test_name,
			pixel, span[0], span[1], span[2], span[3], masked);
	}
}

void test_cleanup(void) {
	W3D_FreeStencilBuffer(context);
}
