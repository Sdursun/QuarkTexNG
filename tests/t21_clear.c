/*
 * W3D_ClearDrawRegion clears whatever the state. A textured quad is drawn,
 * then texturing, additive blending (W3D_SRC_ALPHA, W3D_ONE), an alpha test
 * and fog are left on and the drawing region is cleared to 0x336699: it must
 * be that colour everywhere. A green quad drawn after it with that state
 * switched off shows that drawing goes on normally.
 * (QuarkTex 0.53 drew a rectangle in the current state in a window, so the
 * clear was textured, blended onto the old picture, alpha tested and
 * fogged; it also divided the colour channels by 256, so 0xFF was 254.)
 */
#include <stdio.h>
#include <proto/exec.h>
#include "common.h"

const char test_name[] = "t21_clear";

#define SIZE 8

static UBYTE image[SIZE * SIZE * 3];
static W3D_Texture *texture;

int test_setup(void) {
	ULONG error = 0;
	int i;
	struct TagItem tags[] = {
		{W3D_ATO_IMAGE, 0},
		{W3D_ATO_FORMAT, W3D_R8G8B8},
		{W3D_ATO_WIDTH, SIZE},
		{W3D_ATO_HEIGHT, SIZE},
		{TAG_DONE, 0}
	};
	for (i = 0; i < SIZE * SIZE; ++i) {
		image[i * 3] = (i & 1) ? 255 : 0;
		image[i * 3 + 1] = 128;
		image[i * 3 + 2] = (i & 2) ? 255 : 0;
	}
	tags[0].ti_Data = (ULONG) image;
	texture = W3D_AllocTexObj(context, &error, tags);
	if (!texture || error != W3D_SUCCESS) {
		fail("W3D_AllocTexObj", error);
		return 1;
	}
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

static void quad(float x0, float y0, float x1, float y1, float r, float g, float b, float a, W3D_Texture *t) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], x0, y0, 0.5f, r, g, b, a);
	set_vertex(&v[1], x1, y0, 0.5f, r, g, b, a);
	set_vertex(&v[2], x1, y1, 0.5f, r, g, b, a);
	set_vertex(&v[3], x0, y1, 0.5f, r, g, b, a);
	set_uv(&v[0], 0, 0);
	set_uv(&v[1], SIZE, 0);
	set_uv(&v[2], SIZE, SIZE);
	set_uv(&v[3], 0, SIZE);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = t;
	fan.st_pattern = NULL;
	W3D_DrawTriFan(context, &fan);
}

void test_draw(int frame) {
	W3D_Color none = {0, 0, 0, 0};
	W3D_Float reference = 0.5f;
	W3D_Fog fog;

	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF000000);
	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);
	W3D_SetTexEnv(context, texture, W3D_REPLACE, &none);
	quad(20, 20, 300, 220, 1, 1, 1, 1, texture);

	/* State that a drawn rectangle would go through. */
	W3D_SetBlendMode(context, W3D_SRC_ALPHA, W3D_ONE);
	W3D_SetState(context, W3D_BLENDING, W3D_ENABLE);
	W3D_SetAlphaMode(context, W3D_A_GREATER, &reference);
	W3D_SetState(context, W3D_ALPHATEST, W3D_ENABLE);
	fog.fog_start = 0.0f;
	fog.fog_end = 1.0f;
	fog.fog_density = 1.0f;
	fog.fog_color.r = 1.0f;
	fog.fog_color.g = 0.0f;
	fog.fog_color.b = 0.0f;
	W3D_SetFogParams(context, &fog, W3D_FOG_LINEAR);
	W3D_SetState(context, W3D_FOGGING, W3D_ENABLE);
	W3D_ClearDrawRegion(context, 0xFF336699);

	W3D_SetState(context, W3D_FOGGING, W3D_DISABLE);
	W3D_SetState(context, W3D_ALPHATEST, W3D_DISABLE);
	W3D_SetState(context, W3D_BLENDING, W3D_DISABLE);
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	quad(140, 100, 180, 140, 0, 1, 0, 1, NULL);
}

void test_cleanup(void) {
	if (texture) W3D_FreeTexObj(context, texture);
}
