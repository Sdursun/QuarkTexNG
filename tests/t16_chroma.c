/*
 * The chroma test. An 8 x 8 texture of magenta, red and green diagonals,
 * nearest filtering, on a blue background:
 * top left W3D_CHROMATEST_EXCLUSIVE with magenta as both bounds (magenta
 * texels are rejected), top middle W3D_CHROMATEST_INCLUSIVE with bounds
 * around the red (only red texels pass), top right W3D_CHROMATEST_NONE
 * (all pass); bottom left the first texture with W3D_CHROMATEST switched
 * off (all pass).
 * (QuarkTex 0.53 did not support the chroma test: W3D_SetState returned
 * W3D_UNSUPPORTEDSTATE and every texel was drawn.)
 */
#include <stdio.h>
#include <proto/exec.h>
#include "common.h"

const char test_name[] = "t16_chroma";

#define SIZE 8

static UBYTE *image;
static W3D_Texture *textures[3];

int test_setup(void) {
	ULONG error, results[3];
	int x, y, i;
	static const UBYTE colors[3][3] = {{255, 0, 255}, {224, 16, 16}, {16, 224, 16}};
	struct TagItem tags[] = {
		{W3D_ATO_IMAGE, 0},
		{W3D_ATO_FORMAT, W3D_R8G8B8},
		{W3D_ATO_WIDTH, SIZE},
		{W3D_ATO_HEIGHT, SIZE},
		{TAG_DONE, 0}
	};

	image = AllocVec(SIZE * SIZE * 3, MEMF_ANY);
	if (!image) return 1;
	for (y = 0; y < SIZE; ++y)
		for (x = 0; x < SIZE; ++x)
			for (i = 0; i < 3; ++i) image[(y * SIZE + x) * 3 + i] = colors[(x + y) % 3][i];
	tags[0].ti_Data = (ULONG) image;
	for (i = 0; i < 3; ++i) {
		error = 0;
		textures[i] = W3D_AllocTexObj(context, &error, tags);
		if (!textures[i] || error != W3D_SUCCESS) {
			fail("W3D_AllocTexObj", error);
			return 1;
		}
		W3D_SetFilter(context, textures[i], W3D_NEAREST, W3D_NEAREST);
	}
	results[0] = W3D_SetChromaTestBounds(context, textures[0], 0xFFFF00FF, 0xFFFF00FF, W3D_CHROMATEST_EXCLUSIVE);
	results[1] = W3D_SetChromaTestBounds(context, textures[1], 0x00C00000, 0x00FF4040, W3D_CHROMATEST_INCLUSIVE);
	results[2] = W3D_SetChromaTestBounds(context, textures[2], 0x00000000, 0x00FFFFFF, W3D_CHROMATEST_NONE);
	printf("%s: W3D_Query %ld, W3D_SetChromaTestBounds %ld %ld %ld\n", test_name,
		(long) W3D_Query(context, W3D_Q_CHROMATEST, 0), (long) (LONG) results[0], (long) (LONG) results[1], (long) (LONG) results[2]);

	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

static void quad(W3D_Texture *texture, float x, float y, float r, float g, float b) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], x, y, 0.5f, r, g, b, 1);
	set_vertex(&v[1], x + 90, y, 0.5f, r, g, b, 1);
	set_vertex(&v[2], x + 90, y + 90, 0.5f, r, g, b, 1);
	set_vertex(&v[3], x, y + 90, 0.5f, r, g, b, 1);
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
	ULONG result;
	int i;

	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF000000);
	for (i = 0; i < 3; ++i) quad(NULL, 10 + i * 105, 20, 0, 0, 0.6f);
	quad(NULL, 10, 130, 0, 0, 0.6f);
	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);

	result = W3D_SetState(context, W3D_CHROMATEST, W3D_ENABLE);
	for (i = 0; i < 3; ++i) {
		W3D_SetTexEnv(context, textures[i], W3D_REPLACE, &none);
		quad(textures[i], 10 + i * 105, 20, 1, 1, 1);
	}
	W3D_SetState(context, W3D_CHROMATEST, W3D_DISABLE);
	W3D_SetTexEnv(context, textures[0], W3D_REPLACE, &none);
	quad(textures[0], 10, 130, 1, 1, 1);

	if (frame == FRAMES - 1) printf("%s: W3D_SetState(W3D_CHROMATEST) %ld\n", test_name, (long) (LONG) result);
}

void test_cleanup(void) {
	int i;
	for (i = 0; i < 3; ++i) if (textures[i]) W3D_FreeTexObj(context, textures[i]);
	if (image) FreeVec(image);
}
