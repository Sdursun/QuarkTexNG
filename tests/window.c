#include <stdio.h>
#include <string.h>
#include <intuition/intuitionbase.h>
#include <proto/dos.h>
#include <proto/intuition.h>
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
