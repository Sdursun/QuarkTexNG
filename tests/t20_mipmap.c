/*
 * Mipmap filters. A 64 x 64 texture of 1-texel red and blue stripes is drawn
 * at its size (left) and shrunk to 16 x 16 (middle) and 8 x 8 (right) with
 * W3D_LINEAR_MIP_LINEAR. Warp3D makes the mipmaps the application does not
 * supply, so the shrunk quads come out an even purple; without mipmaps the
 * texture would be missing (white quads), and without filtering the
 * stripes would alias.
 * (QuarkTex up to phase 6 made no mipmaps: OpenGL took the texture as
 * incomplete and drew the quads white.)
 */
#include <stdio.h>
#include <proto/exec.h>
#include "common.h"

const char test_name[] = "t20_mipmap";

#define SIZE 64

static UBYTE *image;
static W3D_Texture *texture;

int test_setup(void) {
	ULONG error = 0;
	int x, y;
	struct TagItem tags[] = {
		{W3D_ATO_IMAGE, 0},
		{W3D_ATO_FORMAT, W3D_R8G8B8},
		{W3D_ATO_WIDTH, SIZE},
		{W3D_ATO_HEIGHT, SIZE},
		{TAG_DONE, 0}
	};
	image = AllocVec(SIZE * SIZE * 3, MEMF_ANY);
	if (!image) return 1;
	for (y = 0; y < SIZE; ++y) {
		for (x = 0; x < SIZE; ++x) {
			UBYTE *p = image + (y * SIZE + x) * 3;
			p[0] = (x & 1) ? 0 : 255;
			p[1] = 0;
			p[2] = (x & 1) ? 255 : 0;
		}
	}
	tags[0].ti_Data = (ULONG) image;
	texture = W3D_AllocTexObj(context, &error, tags);
	if (!texture || error != W3D_SUCCESS) {
		fail("W3D_AllocTexObj", error);
		return 1;
	}
	W3D_SetFilter(context, texture, W3D_LINEAR_MIP_LINEAR, W3D_LINEAR);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

static void quad(float x, float y, float size) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], x, y, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[1], x + size, y, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[2], x + size, y + size, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[3], x, y + size, 0.5f, 1, 1, 1, 1);
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

void test_draw(int frame) {
	W3D_Color none = {0, 0, 0, 0};
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF404040);
	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);
	W3D_SetTexEnv(context, texture, W3D_REPLACE, &none);
	quad(20, 40, 64);
	quad(120, 64, 16);
	quad(180, 68, 8);
}

void test_cleanup(void) {
	if (texture) W3D_FreeTexObj(context, texture);
	if (image) FreeVec(image);
}
