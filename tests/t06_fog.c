/*
 * White bars at increasing depth under fog. Row 1: linear fog without the
 * z-buffer (QuarkTex 0.53 sends no depth then, so no fog shows), row 2:
 * linear fog with the z-buffer, row 3: exponential fog with the z-buffer.
 */
#include "common.h"

const char test_name[] = "t06_fog";

int test_setup(void) {
	ULONG result;
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	result = W3D_AllocZBuffer(context);
	if (result != W3D_SUCCESS) fail("W3D_AllocZBuffer", result);
	W3D_SetZCompareMode(context, W3D_Z_ALWAYS);
	return 0;
}

static void bar(float x, float y, float z) {
	W3D_Triangles fan;
	W3D_Vertex v[4];
	set_vertex(&v[0], x, y, z, 1, 1, 1, 1);
	set_vertex(&v[1], x + 25, y, z, 1, 1, 1, 1);
	set_vertex(&v[2], x + 25, y + 65, z, 1, 1, 1, 1);
	set_vertex(&v[3], x, y + 65, z, 1, 1, 1, 1);
	fan.vertexcount = 4;
	fan.v = v;
	fan.tex = NULL;
	fan.st_pattern = NULL;
	W3D_DrawTriFan(context, &fan);
}

static void row(float y) {
	int i;
	for (i = 0; i < 10; ++i) bar(10 + i * 30, y, i / 9.0f);
}

void test_draw(int frame) {
	W3D_Fog fog;
	W3D_Double clear = 1.0;
	ULONG result;

	W3D_SetState(context, W3D_FOGGING, W3D_DISABLE);
	W3D_SetState(context, W3D_ZBUFFER, W3D_DISABLE);
	W3D_ClearDrawRegion(context, 0xFF000040);
	W3D_ClearZBuffer(context, &clear);

	fog.fog_start = 0.0f;
	fog.fog_end = 1.0f;
	fog.fog_density = 2.0f;
	fog.fog_color.r = 0.2f;
	fog.fog_color.g = 0.4f;
	fog.fog_color.b = 0.8f;

	result = W3D_SetFogParams(context, &fog, W3D_FOG_LINEAR);
	if (result != W3D_SUCCESS) fail("W3D_SetFogParams linear", result);
	W3D_SetState(context, W3D_FOGGING, W3D_ENABLE);
	row(10);

	W3D_SetState(context, W3D_ZBUFFER, W3D_ENABLE);
	row(87);

	result = W3D_SetFogParams(context, &fog, W3D_FOG_EXP);
	if (result != W3D_SUCCESS) fail("W3D_SetFogParams exp", result);
	row(164);
}

void test_cleanup(void) {
	W3D_SetState(context, W3D_FOGGING, W3D_DISABLE);
	W3D_SetState(context, W3D_ZBUFFER, W3D_DISABLE);
	W3D_FreeZBuffer(context);
}
