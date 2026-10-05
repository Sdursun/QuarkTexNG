/*
 * Resources across contexts.
 * Palettes: the test's context has a CHUNKY texture without a palette, a
 * second one (second window) one with a green palette; both are likely to
 * get the same OpenGL texture name in their OpenGL contexts, as did the
 * textures of the contexts below. The first texture is then updated without
 * a palette: it has none, so its texels stay 0, 0, 0, 0 (as in 0.53). First
 * window: black quad on grey; second window: green quad.
 * Memory: ten W3D_CreateContext/W3D_DestroyContext cycles, each allocating
 * three textures and not freeing them, after one cycle to warm up. The bytes
 * of Amiga memory lost are logged; the square at the bottom of the first
 * window is green if less than 4 KB were lost, red otherwise.
 * (Until now the host kept the palettes of all contexts in one map by
 * texture name, never emptied for destroyed contexts, so the first texture
 * took another context's palette; and W3D_DestroyContext left the
 * context's textures allocated.)
 */
#include <stdio.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include "common.h"

const char test_name[] = "t19_leaks";

#define SIZE 4

static struct Window *second;
static W3D_Context *context2;
static W3D_Texture *texture1, *texture2;
static UBYTE indices[SIZE * SIZE];
static ULONG red[256], green[256];
static LONG lost;

static W3D_Texture *chunky(W3D_Context *c, ULONG *palette) {
	ULONG error = 0;
	W3D_Texture *t;
	struct TagItem tags[] = {
		{W3D_ATO_IMAGE, 0},
		{W3D_ATO_FORMAT, W3D_CHUNKY},
		{W3D_ATO_WIDTH, SIZE},
		{W3D_ATO_HEIGHT, SIZE},
		{W3D_ATO_PALETTE, 0},
		{TAG_DONE, 0}
	};
	tags[0].ti_Data = (ULONG) indices;
	tags[4].ti_Data = (ULONG) palette;
	t = W3D_AllocTexObj(c, &error, tags);
	if (!t || error != W3D_SUCCESS) fail("W3D_AllocTexObj", error);
	else W3D_SetFilter(c, t, W3D_NEAREST, W3D_NEAREST);
	return t;
}

static W3D_Context *create(void) {
	ULONG error = 0;
	struct TagItem tags[] = {{W3D_CC_DRIVERTYPE, W3D_DRIVER_BEST}, {TAG_DONE, 0}};
	W3D_Context *c = W3D_CreateContext(&error, tags);
	if (!c) fail("W3D_CreateContext", error);
	return c;
}

/* Contexts created and destroyed with textures left allocated. */
static void cycle(void) {
	W3D_Context *c = create();
	int i;
	if (!c) return;
	for (i = 0; i < 3; ++i) chunky(c, red);
	W3D_DestroyContext(c);
}

int test_setup(void) {
	ULONG before;
	int i;
	for (i = 0; i < SIZE * SIZE; ++i) indices[i] = (UBYTE) i;
	for (i = 0; i < 256; ++i) {
		red[i] = 0xFF000000 | ((ULONG) (128 + i % 128) << 16);
		green[i] = 0xFF000000 | ((ULONG) (128 + i % 128) << 8);
	}

	cycle();
	before = AvailMem(MEMF_ANY);
	for (i = 0; i < 10; ++i) cycle();
	lost = (LONG) (before - AvailMem(MEMF_ANY));
	printf("%s: 10 context cycles lost %s4 KB of Amiga memory\n", test_name, lost < 4096 ? "less than " : "at least ");

	second = OpenWindowTags(NULL,
		WA_Title, (ULONG) "t19 second",
		WA_InnerWidth, 160,
		WA_InnerHeight, 120,
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
	if (!wait_active(second)) fail("second window did not become active", 0);
	context2 = create();
	if (!context2) return 1;

	texture1 = chunky(context, NULL);
	texture2 = chunky(context2, green);
	if (!texture1 || !texture2) return 1;
	/* No palette: the texture keeps its own. */
	W3D_UpdateTexImage(context, texture1, indices, 0, NULL);
	return 0;
}

static void quad(W3D_Context *c, W3D_Texture *t, float x0, float y0, float x1, float y1, float r, float g, float b) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], x0, y0, 0.5f, r, g, b, 1);
	set_vertex(&v[1], x1, y0, 0.5f, r, g, b, 1);
	set_vertex(&v[2], x1, y1, 0.5f, r, g, b, 1);
	set_vertex(&v[3], x0, y1, 0.5f, r, g, b, 1);
	set_uv(&v[0], 0, 0);
	set_uv(&v[1], SIZE, 0);
	set_uv(&v[2], SIZE, SIZE);
	set_uv(&v[3], 0, SIZE);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = t;
	fan.st_pattern = NULL;
	W3D_DrawTriFan(c, &fan);
}

void test_draw(int frame) {
	W3D_Color none = {0, 0, 0, 0};
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context2, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF404040);
	W3D_ClearDrawRegion(context2, 0xFF000000);
	if (lost < 4096) quad(context, NULL, 20, 190, 60, 230, 0, 0.8f, 0);
	else quad(context, NULL, 20, 190, 60, 230, 0.8f, 0, 0);

	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);
	W3D_SetState(context2, W3D_TEXMAPPING, W3D_ENABLE);
	W3D_SetTexEnv(context, texture1, W3D_REPLACE, &none);
	W3D_SetTexEnv(context2, texture2, W3D_REPLACE, &none);
	quad(context, texture1, 20, 20, 160, 160, 1, 1, 1);
	quad(context2, texture2, 20, 10, 140, 110, 1, 1, 1);
	W3D_Flush(context2);
	ClipBlit(second->RPort, 0, 0, second->RPort, 0, 0, 1, 1, 0xC0);
}

void test_cleanup(void) {
	if (texture1) W3D_FreeTexObj(context, texture1);
	if (texture2) W3D_FreeTexObj(context2, texture2);
	if (context2) W3D_DestroyContext(context2);
	if (second) CloseWindow(second);
}
