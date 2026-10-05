#include <stdio.h>
#include <string.h>
#include <intuition/intuitionbase.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/Picasso96.h>
#include "window.h"

static int label_written(const char *name) {
	char line[64];
	BPTR file = Open("QTTEST:capture/label.txt", MODE_NEWFILE);
	if (!file) return 0;
	FPuts(file, name);
	FPuts(file, "\n");
	Close(file);
	file = Open("QTTEST:capture/label.txt", MODE_OLDFILE);
	if (!file) return 0;
	line[0] = 0;
	FGets(file, line, sizeof(line));
	Close(file);
	return strncmp(line, name, strlen(name)) == 0 && line[strlen(name)] == '\n';
}

/* The label file lives on the host (in a OneDrive folder, for example),
 * where a write can fail now and then; then the frames would go out under
 * the previous test's name. So the label is read back and written again
 * until it is right. */
void write_label(const char *name) {
	int attempt;
	for (attempt = 1; attempt <= 20; ++attempt) {
		if (label_written(name)) {
			if (attempt > 1) printf("%s: label written on attempt %d\n", name, attempt);
			return;
		}
		Delay(5);
	}
	printf("%s: FAIL could not write QTTEST:capture/label.txt\n", name);
}

struct Window *open_window(const char *title) {
	return OpenWindowTags(NULL,
		WA_Title, (ULONG) title,
		WA_InnerWidth, WIDTH,
		WA_InnerHeight, HEIGHT,
		WA_Left, 40,
		WA_Top, 40,
		WA_DragBar, TRUE,
		WA_RMBTrap, TRUE,
		WA_Activate, TRUE,
		TAG_DONE);
}

int wait_active(struct Window *window) {
	int i;
	for (i = 0; i < 100; ++i) {
		if (IntuitionBase->ActiveWindow == window) return 1;
		Delay(2);
	}
	return 0;
}

void report_shown(const char *name, struct Window *window) {
	static const int points[5][2] = {{10, 10}, {WIDTH - 11, 10}, {WIDTH / 2, HEIGHT / 2}, {10, HEIGHT - 11}, {WIDTH - 11, HEIGHT - 11}};
	struct Library *P96Base = OpenLibrary("Picasso96API.library", 2);
	int i;
	if (!P96Base) return;
	for (i = 0; i < 5; ++i) {
		int x = points[i][0], y = points[i][1];
		ULONG argb = p96ReadPixel(window->RPort, (UWORD) (window->BorderLeft + x), (UWORD) (window->BorderTop + HEIGHT - 1 - y));
		printf("%s: shown %d,%d: %d %d %d\n", name, x, y, (int) ((argb >> 16) & 0xFF), (int) ((argb >> 8) & 0xFF), (int) (argb & 0xFF));
	}
	CloseLibrary(P96Base);
}
