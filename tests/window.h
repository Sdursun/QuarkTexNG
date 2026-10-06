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

/* Prints what the Amiga display shows at five points of the window's inner
 * area (corners inset by 10 and the centre), as "shown x,y: r g b", with y
 * from the bottom as in the captured frame: with QuarkTex NG's presenting into
 * display memory (phase 8) they match the frame. Only on Picasso96 screens;
 * informational, the result does not fail the test. */
void report_shown(const char *name, struct Window *window);

/* Waits until the window is the active one; 0 on time-out. */
int wait_active(struct Window *window);

#endif
