#include <stdio.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include "agl_common.h"

struct Library *aglBase;
static int failed;

void fail(const char *what, ULONG code) {
	printf("%s: FAIL %s (%lu)\n", test_name, what, (unsigned long) code);
	failed = 1;
}

int main(void) {
	struct Window *window;
	ULONG context;
	int frame;
	struct TagItem tags[] = {
		{AMA_Window, 0},
		{AMA_RGBMode, GL_TRUE},
		{AMA_DoubleBuf, GL_TRUE},
		{TAG_DONE, 0}
	};

	write_label(test_name);
	aglBase = OpenLibrary("agl.library", 0);
	if (!aglBase) {
		fail("OpenLibrary agl.library", 0);
		return 20;
	}
	window = open_window(test_name);
	if (!window) {
		fail("OpenWindow", 0);
		CloseLibrary(aglBase);
		return 20;
	}
	wait_active(window);

	tags[0].ti_Data = (ULONG) window;
	context = AmigaMesaCreateContext(tags);
	if (!context) fail("AmigaMesaCreateContext", 0);
	else {
		if (test_setup() == 0) {
			for (frame = 0; frame < FRAMES; ++frame) {
				test_draw(frame);
				AmigaMesaSwapBuffers((const void *) context);
				Delay(5);
			}
		}
		else fail("test_setup", 0);
		test_cleanup();
		AmigaMesaDestroyContext((const void *) context);
	}
	CloseWindow(window);
	CloseLibrary(aglBase);
	if (!failed) printf("%s: OK\n", test_name);
	return failed ? 10 : 0;
}
