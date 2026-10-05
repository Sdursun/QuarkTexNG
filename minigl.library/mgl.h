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
#include "mglcmd.h"

/* gl/gl.c (host contexts and the command buffer). */
void glInit(void);
void glExit(void);
ULONG createContext(int left, int top, int width, int height, int flags);
void selectContext(ULONG id);
void moveWindow(int left, int top, int width, int height);
void freeContext(void);
void swapBuffers(void);
typedef struct {
	ULONG address, bytesPerRow, format, bitmapWidth, bitmapHeight;
	LONG left, top, width, height;
} QtTarget; /* as in gl/gl.h */
void swapBuffersTo(const QtTarget *target);
/* The host writes an offscreen picture as soon as the GPU has finished it,
 * between the next frame's commands; finishFrame has it written now (before
 * waiting for input, when no commands would follow). */
void finishFrame(void);
/* Whether the host can write frames into the bitmap: Picasso96's, of a format
 * RGBFB_R8G8B8 to RGBFB_B5G5R5PC (host/present.h). */
struct BitMap;
struct Layer;
int presentable(struct BitMap *bitmap);
/* Has the host write the frame into the rectangle of the bitmap, which it
 * locks, and the layer (if any) drawing there. wait: it is written before
 * presentInto returns (else as soon as the GPU has it, see finishFrame), for
 * applications that may stop drawing after any frame (agl, Warp3D). */
void presentInto(struct BitMap *bitmap, struct Layer *layer, LONG left, LONG top, LONG width, LONG height, int wait);
void logString(char *c);
extern int qt_fpu_extended; /* as in gl/gl.h */
ULONG *qt_reserve(ULONG words);
ULONG qt_flush(void);
#define QT_CONTEXT_PLAIN 2 /* as in gl/gl.h */
#define QT_CONTEXT_OFFSCREEN 4

/* A context. MiniGL's GLcontext is opaque to applications: they only pass
 * it back, so ours is a different structure. */
typedef struct QtMglContext {
	ULONG host;
	struct Screen *screen;   /* ours, in fullscreen */
	struct Window *window;   /* NULL for a bitmap context */
	struct BitMap *bitmap;   /* a bitmap context's */
	BOOL offscreen;          /* presented into Amiga display memory */
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
