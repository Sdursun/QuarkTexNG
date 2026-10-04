/*
 * Texture updates. A 32 x 32 grey texture is replaced by a red one with
 * W3D_UpdateTexImage, then two rectangles are updated with
 * W3D_UpdateTexSubImage: a blue 16 x 16 image (srcbpr 0, rows packed) at
 * (4, 4), and the top left 12 x 8 of a 32 pixel wide green image (srcbpr
 * 96) at (18, 20).
 * At the end W3D_FreeAllTexObj frees the textures, only on this build:
 * QuarkTex 0.53 frees the wrong list nodes there and can crash the system.
 * (0.53 also uploaded the texture's original image in UpdateTexSubImage
 * and ignored calls with srcbpr != 0.)
 */
#include <stdio.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include "common.h"

const char test_name[] = "t12_texupdate";

#define SIZE 32

static UBYTE *images[4];
static W3D_Texture *texture;

static UBYTE *rgb_image(int width, int height, UBYTE r, UBYTE g, UBYTE b) {
	UBYTE *image = AllocVec(width * height * 3, MEMF_ANY);
	int i;
	if (!image) return NULL;
	for (i = 0; i < width * height; ++i) {
		/* A darker diagonal line shows the orientation. */
		int dark = (i % width) == (i / width);
		image[i * 3] = dark ? r / 2 : r;
		image[i * 3 + 1] = dark ? g / 2 : g;
		image[i * 3 + 2] = dark ? b / 2 : b;
	}
	return image;
}

int test_setup(void) {
	ULONG error = 0, result;
	W3D_Scissor blue = {4, 4, 16, 16}, green = {18, 20, 12, 8};
	struct TagItem tags[] = {
		{W3D_ATO_IMAGE, 0},
		{W3D_ATO_FORMAT, W3D_R8G8B8},
		{W3D_ATO_WIDTH, SIZE},
		{W3D_ATO_HEIGHT, SIZE},
		{TAG_DONE, 0}
	};

	images[0] = rgb_image(SIZE, SIZE, 128, 128, 128);
	images[1] = rgb_image(SIZE, SIZE, 255, 0, 0);
	images[2] = rgb_image(16, 16, 0, 0, 255);
	images[3] = rgb_image(32, 16, 0, 255, 0);
	if (!images[0] || !images[1] || !images[2] || !images[3]) return 1;

	tags[0].ti_Data = (ULONG) images[0];
	texture = W3D_AllocTexObj(context, &error, tags);
	if (!texture || error != W3D_SUCCESS) {
		fail("W3D_AllocTexObj", error);
		return 1;
	}
	W3D_SetFilter(context, texture, W3D_NEAREST, W3D_NEAREST);

	result = W3D_UpdateTexImage(context, texture, images[1], 0, NULL);
	if (result != W3D_SUCCESS) fail("W3D_UpdateTexImage", result);
	result = W3D_UpdateTexSubImage(context, texture, images[2], 0, NULL, &blue, 0);
	if (result != W3D_SUCCESS) fail("W3D_UpdateTexSubImage srcbpr 0", result);
	result = W3D_UpdateTexSubImage(context, texture, images[3], 0, NULL, &green, 32 * 3);
	if (result != W3D_SUCCESS) fail("W3D_UpdateTexSubImage srcbpr 96", result);

	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

void test_draw(int frame) {
	W3D_Triangles fan;
	W3D_Vertex v[4];

	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF000000);
	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);

	set_vertex(&v[0], 60, 20, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[1], 260, 20, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[2], 260, 220, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[3], 60, 220, 0.5f, 1, 1, 1, 1);
	set_uv(&v[0], 0, 0);
	set_uv(&v[1], SIZE, 0);
	set_uv(&v[2], SIZE, SIZE);
	set_uv(&v[3], 0, SIZE);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = texture;
	fan.st_pattern = NULL;
	W3D_DrawTriFan(context, &fan);
}

void test_cleanup(void) {
	int i;
	const char *id = (const char *) Warp3DBase->lib_IdString;
	if (texture) {
		if (id && strstr(id, "QuarkTex")) {
			ULONG result = W3D_FreeAllTexObj(context);
			if (result != W3D_SUCCESS) fail("W3D_FreeAllTexObj", result);
			else printf("%s: W3D_FreeAllTexObj returned\n", test_name);
		}
		else W3D_FreeTexObj(context, texture);
	}
	for (i = 0; i < 4; ++i) if (images[i]) FreeVec(images[i]);
}
