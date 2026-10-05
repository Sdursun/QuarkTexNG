/*
 * W3D_Query and W3D_QueryDriver: one square per query, in the order of
 * Warp3D.h, eight per row: green W3D_FULLY_SUPPORTED, yellow
 * W3D_PARTIALLY_SUPPORTED, red W3D_NOT_SUPPORTED, white for a number (the
 * maximum texture sizes), grey for anything else. A square's right half
 * shows W3D_QueryDriver, which must agree. The answers are logged.
 * (QuarkTex 0.53 answered W3D_FULLY_SUPPORTED to every query but the maximum
 * texture width and height, also for mipmapping, specular highlights,
 * stippling and antialiasing, which it does not do, and for the maximum
 * perspective texture sizes, which are numbers.)
 */
#include <stdio.h>
#include "common.h"

const char test_name[] = "t18_query";

#define W3D_QueryDriver(driver, query, destfmt) \
	LP3(408, ULONG, W3D_QueryDriver, W3D_Driver *, driver, a0, ULONG, query, d0, ULONG, destfmt, d1, , Warp3DBase)
#define W3D_GetDrivers() \
	LP0(402, W3D_Driver **, W3D_GetDrivers, , Warp3DBase)

static const ULONG queries[] = {
	W3D_Q_DRAW_POINT, W3D_Q_DRAW_LINE, W3D_Q_DRAW_TRIANGLE, W3D_Q_DRAW_POINT_X, W3D_Q_DRAW_LINE_X,
	W3D_Q_DRAW_LINE_ST, W3D_Q_DRAW_POLY_ST, W3D_Q_DRAW_POINT_FX, W3D_Q_DRAW_LINE_FX,
	W3D_Q_TEXMAPPING, W3D_Q_MIPMAPPING, W3D_Q_BILINEARFILTER, W3D_Q_MMFILTER, W3D_Q_LINEAR_REPEAT,
	W3D_Q_LINEAR_CLAMP, W3D_Q_PERSPECTIVE, W3D_Q_PERSP_REPEAT, W3D_Q_PERSP_CLAMP, W3D_Q_ENV_REPLACE,
	W3D_Q_ENV_DECAL, W3D_Q_ENV_MODULATE, W3D_Q_ENV_BLEND, W3D_Q_WRAP_ASYM, W3D_Q_SPECULAR,
	W3D_Q_BLEND_DECAL_FOG, W3D_Q_TEXMAPPING3D, W3D_Q_CHROMATEST,
	W3D_Q_FLATSHADING, W3D_Q_GOURAUDSHADING,
	W3D_Q_ZBUFFER, W3D_Q_ZBUFFERUPDATE, W3D_Q_ZCOMPAREMODES,
	W3D_Q_ALPHATEST, W3D_Q_ALPHATESTMODES,
	W3D_Q_BLENDING, W3D_Q_SRCFACTORS, W3D_Q_DESTFACTORS, W3D_Q_ONE_ONE,
	W3D_Q_FOGGING, W3D_Q_LINEAR, W3D_Q_EXPONENTIAL, W3D_Q_S_EXPONENTIAL, W3D_Q_INTERPOLATED,
	W3D_Q_ANTIALIASING, W3D_Q_ANTI_POINT, W3D_Q_ANTI_LINE, W3D_Q_ANTI_POLYGON, W3D_Q_ANTI_FULLSCREEN,
	W3D_Q_DITHERING, W3D_Q_PALETTECONV, W3D_Q_SCISSOR,
	W3D_Q_MAXTEXWIDTH, W3D_Q_MAXTEXHEIGHT, W3D_Q_MAXTEXWIDTH_P, W3D_Q_MAXTEXHEIGHT_P, W3D_Q_RECTTEXTURES,
	W3D_Q_LOGICOP, W3D_Q_MASKING,
	W3D_Q_STENCILBUFFER, W3D_Q_STENCIL_MASK, W3D_Q_STENCIL_FUNC, W3D_Q_STENCIL_SFAIL, W3D_Q_STENCIL_DPFAIL,
	W3D_Q_STENCIL_DPPASS, W3D_Q_STENCIL_WRMASK,
	W3D_Q_DRAW_POINT_TEX, W3D_Q_DRAW_LINE_TEX, W3D_Q_CULLFACE,
	200 /* not a query */
};
#define COUNT (sizeof(queries) / sizeof(queries[0]))

static ULONG answers[COUNT], driverAnswers[COUNT];

int test_setup(void) {
	W3D_Driver **drivers = W3D_GetDrivers();
	int i;
	for (i = 0; i < COUNT; ++i) {
		answers[i] = W3D_Query(context, queries[i], W3D_FMT_R8G8B8);
		driverAnswers[i] = drivers && drivers[0] ? W3D_QueryDriver(drivers[0], queries[i], W3D_FMT_R8G8B8) : 0;
	}
	for (i = 0; i < COUNT; ++i) {
		printf("%s: query %lu: %lu, driver %lu\n", test_name,
			(unsigned long) queries[i], (unsigned long) answers[i], (unsigned long) driverAnswers[i]);
	}
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

static void rectangle(float x0, float y0, float x1, float y1, ULONG answer) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	float r = 0.5f, g = 0.5f, b = 0.5f;
	if (answer == W3D_FULLY_SUPPORTED) { r = 0; g = 0.8f; b = 0; }
	else if (answer == W3D_PARTIALLY_SUPPORTED) { r = 0.9f; g = 0.9f; b = 0; }
	else if (answer == W3D_NOT_SUPPORTED) { r = 0.8f; g = 0; b = 0; }
	else if (answer >= 256) { r = 1; g = 1; b = 1; }
	set_vertex(&v[0], x0, y0, 0.5f, r, g, b, 1);
	set_vertex(&v[1], x1, y0, 0.5f, r, g, b, 1);
	set_vertex(&v[2], x1, y1, 0.5f, r, g, b, 1);
	set_vertex(&v[3], x0, y1, 0.5f, r, g, b, 1);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = NULL;
	fan.st_pattern = NULL;
	W3D_DrawTriFan(context, &fan);
}

void test_draw(int frame) {
	int i;
	W3D_ClearDrawRegion(context, 0xFF000000);
	for (i = 0; i < COUNT; ++i) {
		float x = 10 + (i % 8) * 38, y = 10 + (i / 8) * 26;
		rectangle(x, y, x + 16, y + 20, answers[i]);
		rectangle(x + 16, y, x + 32, y + 20, driverAnswers[i]);
	}
}

void test_cleanup(void) {
}
