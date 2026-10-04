/*
 * One quad per Warp3D texture format (W3D_CHUNKY .. W3D_R8G8B8A8), blended
 * over grey so that the alpha channel shows. The pattern encodes red in x,
 * green in y, blue as a checkerboard and alpha as a diagonal ramp.
 */
#include <exec/memory.h>
#include <proto/exec.h>
#include "common.h"

const char test_name[] = "t03_textures";

#define SIZE 32
#define FORMATS 11

static W3D_Texture *textures[FORMATS];
static void *images[FORMATS];
static ULONG palette[256];

static void texel(int x, int y, int *r, int *g, int *b, int *a) {
	*r = x * 255 / (SIZE - 1);
	*g = y * 255 / (SIZE - 1);
	*b = ((x / 4 + y / 4) & 1) ? 255 : 0;
	*a = (x + y) * 255 / (2 * SIZE - 2);
}

static void *make_image(ULONG format) {
	int bytes = (format == W3D_R8G8B8) ? 3 : (format == W3D_A8R8G8B8 || format == W3D_R8G8B8A8) ? 4
		: (format == W3D_A1R5G5B5 || format == W3D_R5G6B5 || format == W3D_A4R4G4B4 || format == W3D_L8A8) ? 2 : 1;
	UBYTE *image = AllocVec(SIZE * SIZE * bytes, MEMF_ANY);
	UBYTE *p = image;
	int x, y, r, g, b, a, l;
	UWORD w;
	if (!image) return NULL;
	for (y = 0; y < SIZE; ++y) {
		for (x = 0; x < SIZE; ++x) {
			texel(x, y, &r, &g, &b, &a);
			l = (r + g + b) / 3;
			switch (format) {
			case W3D_CHUNKY: *p++ = (UBYTE) ((x / 4) + (y / 4) * 8); break;
			case W3D_A1R5G5B5: w = ((a > 127) << 15) | ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3); *p++ = w >> 8; *p++ = w; break;
			case W3D_R5G6B5: w = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3); *p++ = w >> 8; *p++ = w; break;
			case W3D_R8G8B8: *p++ = r; *p++ = g; *p++ = b; break;
			case W3D_A4R4G4B4: w = ((a >> 4) << 12) | ((r >> 4) << 8) | ((g >> 4) << 4) | (b >> 4); *p++ = w >> 8; *p++ = w; break;
			case W3D_A8R8G8B8: *p++ = a; *p++ = r; *p++ = g; *p++ = b; break;
			case W3D_A8: *p++ = a; break;
			case W3D_L8: *p++ = l; break;
			case W3D_L8A8: *p++ = l; *p++ = a; break;
			case W3D_I8: *p++ = l; break;
			case W3D_R8G8B8A8: *p++ = r; *p++ = g; *p++ = b; *p++ = a; break;
			}
		}
	}
	return image;
}

int test_setup(void) {
	int i;
	ULONG error;
	struct TagItem tags[] = {
		{W3D_ATO_IMAGE, 0},
		{W3D_ATO_FORMAT, 0},
		{W3D_ATO_WIDTH, SIZE},
		{W3D_ATO_HEIGHT, SIZE},
		{W3D_ATO_PALETTE, (ULONG) palette},
		{TAG_DONE, 0}
	};
	for (i = 0; i < 256; ++i) palette[i] = 0xFF000000 | ((i * 37 & 255) << 16) | ((i * 91 & 255) << 8) | (i * 13 & 255);

	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	W3D_SetState(context, W3D_BLENDING, W3D_ENABLE);
	W3D_SetBlendMode(context, W3D_SRC_ALPHA, W3D_ONE_MINUS_SRC_ALPHA);

	for (i = 0; i < FORMATS; ++i) {
		images[i] = make_image(i + 1);
		if (!images[i]) return 1;
		tags[0].ti_Data = (ULONG) images[i];
		tags[1].ti_Data = i + 1;
		error = 0;
		textures[i] = W3D_AllocTexObj(context, &error, tags);
		if (!textures[i] || error != W3D_SUCCESS) fail("W3D_AllocTexObj", (i + 1) * 1000 + error);
		else W3D_SetFilter(context, textures[i], W3D_NEAREST, W3D_NEAREST);
	}
	return 0;
}

void test_draw(int frame) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	int i;
	float x, y;

	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF808080);
	W3D_SetState(context, W3D_TEXMAPPING, W3D_ENABLE);

	for (i = 0; i < FORMATS; ++i) {
		if (!textures[i]) continue;
		x = 10 + (i % 4) * 77;
		y = 10 + (i / 4) * 77;
		set_vertex(&v[0], x, y, 0.5f, 1, 1, 1, 1);
		set_vertex(&v[1], x + 70, y, 0.5f, 1, 1, 1, 1);
		set_vertex(&v[2], x + 70, y + 70, 0.5f, 1, 1, 1, 1);
		set_vertex(&v[3], x, y + 70, 0.5f, 1, 1, 1, 1);
		set_uv(&v[0], 0, 0);
		set_uv(&v[1], SIZE, 0);
		set_uv(&v[2], SIZE, SIZE);
		set_uv(&v[3], 0, SIZE);
		fan.vertexcount = 4;
		fan.v = v;
		fan.tex = textures[i];
		fan.st_pattern = NULL;
		W3D_DrawTriFan(context, &fan);
	}
}

void test_cleanup(void) {
	int i;
	for (i = 0; i < FORMATS; ++i) {
		if (textures[i]) W3D_FreeTexObj(context, textures[i]);
		if (images[i]) FreeVec(images[i]);
	}
}
