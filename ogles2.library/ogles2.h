/*
 * QuarkTex's ogles2.library (docs/phase9-ogles2.md): what its hand-written
 * parts share. GLES2's tokens are already OpenGL's own values (no MiniGL-
 * style translation needed); the host runs a real OpenGL 3.3 core context
 * (QT_CONTEXT_CORE, as Warp3D does), so GLES2's shader-only model needs none
 * of minigl/agl's fixed-function emulation either.
 */
#ifndef QUARKTEX_OGLES2_H
#define QUARKTEX_OGLES2_H

#define GL_GLES_PROTOTYPES 0
#include <GLES2/gl2.h>
#include <libraries/ogles2.h>
#include <libraries/ogles2_dispatch.h>
#include <intuition/intuition.h>

/* GLES2 has no double-precision entries, so <GLES2/gl2.h> defines neither:
 * gl/gldeclarations.auto.h (desktop GL 1.1's full set, Warp3D/agl's own)
 * needs them to declare the handful of _gl<Name> functions this library
 * calls by hand (glfuncs.c); the ones GLES2 actually has never use them. */
typedef double GLdouble;
typedef double GLclampd;

/* gl/gl.c (host contexts and the command buffer). */
void glInit(void);
void glExit(void);
ULONG createContext(int left, int top, int width, int height, int flags);
void selectContext(ULONG id);
void moveWindow(int left, int top, int width, int height);
void freeContext(void);
void swapBuffers(void);
struct BitMap;
struct Layer;
int presentable(struct BitMap *bitmap);
void presentInto(struct BitMap *bitmap, struct Layer *layer, LONG left, LONG top, LONG width, LONG height, int wait);
void finishFrame(void);
void logString(char *c);
#define QT_CONTEXT_CORE 1
#define QT_CONTEXT_OFFSCREEN 4

/* A context. <libraries/ogles2_dispatch.h> forward-declares "struct
 * OGLES2Context; /@ opaque @/" -- applications only ever pass the pointer
 * back, so this gives that tag its real body, as MiniGL's own headers do
 * for GLcontext/GLcontext_t. */
struct OGLES2Context {
	ULONG host;
	struct Window *window;   /* NULL for a bitmap context */
	struct Screen *screen;   /* ours, in fullscreen (OGLES2_CCT_MODEID) */
	struct BitMap *bitmap;   /* an application bitmap context's */
	BOOL offscreen;          /* presented into Amiga display memory */
	BOOL ownWindow;
	BOOL ownScreen;
	int left, top, width, height;
};

extern struct OGLES2Context *ogles2_current;

/* lib.c: an entry the library does not implement yet. */
void ogles2_missing(const char *name);

#endif
