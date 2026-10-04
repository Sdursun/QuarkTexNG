/*
 * Shared frame for the agl.library (StormMESA) reference tests.
 *
 * A test defines test_name and three functions; main() in agl_common.c opens a
 * window, creates an AmigaMesa context on it and draws FRAMES frames. Each
 * AmigaMesaSwapBuffers makes the host swap, and a capture-enabled host writes
 * the frame to QTTEST:capture/<test_name>_<frame>.bmp.
 */
#ifndef QUARKTEX_TESTS_AGL_COMMON_H
#define QUARKTEX_TESTS_AGL_COMMON_H

#include <exec/types.h>
#include <utility/tagitem.h>
#include "gl.h"          /* GL types and constants (gl/) */
#include "agl_calls.h"   /* generated from agl.library/agl_lib.fd */
#include "window.h"

/* Tags of AmigaMesaCreateContext (agl.library/Amigamesa.h) */
#define AMA_Dummy       (TAG_USER + 32)
#define AMA_Window      (AMA_Dummy + 0x0007)
#define AMA_DoubleBuf   (AMA_Dummy + 0x0030)
#define AMA_RGBMode     (AMA_Dummy + 0x0031)

/* Provided by each test. Return non-zero from test_setup() to fail. */
extern const char test_name[];
int test_setup(void);
void test_draw(int frame);
void test_cleanup(void);

/* Logs a failed check; the test then exits with return code 10. */
void fail(const char *what, ULONG code);

#endif
