#include "ogles2.h"
#include "gldeclarations.auto.h"
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/picasso96.h>
#include <proto/utility.h>
#include <graphics/displayinfo.h>
#include <graphics/modeid.h>
#include <intuition/screens.h>

/*
 * Contexts (docs/phase9-ogles2.md, stage 1): a host OpenGL 3.3 core context
 * (QT_CONTEXT_CORE, as Warp3D's), presented like Warp3D's and minigl's: into
 * Amiga display memory when the target bitmap is one the host can write
 * (phase 8), else into a host window over the Amiga one (only visible while
 * WinUAE runs windowed).
 *
 * aglCreateContext2 supports OGLES2_CCT_WINDOW for now (what the SDK's
 * examples use); OGLES2_CCT_BITMAP and OGLES2_CCT_MODEID are stage 2.
 */

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *P96Base;

static void openLibraries(void) {
	if (!IntuitionBase) IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 39);
	if (!GfxBase) GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 39);
	if (!P96Base) P96Base = OpenLibrary("Picasso96API.library", 2);
}

static void destroy(struct OGLES2Context *c) {
	int i;
	/* Before freeContext: the task's slot (gl/gl.c) goes with its last
	 * context, and qt_taskData would make it a new one. */
	if (ogles2_current == c) ogles2_current = NULL;
	if (c->host) {
		selectContext(c->host);
		freeContext();
	}
	for (i = 0; i < 2; ++i) if (c->mapped[i]) FreeVec(c->mapped[i]);
	FreeVec(c);
}

static struct BitMap *targetBitMap(struct OGLES2Context *c) {
	return c->window ? c->window->RPort->BitMap : c->bitmap;
}

static void *attach(struct OGLES2Context *c) {
	struct BitMap *bitmap = targetBitMap(c);
	c->offscreen = bitmap && presentable(bitmap);
	if (c->offscreen) c->host = createContext(0, 0, c->width, c->height, QT_CONTEXT_CORE | QT_CONTEXT_OFFSCREEN);
	else c->host = createContext(c->left, c->top, c->width, c->height, QT_CONTEXT_CORE);
	if (!c->host) {
		destroy(c);
		return NULL;
	}
	/* GL's own default viewport is 0x0 until set; a GLES2 program may never
	 * call glViewport itself if it never resizes (as minigl.library's
	 * contexts also start out). */
	_glViewport(0, 0, c->width, c->height);
	/* GLES2 vertex shaders always set gl_PointSize themselves; a desktop
	 * core profile only lets them with GL_PROGRAM_POINT_SIZE on. */
	_glEnable(0x8642);
	ogles2_current = c;
	return c;
}

/* The inner area of a window, in screen coordinates. */
static void innerArea(struct OGLES2Context *c, struct Window *w) {
	c->left = w->LeftEdge + w->BorderLeft;
	c->top = w->TopEdge + w->BorderTop;
	c->width = w->Width - (w->BorderLeft + w->BorderRight);
	c->height = w->Height - (w->BorderTop + w->BorderBottom);
}

/* Reads one tag's ULONG value, or def if it is not in the list. */
static ULONG tagValue(struct TagItem *tags, ULONG tag, ULONG def) {
	struct TagItem *item = FindTagItem(tag, tags);
	return item ? item->ti_Data : def;
}

struct OGLES2Context *ogles2_aglCreateContext2(ULONG *errcode, struct TagItem *tags) {
	struct OGLES2Context *c;
	struct Window *window;
	ULONG *width, *height;
	openLibraries();
	if (!IntuitionBase || !GfxBase) {
		if (errcode) *errcode = OGLES2_CCE_NO_DISPLAY;
		return NULL;
	}
	if (!(c = (struct OGLES2Context *) AllocVec(sizeof(struct OGLES2Context), MEMF_ANY | MEMF_CLEAR))) {
		if (errcode) *errcode = OGLES2_CCE_NOMEM;
		return NULL;
	}
	{
		int i;
		for (i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) c->currentAttrib[i][3] = 1; /* GLES2's (0, 0, 0, 1) */
	}
	window = (struct Window *) tagValue(tags, OGLES2_CCT_WINDOW, 0);
	if (!window) {
		/* OGLES2_CCT_BITMAP and OGLES2_CCT_MODEID: stage 2. */
		FreeVec(c);
		if (errcode) *errcode = OGLES2_CCE_BAD_TAGS;
		return NULL;
	}
	c->window = window;
	innerArea(c, window);
	if (!attach(c)) {
		if (errcode) *errcode = OGLES2_CCE_NO_GPU;
		return NULL;
	}
	if ((width = (ULONG *) tagValue(tags, OGLES2_CCT_GET_WIDTH, 0))) *width = c->width;
	if ((height = (ULONG *) tagValue(tags, OGLES2_CCT_GET_HEIGHT, 0))) *height = c->height;
	if (errcode) *errcode = OGLES2_CCE_OK;
	return c;
}

void ogles2_aglDestroyContext(struct OGLES2Context *ctx) {
	if (ctx) destroy(ctx);
}

void ogles2_aglMakeCurrent(struct OGLES2Context *ctx) {
	ogles2_current = ctx;
	if (ctx) selectContext(ctx->host);
}

/* Writes the frame into the window's bitmap, over its inner area (the
 * window's layer locked, so nothing draws there meanwhile), or shows it in
 * the host's own window. */
void ogles2_aglSwapBuffers(struct OGLES2Context *ctx) {
	if (!ctx) return;
	selectContext(ctx->host);
	if (ctx->offscreen) presentInto(targetBitMap(ctx), ctx->window ? ctx->window->WLayer : NULL, ctx->left, ctx->top, ctx->width, ctx->height, 0);
	else swapBuffers();
}

/* Stage 2: resizing, OGLES2_CCT_VSYNC and the other parameters. */
void ogles2_aglSetParams2(struct OGLES2Context *ctx, struct TagItem *tags) {}

/* OGLES2_CCT_BITMAP contexts (agl.library's equivalent, stage 2). */
void ogles2_aglSetBitmap(struct OGLES2Context *ctx, struct BitMap *bitmap) {
	ogles2_missing("aglSetBitmap");
}
