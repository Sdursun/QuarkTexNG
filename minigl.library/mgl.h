/*
 * QuarkTex's minigl.library (docs/phase7-minigl.md): what its hand-written
 * parts share. They include MiniGL's headers (the 29 SDK), whose GL_*
 * constants already carry OpenGL's own values; mgl_enum only needs to catch
 * MiniGL's private ones (0x7000-0x7FFF) and an application's own unknown
 * numbers. OpenGL's constants are QGL_* (qgl.auto.h), and gl/gl.h, which
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
ULONG *qt_reserve(ULONG words);
ULONG qt_flush(void);
/* Two pointers kept per task (gl/gl.c): [1] is its GLUT state (glu.c). */
APTR *qt_taskData(void);
/* Whether the task still runs (is ready or waiting). */
struct Task;
int qt_alive(struct Task *task);
#define QT_CONTEXT_PLAIN 2 /* as in gl/gl.h */
#define QT_CONTEXT_OFFSCREEN 4

/* A client-side vertex array (glfuncs.c). */
typedef struct {
	BOOL enabled;
	int size;
	unsigned int type; /* OpenGL's */
	int stride;        /* in bytes, never 0 */
	const UBYTE *pointer;
} QtMglArray;

/* The state glfuncs.c keeps on the 68k side: per context, as several
 * programs may use the library at once (docs/multitasking.md). Texture
 * coordinates per unit; glTexCoordPointer and the client state of
 * GL_TEXTURE_COORD_ARRAY go to the unit glClientActiveTextureARB chose. */
typedef struct {
	QtMglArray vertices, colors, texCoords[QT_MGL_TEXTURE_UNITS], normals;
	int clientUnit;
	GLboolean sharedPalette; /* GL_SHARED_TEXTURE_PALETTE_EXT */
	int unpackAlignment;     /* for colour index images, expanded on the 68k */
	UBYTE palette[256][4];   /* glColorTable's */
	BOOL paletteAlpha;
} QtMglClient;

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
	BOOL running;            /* MGLMainLoop, GLUTMainLoop */
	GLbitfield buttons;      /* the mouse buttons held */
	IdleFn idle;
	KeyHandlerFn key;
	MouseHandlerFn mouse;
	SpecialHandlerFn special;
	UWORD *emptyPointer;     /* MGLClearPointer, chip memory */
	QtMglClient client;
} QtMglContext;

#define QT_MGL(context) ((QtMglContext *) (context))

/* The calling program's current context: the variable its copy of the
 * dispatch table points to (lib.c), which MiniGL 29's applications read and
 * write themselves (MGLD_CTX). */
GLcontext *mgl_currentFor(void);
#define mgl_current (*mgl_currentFor())

/* lib.c: an entry the library does not implement yet, and a call with a
 * constant MiniGL does not know, are logged. */
void mgl_missing(const char *name);
void mgl_unknown(const char *name);

/* glfuncs.c: the vertex arrays of the current context are reset. */
void mgl_resetArrays(GLcontext context);
/* glfuncs.c: sine and cosine, square root (no maths library here). */
void mgl_sinCos(double x, double *s, double *c);
GLfloat mgl_squareRoot(GLfloat x);
/* glu.c: GLUT's state is reset when its window's context goes; at the
 * library's end its clock goes. */
void mgl_glutForget(GLcontext context);
void mgl_gluExit(void);
/* context.c, for the main loops: the messages of the context's window so far
 * to its handlers (FALSE: the loop is to end), and waiting for the next. */
BOOL mgl_handleEvents(QtMglContext *c);
void mgl_waitForEvent(QtMglContext *c);
/* context.c: contexts as MGLCreateContext makes them, a window or a screen
 * (window: not the mglChooseWindowMode setting). */
GLcontext mgl_createContext(int left, int top, int width, int height, int window);

#endif
