/* Window and capture label handling shared by the Warp3D and agl tests. */
#ifndef QUARKTEX_TESTS_WINDOW_H
#define QUARKTEX_TESTS_WINDOW_H

#include <intuition/intuition.h>

#define WIDTH 320
#define HEIGHT 240
#define FRAMES 3

/* Names the frames the host captures for this test (QTTEST:capture/label.txt). */
void write_label(const char *name);

/* Opens an activated WIDTH x HEIGHT window on the default public screen. */
struct Window *open_window(const char *title);

/* Waits until the window is the active one; 0 on time-out. */
int wait_active(struct Window *window);

#endif
