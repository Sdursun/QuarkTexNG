/*
 * Throughput: TRIANGLES gouraud shaded triangles per frame, drawn one
 * W3D_DrawTriangle call at a time like a game would. The time from the first
 * triangle to the end of W3D_Flush (which waits for the host) is measured
 * with the E clock and logged as triangles per second. The scene is the same
 * every frame, so the frames can still be compared.
 *
 * For measurements, build it by hand with -DTRIANGLES=400000, and once more
 * with -DNO_DRAW, which runs the loop without W3D_DrawTriangle: the
 * difference between the two is the time spent in Warp3D.
 */
#include <stdio.h>
#include <devices/timer.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include "common.h"

/* A separate name keeps the loop-only frames from replacing the real ones. */
#ifdef NO_DRAW
const char test_name[] = "t09_loop_only";
#else
const char test_name[] = "t09_throughput";
#endif

#ifndef TRIANGLES
#define TRIANGLES 20000
#endif

struct Device *TimerBase;
static struct timerequest timer;
static int timerOpen;
static double total;
static int frames;

int test_setup(void) {
	W3D_SetState(context, W3D_TEXMAPPING, W3D_DISABLE);
	W3D_SetState(context, W3D_GOURAUD, W3D_ENABLE);
	if (OpenDevice(TIMERNAME, UNIT_ECLOCK, (struct IORequest *) &timer, 0) != 0) return 1;
	timerOpen = 1;
	TimerBase = timer.tr_node.io_Device;
	return 0;
}

static double seconds(void) {
	struct EClockVal clock;
	ULONG frequency = ReadEClock(&clock);
	return ((double) clock.ev_hi * 4294967296.0 + clock.ev_lo) / frequency;
}

void test_draw(int frame) {
	W3D_Triangle tri;
	double start, elapsed;
	int i;
	float x, y, shade;

	W3D_ClearDrawRegion(context, 0xFF000000);
	start = seconds();
	for (i = 0; i < TRIANGLES; ++i) {
		/* A 100 x 200 grid of small triangles covering the window */
		x = (i % 100) * 3.2f;
		y = (i / 100) * 1.2f;
		shade = (i % 256) / 255.0f;
		set_vertex(&tri.v1, x, y, 0.5f, shade, 0, 1 - shade, 1);
		set_vertex(&tri.v2, x + 3.2f, y, 0.5f, 0, shade, 0, 1);
		set_vertex(&tri.v3, x, y + 1.2f, 0.5f, 1 - shade, 1, shade, 1);
		tri.tex = NULL;
		tri.st_pattern = NULL;
#ifndef NO_DRAW
		W3D_DrawTriangle(context, &tri);
#endif
	}
	W3D_Flush(context);
	elapsed = seconds() - start;
	total += elapsed;
	++frames;
}

void test_cleanup(void) {
	if (frames) {
		printf("%s: %d triangles x %d frames in %.3f s = %.0f triangles/s\n",
			test_name, TRIANGLES, frames, total, TRIANGLES * frames / total);
	}
	if (timerOpen) CloseDevice((struct IORequest *) &timer);
}
