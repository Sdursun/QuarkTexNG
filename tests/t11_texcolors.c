/*
 * Colours passed with textures, both green, so that swapped green and blue
 * channels show as blue.
 * Left: W3D_SetTexEnv(W3D_BLEND) with a green environment colour on a white
 * and black checkerboard: white texels take the environment colour.
 * Right: W3D_SetWrapMode(W3D_CLAMP) with a green border colour; texture
 * coordinates run from -1 to 2, so with linear filtering the clamped edges
 * are half border colour.
 * (QuarkTex 0.53 passed both colours as r, b, g, a.)
 */
#include <exec/memory.h>
#include <proto/exec.h>
#include "common.h"

const char test_name[] = "t11_texcolors";

#define SIZE 16

static UBYTE *image;
static W3D_Texture *textures[2];

int test_setup(void) {
	ULONG error;
	int x, y, i;
	struct TagItem tags[] = {
		{W3D_ATO_IMAGE, 0},
		{W3D_ATO_FORMAT, W3D_R8G8B8},
		{W3D_ATO_WIDTH, SIZE},
		{W3D_ATO_HEIGHT, SIZE},
		{TAG_DONE, 0}
	};
	W3D_Color green = {0.0f, 1.0f, 0.0f, 1.0f};

	image = AllocVec(SIZE * SIZE * 3, MEMF_ANY);
	if (!image) return 1;
	for (y = 0; y < SIZE; ++y)
		for (x = 0; x < SIZE; ++x)
			for (i = 0; i < 3; ++i) image[(y * SIZE + x) * 3 + i] = ((x / 4 + y / 4) & 1) ? 255 : 0;
	tags[0].ti_Data = (ULONG) image;
	for (i = 0; i < 2; ++i) {
		error = 0;
		textures[i] = W3D_AllocTexObj(context, &error, tags);
		if (!textures[i] || error != W3D_SUCCESS) {
			fail("W3D_AllocTexObj", error);
			return 1;
		}
		W3D_SetFilter(context, textures[i], W3D_LINEAR, W3D_LINEAR);
	}
	W3D_SetTexEnv(context, textures[0], W3D_BLEND, &green);
	W3D_SetWrapMode(context, textures[1], W3D_CLAMP, W3D_CLAMP, &green);

	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

static void quad(W3D_Texture *texture, float x, float uv0, float uv1) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], x, 40, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[1], x + 140, 40, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[2], x + 140, 180, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[3], x, 180, 0.5f, 1, 1, 1, 1);
	set_uv(&v[0], uv0, uv0);
	set_uv(&v[1], uv1, uv0);
	set_uv(&v[2], uv1, uv1);
	set_uv(&v[3], uv0, uv1);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = texture;
	fan.st_pattern = NULL;
	W3D_DrawTriFan(context, &fan);
}

void test_draw(int frame) {
	W3D_Color none = {0, 0, 0, 0};
	W3D_Color green = {0.0f, 1.0f, 0.0f, 1.0f};
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF404040);
	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);

	/* OpenGL keeps one texture environment, so set it before each quad. */
	W3D_SetTexEnv(context, textures[0], W3D_BLEND, &green);
	quad(textures[0], 10, 0, SIZE);
	W3D_SetTexEnv(context, textures[1], W3D_REPLACE, &none);
	quad(textures[1], 170, -SIZE, 2 * SIZE);
}

void test_cleanup(void) {
	int i;
	for (i = 0; i < 2; ++i) if (textures[i]) W3D_FreeTexObj(context, textures[i]);
	if (image) FreeVec(image);
}
