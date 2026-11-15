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
	/* With the variable AGL_RASTPORT set, the context draws into the
	 * window's rastport instead, as StormMESA's AMA_RastPort + AMA_Screen
	 * contexts do, its area given by the window's borders: what shows must
	 * be the same. */
	{
		char value[4];
		struct TagItem rastTags[] = {
			{AMA_RastPort, 0}, {AMA_Screen, 0}, {AMA_Left, 0}, {AMA_Bottom, 0}, {AMA_Width, WIDTH}, {AMA_Height, HEIGHT},
			{AMA_RGBMode, GL_TRUE}, {AMA_DoubleBuf, GL_TRUE}, {TAG_DONE, 0}
		};
		if (GetVar("AGL_RASTPORT", value, sizeof(value), 0) >= 0) {
			rastTags[0].ti_Data = (ULONG) window->RPort;
			rastTags[1].ti_Data = (ULONG) window->WScreen;
			rastTags[2].ti_Data = window->BorderLeft;
			rastTags[3].ti_Data = window->BorderBottom;
			printf("%s: AMA_RastPort context\n", test_name);
			context = AmigaMesaCreateContext(rastTags);
		}
		else context = AmigaMesaCreateContext(tags);
	}
	if (!context) fail("AmigaMesaCreateContext", 0);
	else {
		if (test_setup() == 0) {
			for (frame = 0; frame < FRAMES; ++frame) {
				test_draw(frame);
				AmigaMesaSwapBuffers((const void *) context);
				Delay(5);
			}
			report_shown(test_name, window);
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
