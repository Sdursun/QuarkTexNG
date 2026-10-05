#include <stdio.h>
#include <string.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include "common.h"

struct Library *Warp3DBase;
W3D_Context *context;
static struct Window *window;
static int failed;

void set_vertex(W3D_Vertex *v, float x, float y, float z, float r, float g, float b, float a) {
	memset(v, 0, sizeof(*v));
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = 1.0f;
	v->color.r = r;
	v->color.g = g;
	v->color.b = b;
	v->color.a = a;
}

void set_uv(W3D_Vertex *v, float u, float v_) {
	v->u = u;
	v->v = v_;
}

void fail(const char *what, ULONG code) {
	printf("%s: FAIL %s (%lu)\n", test_name, what, (unsigned long) code);
	failed = 1;
}

int main(void) {
	ULONG error = 0;
	int frame;
	struct TagItem tags[] = {
		{W3D_CC_BITMAP, 0},
		{W3D_CC_YOFFSET, 0},
		{W3D_CC_DRIVERTYPE, W3D_DRIVER_BEST},
		{TAG_DONE, 0}
	};

	write_label(test_name);
	Warp3DBase = OpenLibrary("Warp3D.library", 4);
	if (!Warp3DBase) {
		fail("OpenLibrary Warp3D.library 4", 0);
		return 20;
	}
	window = open_window(test_name);
	if (!window) {
		fail("OpenWindow", 0);
		CloseLibrary(Warp3DBase);
		return 20;
	}
	/* QuarkTex attaches the context to IntuitionBase->ActiveWindow. */
	if (!wait_active(window)) fail("window did not become active", 0);

	tags[0].ti_Data = (ULONG) window->RPort->BitMap;
	context = W3D_CreateContext(&error, tags);
	if (!context || error != W3D_SUCCESS) {
		fail("W3D_CreateContext", error);
	}
	else {
		if (test_setup() == 0) {
			for (frame = 0; frame < FRAMES; ++frame) {
				W3D_LockHardware(context);
				test_draw(frame);
				W3D_UnLockHardware(context);
				W3D_Flush(context);
				/* Presents the frame: QuarkTex patches ClipBlit. */
				ClipBlit(window->RPort, 0, 0, window->RPort, 0, 0, 1, 1, 0xC0);
				Delay(5);
			}
			report_shown(test_name, window);
		}
		else fail("test_setup", 0);
		test_cleanup();
		W3D_DestroyContext(context);
	}
	CloseWindow(window);
	CloseLibrary(Warp3DBase);
	if (!failed) printf("%s: OK\n", test_name);
	return failed ? 10 : 0;
}
