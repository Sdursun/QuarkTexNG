/*
 * w01_w3d_fullscreen: Warp3D in fullscreen as a game uses it, checking what
 * reaches Amiga display memory (QuartexNG phase 8). Not a reference test:
 * QuarkTex 0.53 could not create a fullscreen context. Prints PASS or FAIL
 * per check, returns 5 if one failed; run it with tests/run-app.ps1.
 *
 * A 320 x 240 16-bit RTG screen with two screen buffers and a Warp3D context
 * on it (W3D_CC_MODEID). Three frames, each into the buffer not shown: the
 * draw region set to it, cleared to red, green and blue in turn, a white
 * triangle in the middle, then ChangeScreenBuffer to show it. Afterwards
 * the last buffer holds blue with the triangle and the other green.
 */
#include <stdio.h>
#include <string.h>
#include <exec/types.h>
#include <graphics/rastport.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/Picasso96.h>
#include "warp3d_calls.h"

#define W 320
#define H 240

struct Library *Warp3DBase, *P96Base;
static int failures;

static void vertex(W3D_Vertex *v, float x, float y) {
	memset(v, 0, sizeof(*v));
	v->x = x;
	v->y = y;
	v->z = 0.5;
	v->w = 1;
	v->color.r = v->color.g = v->color.b = v->color.a = 1;
}

/* What the buffer's bitmap holds at (x, y), top down. */
static void expect(const char *what, struct BitMap *bitmap, int x, int y, int r, int g, int b) {
	struct RastPort rp;
	ULONG argb;
	int pr, pg, pb, ok;
	InitRastPort(&rp);
	rp.BitMap = bitmap;
	argb = p96ReadPixel(&rp, (UWORD) x, (UWORD) y);
	pr = (int) ((argb >> 16) & 0xFF);
	pg = (int) ((argb >> 8) & 0xFF);
	pb = (int) (argb & 0xFF);
	ok = pr >= r - 8 && pr <= r + 8 && pg >= g - 8 && pg <= g + 8 && pb >= b - 8 && pb <= b + 8;
	if (!ok) ++failures;
	printf("w01: %s %s: (%d, %d, %d), expected (%d, %d, %d)\n", ok ? "PASS" : "FAIL", what, pr, pg, pb, r, g, b);
}

int main(void) {
	static const ULONG clears[3] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF};
	struct Screen *screen = NULL;
	struct ScreenBuffer *buffers[2] = {NULL, NULL};
	W3D_Context *context = NULL;
	W3D_Scissor scissor = {0, 0, W, H};
	W3D_Triangle triangle;
	ULONG mode, error = 0;
	int frame, current = 1;

	Warp3DBase = OpenLibrary("Warp3D.library", 4);
	P96Base = OpenLibrary("Picasso96API.library", 2);
	if (!Warp3DBase || !P96Base) {
		printf("w01: FAIL Warp3D.library or Picasso96API.library missing\n");
		goto out;
	}
	mode = p96BestModeIDTags(P96BIDTAG_NominalWidth, W, P96BIDTAG_NominalHeight, H, P96BIDTAG_Depth, 16, TAG_DONE);
	screen = OpenScreenTags(NULL, SA_DisplayID, mode, SA_Width, W, SA_Height, H, SA_Depth, 16, SA_Quiet, TRUE,
		SA_ShowTitle, FALSE, SA_Type, CUSTOMSCREEN, TAG_DONE);
	if (screen) {
		buffers[0] = AllocScreenBuffer(screen, NULL, SB_SCREEN_BITMAP);
		buffers[1] = AllocScreenBuffer(screen, NULL, 0);
	}
	if (!screen || !buffers[0] || !buffers[1]) {
		printf("w01: FAIL no double-buffered screen (mode 0x%lx)\n", (unsigned long) mode);
		++failures;
		goto out;
	}
	{
		struct TagItem tags[] = {
			{W3D_CC_MODEID, 0},
			{W3D_CC_BITMAP, 0},
			{W3D_CC_YOFFSET, 0},
			{W3D_CC_DRIVERTYPE, W3D_DRIVER_BEST},
			{TAG_DONE, 0}
		};
		tags[0].ti_Data = mode;
		tags[1].ti_Data = (ULONG) buffers[current]->sb_BitMap;
		context = W3D_CreateContext(&error, tags);
	}
	if (!context || error != W3D_SUCCESS) {
		printf("w01: FAIL W3D_CreateContext (%lu)\n", (unsigned long) error);
		++failures;
		goto out;
	}
	printf("w01: context %dx%d, bprow %d, depth %d, format 0x%lx\n", context->width, context->height, context->bprow,
		context->depth, (unsigned long) context->format);
	if (context->width != W || context->height != H || !context->bprow) {
		printf("w01: FAIL W3D_Context does not describe the draw region\n");
		++failures;
	}

	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	for (frame = 0; frame < 3; ++frame) {
		W3D_SetDrawRegion(context, buffers[current]->sb_BitMap, 0, &scissor);
		W3D_LockHardware(context);
		W3D_ClearDrawRegion(context, clears[frame]);
		memset(&triangle, 0, sizeof(triangle));
		vertex(&triangle.v1, 160, 80);
		vertex(&triangle.v2, 220, 180);
		vertex(&triangle.v3, 100, 180);
		W3D_DrawTriangle(context, &triangle);
		W3D_UnLockHardware(context);
		while (!ChangeScreenBuffer(screen, buffers[current])) WaitTOF();
		current ^= 1;
		Delay(10);
	}

	/* current is the buffer drawn second to last (frame 1, green). */
	expect("last frame, corner", buffers[current ^ 1]->sb_BitMap, 10, 10, 0, 0, 255);
	expect("last frame, triangle", buffers[current ^ 1]->sb_BitMap, 160, 150, 255, 255, 255);
	expect("frame before, corner", buffers[current]->sb_BitMap, 10, 10, 0, 255, 0);

out:
	if (context) W3D_DestroyContext(context);
	if (screen) {
		/* The screen's own bitmap must be the one shown when it closes. */
		if (buffers[0]) while (!ChangeScreenBuffer(screen, buffers[0])) WaitTOF();
		WaitTOF();
		if (buffers[1]) FreeScreenBuffer(screen, buffers[1]);
		if (buffers[0]) FreeScreenBuffer(screen, buffers[0]);
		CloseScreen(screen);
	}
	if (P96Base) CloseLibrary(P96Base);
	if (Warp3DBase) CloseLibrary(Warp3DBase);
	printf("w01: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 5 : 0;
}
