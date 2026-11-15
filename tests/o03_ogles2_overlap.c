/*
 * o03_ogles2_overlap: a window in front of an OpenGL ES window keeps what it
 * shows. The host writes each frame only into the visible parts of the
 * window's layer (gl/gl.c's QtClipTarget); before, it wrote over the whole
 * rectangle, also over the windows in front.
 */
#include <proto/ogles2.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/Picasso96.h>
#include <stdio.h>
#include "window.h"

struct Library *P96Base;
static int failures;

static void expect(const char *what, struct RastPort *rp, int x, int y, ULONG rgb) {
	ULONG argb = p96ReadPixel(rp, (UWORD) x, (UWORD) y) & 0xFFFFFF;
	int ok = argb == rgb;
	if (!ok) ++failures;
	printf("o03: %s %s: %06lx, expected %06lx\n", ok ? "PASS" : "FAIL", what, (unsigned long) argb, (unsigned long) rgb);
}

int main(void) {
	struct Window *win, *front;
	void *ctx;
	ULONG err = 0;
	int i;

	write_label("o03_ogles2_overlap");
	P96Base = OpenLibrary((STRPTR) "Picasso96API.library", 2);
	if (!P96Base || !OGLES2Open()) {
		printf("o03: FAIL no Picasso96API.library or ogles2.library\n");
		return 20;
	}
	if (!(win = open_window("o03: ogles2 overlap"))) {
		printf("o03: FAIL no window\n");
		return 20;
	}
	ctx = aglCreateContextTags2(&err, OGLES2_CCT_WINDOW, (ULONG) win, TAG_DONE);
	if (!ctx) {
		printf("o03: FAIL aglCreateContextTags2, error %lu\n", (unsigned long) err);
		CloseWindow(win);
		return 20;
	}
	aglMakeCurrent(ctx);
	/* A borderless window over the middle of the drawing area, magenta. */
	front = OpenWindowTags(NULL, WA_Left, win->LeftEdge + win->BorderLeft + 100, WA_Top, win->TopEdge + win->BorderTop + 60,
		WA_Width, 120, WA_Height, 80, WA_Borderless, TRUE, WA_SimpleRefresh, TRUE, WA_Activate, FALSE, TAG_DONE);
	if (!front) {
		printf("o03: FAIL no front window\n");
		aglDestroyContext(ctx);
		CloseWindow(win);
		return 20;
	}
	p96RectFill(front->RPort, 0, 0, 119, 79, 0xFFFF00FF);
	glViewport(0, 0, WIDTH, HEIGHT);
	for (i = 0; i < 4; ++i) {
		glClearColor(0, 1, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		aglSwapBuffers();
		Delay(5);
	}
	glFinish();
	Delay(25);
	expect("the front window keeps its colour", front->RPort, 60, 40, 0xFF00FF);
	expect("the front window's corner", front->RPort, 1, 1, 0xFF00FF);
	expect("the GL window's visible part is drawn", win->RPort, win->BorderLeft + 10, win->BorderTop + 10, 0x00FF00);
	expect("... also right of the front window", win->RPort, win->BorderLeft + 300, win->BorderTop + 100, 0x00FF00);
	expect("... and below it", win->RPort, win->BorderLeft + 150, win->BorderTop + 200, 0x00FF00);

	CloseWindow(front);
	aglDestroyContext(ctx);
	CloseWindow(win);
	OGLES2Close();
	CloseLibrary(P96Base);
	printf("o03: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 10 : 0;
}
