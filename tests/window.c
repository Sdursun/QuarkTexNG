#include <intuition/intuitionbase.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include "window.h"

void write_label(const char *name) {
	BPTR file = Open("QTTEST:capture/label.txt", MODE_NEWFILE);
	if (!file) return;
	FPuts(file, name);
	FPuts(file, "\n");
	Close(file);
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
