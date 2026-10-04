/*
 * Shared frame for the Warp3D reference tests.
 *
 * A test defines TEST_NAME and three functions; main() in common.c opens a
 * window, creates the context and draws FRAMES frames. Each ClipBlit makes
 * QuarkTex swap buffers, and a capture-enabled QuarkTex.alib writes the frame
 * to QTTEST:capture/<TEST_NAME>_<frame>.bmp.
 */
#ifndef QUARKTEX_TESTS_COMMON_H
#define QUARKTEX_TESTS_COMMON_H

#include <exec/types.h>
#include "warp3d_calls.h"

#define WIDTH 320
#define HEIGHT 240
#define FRAMES 3

extern W3D_Context *context;

/* Provided by each test. Return non-zero from test_setup() to fail. */
extern const char test_name[];
int test_setup(void);
void test_draw(int frame);
void test_cleanup(void);

/* Helpers. */
void set_vertex(W3D_Vertex *v, float x, float y, float z, float r, float g, float b, float a);
void set_uv(W3D_Vertex *v, float u, float v_);
void fail(const char *what, ULONG code);

#endif
