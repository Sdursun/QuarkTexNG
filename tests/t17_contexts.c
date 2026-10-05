/*
 * Two Warp3D contexts at once: the test's window (context 1) and a second,
 * smaller window (context 2), drawn in turns, triangle by triangle.
 * Context 1 draws red triangles with alpha 0.5 and blending off, so they
 * stay pure red; context 2 has blending on and draws green ones with alpha
 * 0.5 over blue. Each window must show only its own triangles, in its own
 * state. The host captures the second context as t17_contexts-2.
 * (QuarkTex 0.53 had one host window and OpenGL context: the second
 * W3D_CreateContext replaced the first, so both drew into the second
 * window.)
 */
#include <stdio.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include "common.h"

const char test_name[] = "t17_contexts";

#define SECOND_WIDTH 240
#define SECOND_HEIGHT 180

static struct Window *second;
static W3D_Context *context2;

int test_setup(void) {
	ULONG error = 0;
	struct TagItem tags[] = {
		{W3D_CC_BITMAP, 0},
		{W3D_CC_YOFFSET, 0},
		{W3D_CC_DRIVERTYPE, W3D_DRIVER_BEST},
		{TAG_DONE, 0}
	};
	second = OpenWindowTags(NULL,
		WA_Title, (ULONG) "t17 second",
		WA_InnerWidth, SECOND_WIDTH,
		WA_InnerHeight, SECOND_HEIGHT,
		WA_Left, 380,
		WA_Top, 40,
		WA_DragBar, TRUE,
		WA_RMBTrap, TRUE,
		WA_Activate, TRUE,
		TAG_DONE);
	if (!second) {
		fail("OpenWindow second", 0);
		return 1;
	}
	/* QuarkTex attaches a context to IntuitionBase->ActiveWindow. */
	if (!wait_active(second)) fail("second window did not become active", 0);
	tags[0].ti_Data = (ULONG) second->RPort->BitMap;
	context2 = W3D_CreateContext(&error, tags);
	printf("%s: second W3D_CreateContext %s, error %ld\n", test_name, context2 ? "ok" : "NULL", (long) (LONG) error);
	if (!context2) return 1;

	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	W3D_SetState(context2, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context2, W3D_GOURAUD, W3D_ENABLE);
	W3D_SetBlendMode(context2, W3D_SRC_ALPHA, W3D_ONE_MINUS_SRC_ALPHA);
	W3D_SetState(context2, W3D_BLENDING, W3D_ENABLE);
	return 0;
}

static void triangle(W3D_Context *c, float x, float y, float size, float r, float g, float b) {
	W3D_Triangle t;
	set_vertex(&t.v1, x, y + size, 0.5f, r, g, b, 0.5f);
	set_vertex(&t.v2, x + size / 2, y, 0.5f, r, g, b, 0.5f);
	set_vertex(&t.v3, x + size, y + size, 0.5f, r, g, b, 0.5f);
	t.tex = NULL;
	t.st_pattern = NULL;
	W3D_DrawTriangle(c, &t);
}

void test_draw(int frame) {
	int i;
	W3D_ClearDrawRegion(context, 0xFF000000);
	W3D_ClearDrawRegion(context2, 0xFF0000C0);
	for (i = 0; i < 4; ++i) {
		triangle(context, 20 + i * 70, 40 + i * 30, 60, 1, 0, 0);
		triangle(context2, 10 + i * 55, 20 + i * 35, 50, 0, 1, 0);
	}
	W3D_Flush(context2);
	/* Presents the second context; main() presents the first. */
	ClipBlit(second->RPort, 0, 0, second->RPort, 0, 0, 1, 1, 0xC0);
}

void test_cleanup(void) {
	if (context2) W3D_DestroyContext(context2);
	if (second) CloseWindow(second);
}
