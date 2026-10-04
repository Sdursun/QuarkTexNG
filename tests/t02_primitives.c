/* Points, lines, line strip/loop, triangle fan and strip side by side. */
#include "common.h"

const char test_name[] = "t02_primitives";

int test_setup(void) {
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	return 0;
}

void test_draw(int frame) {
	W3D_Point point;
	W3D_Line line;
	W3D_Lines lines;
	W3D_Triangles tris;
	W3D_Vertex v[6];
	int i;

	W3D_ClearDrawRegion(context, 0xFF000000);

	/* Row 1: points, a single line, a line strip */
	for (i = 0; i < 5; ++i) {
		set_vertex(&point.v1, 20 + i * 15, 40, 0.5f, 1, 1, 0, 1);
		point.tex = NULL;
		point.pointsize = 1 + i;
		W3D_DrawPoint(context, &point);
	}
	set_vertex(&line.v1, 120, 10, 0.5f, 1, 0, 0, 1);
	set_vertex(&line.v2, 200, 90, 0.5f, 0, 0, 1, 1);
	line.tex = NULL;
	line.linewidth = 1;
	line.st_enable = W3D_FALSE;
	W3D_DrawLine(context, &line);

	set_vertex(&v[0], 220, 90, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[1], 240, 10, 0.5f, 1, 0, 0, 1);
	set_vertex(&v[2], 270, 90, 0.5f, 0, 1, 0, 1);
	set_vertex(&v[3], 300, 10, 0.5f, 0, 0, 1, 1);
	lines.vertexcount = 4;
	lines.v = v;
	lines.tex = NULL;
	lines.linewidth = 1;
	lines.st_enable = W3D_FALSE;
	W3D_DrawLineStrip(context, &lines);

	/* Row 2: line loop, triangle fan, triangle strip */
	set_vertex(&v[0], 20, 130, 0.5f, 1, 1, 0, 1);
	set_vertex(&v[1], 90, 130, 0.5f, 0, 1, 1, 1);
	set_vertex(&v[2], 90, 220, 0.5f, 1, 0, 1, 1);
	set_vertex(&v[3], 20, 220, 0.5f, 1, 1, 1, 1);
	W3D_DrawLineLoop(context, &lines);

	set_vertex(&v[0], 150, 175, 0.5f, 1, 1, 1, 1);
	set_vertex(&v[1], 150, 120, 0.5f, 1, 0, 0, 1);
	set_vertex(&v[2], 200, 160, 0.5f, 0, 1, 0, 1);
	set_vertex(&v[3], 180, 225, 0.5f, 0, 0, 1, 1);
	set_vertex(&v[4], 115, 225, 0.5f, 1, 1, 0, 1);
	set_vertex(&v[5], 100, 160, 0.5f, 1, 0, 1, 1);
	tris.vertexcount = 6;
	tris.v = v;
	tris.tex = NULL;
	tris.st_pattern = NULL;
	W3D_DrawTriFan(context, &tris);

	for (i = 0; i < 6; ++i) set_vertex(&v[i], 220 + (i / 2) * 40, (i & 1) ? 220 : 130, 0.5f, i & 1, (i >> 1) & 1, 1 - (i & 1), 1);
	W3D_DrawTriStrip(context, &tris);
}

void test_cleanup(void) {
}
