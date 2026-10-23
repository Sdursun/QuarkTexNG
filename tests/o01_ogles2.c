/*
 * o01_ogles2: ogles2.library's own checks, through an application built on
 * the PiStorm/Emu68 ogles2 SDK's import library (<proto/ogles2.h>), the way
 * a real GLES2 program uses it. Stage 1 (docs/phase9-ogles2.md): only the
 * context and the entries without a pointer argument are implemented, so
 * this clears the window to a colour and reads it back; it does not draw
 * anything with a shader yet.
 *
 * Built only when the ogles2 SDK headers are there (tests/Makefile).
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

/* What the Amiga display shows at (x, y) of the picture (from the bottom, as
 * glReadPixels has it), after aglSwapBuffers has written the frame into
 * display memory (phase 8). */
static void expectShown(struct Window *w, const char *what, int x, int y, int r, int g, int b) {
	ULONG argb;
	int pr, pg, pb, ok;
	if (!P96Base || !w) {
		printf("o01: SKIP shown %s: no Picasso96 window\n", what);
		return;
	}
	argb = p96ReadPixel(w->RPort, (UWORD) (x + w->BorderLeft), (UWORD) (HEIGHT - 1 - y + w->BorderTop));
	pr = (argb >> 16) & 0xFF;
	pg = (argb >> 8) & 0xFF;
	pb = argb & 0xFF;
	ok = pr >= r - 8 && pr <= r + 8 && pg >= g - 8 && pg <= g + 8 && pb >= b - 8 && pb <= b + 8;
	if (!ok) ++failures;
	printf("o01: %s shown %s: (%d, %d, %d), expected (%d, %d, %d)\n", ok ? "PASS" : "FAIL", what, pr, pg, pb, r, g, b);
}

int main(void) {
	struct Window *win;
	void *ctx;
	ULONG err = 0;
	const char *renderer, *version;

	write_label("o01_ogles2");
	P96Base = OpenLibrary((STRPTR) "Picasso96API.library", 2);

	if (!OGLES2Open()) {
		printf("o01: FAIL no ogles2.library\n");
		return 20;
	}
	if (!(win = open_window("o01: ogles2"))) {
		printf("o01: FAIL no window\n");
		OGLES2Close();
		return 20;
	}
	ctx = aglCreateContextTags2(&err, OGLES2_CCT_WINDOW, (ULONG) win, OGLES2_CCT_DEPTH, 0, OGLES2_CCT_STENCIL, 0,
		TAG_DONE);
	if (!ctx) {
		printf("o01: FAIL aglCreateContextTags2, error %lu\n", (unsigned long) err);
		CloseWindow(win);
		OGLES2Close();
		return 20;
	}
	aglMakeCurrent(ctx);

	renderer = (const char *) glGetString(GL_RENDERER);
	version = (const char *) glGetString(GL_VERSION);
	printf("o01: %s, %s\n", renderer ? renderer : "(null)", version ? version : "(null)");

	{
		int frame;
		glClearColor(0.2f, 0.4f, 0.6f, 1.0f);
		for (frame = 0; frame < FRAMES; ++frame) {
			glClear(GL_COLOR_BUFFER_BIT);
			aglSwapBuffers();
		}
	}
	wait_active(win);
	report_shown("o01_ogles2", win);
	expectShown(win, "cleared", WIDTH / 2, HEIGHT / 2, 51, 102, 153);

	printf("o01: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	aglDestroyContext(ctx);
	CloseWindow(win);
	OGLES2Close();
	return failures ? 5 : 0;
}
