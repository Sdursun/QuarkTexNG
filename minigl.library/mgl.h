/*
 * QuarkTex's minigl.library (docs/phase7-minigl.md): what its hand-written
 * parts share. They include MiniGL's headers, whose GL_* constants are
 * MiniGL's own numbers; OpenGL's are QGL_* (qgl.auto.h), and gl/gl.h, which
 * defines GL_* too, is replaced by the declarations below.
 */
#ifndef QUARKTEX_MGL_H
#define QUARKTEX_MGL_H

#define MINIGL_LIBRARY_BUILD
#include <libraries/minigl_dispatch.h>
#include <intuition/intuition.h>
#include "qgl.auto.h"

/* gl/gl.c (host contexts and the command buffer). */
void glInit(void);
void glExit(void);
ULONG createContext(int left, int top, int width, int height, int flags);
void selectContext(ULONG id);
void moveWindow(int left, int top, int width, int height);
void freeContext(void);
void swapBuffers(void);
void logString(char *c);
ULONG qt_flush(void);
#define QT_CONTEXT_PLAIN 2 /* as in gl/gl.h */

/* A context. MiniGL's GLcontext is opaque to applications: they only pass
 * it back, so ours is a different structure. */
typedef struct QtMglContext {
	ULONG host;
	struct Screen *screen;   /* ours, in fullscreen */
	struct Window *window;
	BOOL ownWindow;
	BOOL fullscreen;
	int left, top, width, height;
	BOOL running;            /* MGLMainLoop */
	IdleFn idle;
	KeyHandlerFn key;
	MouseHandlerFn mouse;
	SpecialHandlerFn special;
	UWORD *emptyPointer;     /* MGLClearPointer, chip memory */
} QtMglContext;

#define QT_MGL(context) ((QtMglContext *) (context))

extern GLcontext mgl_current;

/* lib.c: an entry the library does not implement yet, and a call with a
 * constant MiniGL does not know, are logged. */
void mgl_missing(const char *name);
void mgl_unknown(const char *name);

/* glfuncs.c: the vertex arrays of the current context are reset. */
void mgl_resetArrays(void);

#endif
