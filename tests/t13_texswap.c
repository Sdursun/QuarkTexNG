/*
 * Byte order of texture updates. An R5G6B5 texture is allocated, then an
 * R8G8B8 one, and then the R5G6B5 texture is updated with
 * W3D_UpdateTexImage: left a red/blue, right a green/white checkerboard.
 * (QuarkTex 0.53 set the OpenGL byte swapping only when a texture was
 * allocated, so the update of the 16-bit texture came out with its bytes
 * swapped after the 8-bit allocation.)
 */
#include <exec/memory.h>
#include <proto/exec.h>
#include "common.h"

const char test_name[] = "t13_texswap";

#define SIZE 16

static UWORD *image16[2];
static UBYTE *image24;
static W3D_Texture *textures[2];

static UWORD rgb565(int r, int g, int b) {
	return (UWORD) (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

static UWORD *checker(UWORD a, UWORD b) {
	UWORD *image = AllocVec(SIZE * SIZE * 2, MEMF_ANY);
	int x, y;
	if (!image) return NULL;
	for (y = 0; y < SIZE; ++y)
		for (x = 0; x < SIZE; ++x) image[y * SIZE + x] = ((x / 4 + y / 4) & 1) ? a : b;
	return image;
}

int test_setup(void) {
	ULONG error = 0, result;
	struct TagItem tags[] = {
		{W3D_ATO_IMAGE, 0},
		{W3D_ATO_FORMAT, W3D_R5G6B5},
		{W3D_ATO_WIDTH, SIZE},
		{W3D_ATO_HEIGHT, SIZE},
		{TAG_DONE, 0}
	};

	image16[0] = checker(rgb565(255, 0, 0), rgb565(0, 0, 255));
	image16[1] = checker(rgb565(0, 255, 0), rgb565(255, 255, 255));
	image24 = AllocVec(SIZE * SIZE * 3, MEMF_ANY | MEMF_CLEAR);
	if (!image16[0] || !image16[1] || !image24) return 1;

	/* Allocated with a grey image first, updated after the 8-bit allocation. */
	tags[0].ti_Data = (ULONG) image24;
	textures[0] = W3D_AllocTexObj(context, &error, tags);
	tags[1].ti_Data = W3D_R8G8B8;
	textures[1] = W3D_AllocTexObj(context, &error, tags);
	if (!textures[0] || !textures[1]) {
		fail("W3D_AllocTexObj", error);
		return 1;
	}
	W3D_SetFilter(context, textures[0], W3D_NEAREST, W3D_NEAREST);

	result = W3D_UpdateTexImage(context, textures[0], image16[0], 0, NULL);
	if (result != W3D_SUCCESS) fail("W3D_UpdateTexImage", result);

	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

static void quad(float x) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], x, 50, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[1], x + 140, 50, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[2], x + 140, 190, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[3], x, 190, 0.5f, 1, 1, 1, 1);
	set_uv(&v[0], 0, 0);
	set_uv(&v[1], SIZE, 0);
	set_uv(&v[2], SIZE, SIZE);
	set_uv(&v[3], 0, SIZE);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = textures[0];
	fan.st_pattern = NULL;
	W3D_DrawTriFan(context, &fan);
}

void test_draw(int frame) {
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF000000);
	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);
	/* The same texture twice, updated in between with the second image. */
	W3D_UpdateTexImage(context, textures[0], image16[0], 0, NULL);
	quad(10);
	W3D_UpdateTexImage(context, textures[0], image16[1], 0, NULL);
	quad(170);
}

void test_cleanup(void) {
	int i;
	for (i = 0; i < 2; ++i) {
		if (textures[i]) W3D_FreeTexObj(context, textures[i]);
		if (image16[i]) FreeVec(image16[i]);
	}
	if (image24) FreeVec(image24);
}
