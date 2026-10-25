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
 * The surfaces of the SDK's aglCreateContext2: the program's window
 * (OGLES2_CCT_WINDOW), a screen of the library's own with a backdrop window
 * for input (OGLES2_CCT_MODEID), the program's bitmap (OGLES2_CCT_BITMAP),
 * or, with none of these, a window of the library's own on the default
 * public screen.
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
	if (c->window && c->ownWindow) CloseWindow(c->window);
	if (c->screen && c->ownScreen) CloseScreen(c->screen);
	for (i = 0; i < 2; ++i) if (c->mapped[i]) FreeVec(c->mapped[i]);
	FreeVec(c);
}

static struct BitMap *targetBitMap(struct OGLES2Context *c) {
	return c->window ? c->window->RPort->BitMap : c->bitmap;
}

static void *attach(struct OGLES2Context *c) {
	struct BitMap *bitmap = targetBitMap(c);
	c->offscreen = bitmap && presentable(bitmap);
	/* A bitmap is only ever written into: there is no host window for it. */
	if (!c->window && !c->offscreen) {
		destroy(c);
		return NULL;
	}
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

/* The tags that answer (OGLES2_CCT_GET_WIDTH, _GET_HEIGHT, _PISTORM_GET_
 * WINDOW), for aglCreateContext2 and aglSetParams2. The window that gets
 * input is the program's, ours, or the fullscreen backdrop window; NULL for
 * a bitmap. */
static void answer(struct OGLES2Context *c, struct TagItem *tags) {
	ULONG *value;
	struct Window **window;
	if ((value = (ULONG *) tagValue(tags, OGLES2_CCT_GET_WIDTH, 0))) *value = c->width;
	if ((value = (ULONG *) tagValue(tags, OGLES2_CCT_GET_HEIGHT, 0))) *value = c->height;
	if ((window = (struct Window **) tagValue(tags, OGLES2_CCT_PISTORM_GET_WINDOW, 0))) *window = c->window;
}

#define QT_OGLES2_IDCMP (IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW)

/* A full screen in the mode, of the mode's size unless the tags give one,
 * and a backdrop window on it for input. */
static ULONG openScreen(struct OGLES2Context *c, ULONG mode, struct TagItem *tags) {
	struct DimensionInfo dims;
	ULONG width, height;
	if (!GetDisplayInfoData(NULL, (UBYTE *) &dims, sizeof(dims), DTAG_DIMS, mode) || dims.MaxDepth < 24) return OGLES2_CCE_NO_DISPLAY;
	width = tagValue(tags, OGLES2_CCT_PISTORM_WIDTH, dims.Nominal.MaxX - dims.Nominal.MinX + 1);
	height = tagValue(tags, OGLES2_CCT_PISTORM_HEIGHT, dims.Nominal.MaxY - dims.Nominal.MinY + 1);
	c->screen = OpenScreenTags(NULL, SA_DisplayID, mode, SA_Width, width, SA_Height, height, SA_Depth, 24,
		SA_Quiet, TRUE, SA_ShowTitle, FALSE, SA_Type, CUSTOMSCREEN, SA_Title, (ULONG) "ogles2", TAG_DONE);
	if (!c->screen) return OGLES2_CCE_NO_DISPLAY;
	c->ownScreen = TRUE;
	c->window = OpenWindowTags(NULL, WA_CustomScreen, (ULONG) c->screen, WA_Left, 0, WA_Top, 0,
		WA_Width, width, WA_Height, height, WA_Backdrop, TRUE, WA_Borderless, TRUE, WA_Activate, TRUE,
		WA_RMBTrap, TRUE, WA_ReportMouse, TRUE, WA_IDCMP, QT_OGLES2_IDCMP, TAG_DONE);
	if (!c->window) return OGLES2_CCE_NO_DISPLAY;
	c->ownWindow = TRUE;
	c->left = c->top = 0;
	c->width = (int) width;
	c->height = (int) height;
	return OGLES2_CCE_OK;
}

/* A window of our own on the default public screen: no close gadget and no
 * IDCMP (the program sets what it wants with ModifyIDCMP). */
static ULONG openWindow(struct OGLES2Context *c, struct TagItem *tags) {
	c->window = OpenWindowTags(NULL, WA_InnerWidth, tagValue(tags, OGLES2_CCT_PISTORM_WIDTH, 640),
		WA_InnerHeight, tagValue(tags, OGLES2_CCT_PISTORM_HEIGHT, 480), WA_Title, (ULONG) "ogles2",
		WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_Activate, TRUE, WA_RMBTrap, TRUE, WA_ReportMouse, TRUE,
		TAG_DONE);
	if (!c->window) return OGLES2_CCE_NO_DISPLAY;
	c->ownWindow = TRUE;
	innerArea(c, c->window);
	return OGLES2_CCE_OK;
}

struct OGLES2Context *ogles2_aglCreateContext2(ULONG *errcode, struct TagItem *tags) {
	struct OGLES2Context *c;
	struct Window *window;
	struct BitMap *bitmap;
	ULONG mode, error = OGLES2_CCE_OK;
	int i;
	openLibraries();
	if (!IntuitionBase || !GfxBase) {
		if (errcode) *errcode = OGLES2_CCE_NO_DISPLAY;
		return NULL;
	}
	window = (struct Window *) tagValue(tags, OGLES2_CCT_WINDOW, 0);
	bitmap = (struct BitMap *) tagValue(tags, OGLES2_CCT_BITMAP, 0);
	mode = tagValue(tags, OGLES2_CCT_MODEID, tagValue(tags, OGLES2_CCT_CONTEXT_FOR_MODEID, INVALID_ID));
	if ((window != NULL) + (bitmap != NULL) + (mode != INVALID_ID) > 1) {
		if (errcode) *errcode = OGLES2_CCE_BAD_TAGS;
		return NULL;
	}
	if (!(c = (struct OGLES2Context *) AllocVec(sizeof(struct OGLES2Context), MEMF_ANY | MEMF_CLEAR))) {
		if (errcode) *errcode = OGLES2_CCE_NOMEM;
		return NULL;
	}
	for (i = 0; i < QT_OGLES2_MAX_ATTRIBS; ++i) c->currentAttrib[i][3] = 1; /* GLES2's (0, 0, 0, 1) */
	if (window) {
		c->window = window;
		innerArea(c, window);
	}
	else if (bitmap) {
		if (!P96Base || !presentable(bitmap)) error = OGLES2_CCE_NO_DISPLAY;
		else {
			c->bitmap = bitmap;
			c->width = (int) p96GetBitMapAttr(bitmap, P96BMA_WIDTH);
			c->height = (int) p96GetBitMapAttr(bitmap, P96BMA_HEIGHT);
		}
	}
	else if (mode != INVALID_ID) error = openScreen(c, mode, tags);
	else error = openWindow(c, tags);
	if (error != OGLES2_CCE_OK) {
		destroy(c);
		if (errcode) *errcode = error;
		return NULL;
	}
	if (!attach(c)) {
		if (errcode) *errcode = OGLES2_CCE_NO_GPU;
		return NULL;
	}
	answer(c, tags);
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
 * window's layer locked, so nothing draws there meanwhile), or into the
 * program's bitmap, or shows it in the host's own window. */
static void present(struct OGLES2Context *ctx) {
	if (!ctx->offscreen) swapBuffers();
	else if (ctx->window) presentInto(targetBitMap(ctx), ctx->window->WLayer, ctx->left, ctx->top, ctx->width, ctx->height, 0);
	else presentInto(ctx->bitmap, NULL, 0, 0, ctx->width, ctx->height, 0);
}

void ogles2_aglSwapBuffers(struct OGLES2Context *ctx) {
	if (!ctx) return;
	selectContext(ctx->host);
	present(ctx);
}

/* The default framebuffer takes the surface's new size; the viewport stays
 * as the program set it. */
static void resize(struct OGLES2Context *ctx) {
	selectContext(ctx->host);
	if (ctx->offscreen) moveWindow(0, 0, ctx->width, ctx->height);
	else moveWindow(ctx->left, ctx->top, ctx->width, ctx->height);
}

/* OGLES2_CCT_WINDOW (another window, or the same one after IDCMP_NEWSIZE)
 * for a context made with the program's window, and the answering tags.
 * OGLES2_CCT_VSYNC is taken and ignored: frames go into Amiga display
 * memory, which the emulator shows at its own pace. */
void ogles2_aglSetParams2(struct OGLES2Context *ctx, struct TagItem *tags) {
	struct Window *window;
	if (!ctx) return;
	window = (struct Window *) tagValue(tags, OGLES2_CCT_WINDOW, 0);
	if (window && ctx->window && !ctx->ownWindow) {
		int left = ctx->left, top = ctx->top, width = ctx->width, height = ctx->height;
		ctx->window = window;
		innerArea(ctx, window);
		if (ctx->left != left || ctx->top != top || ctx->width != width || ctx->height != height) resize(ctx);
	}
	answer(ctx, tags);
}

/* For a context made with OGLES2_CCT_BITMAP: the open frame goes into the
 * old bitmap, then the context draws into the new one, at its size. */
void ogles2_aglSetBitmap(struct OGLES2Context *ctx, struct BitMap *bitmap) {
	int width, height;
	if (!ctx || !ctx->bitmap || !bitmap || !presentable(bitmap)) return;
	selectContext(ctx->host);
	present(ctx);
	finishFrame();
	ctx->bitmap = bitmap;
	width = (int) p96GetBitMapAttr(bitmap, P96BMA_WIDTH);
	height = (int) p96GetBitMapAttr(bitmap, P96BMA_HEIGHT);
	if (width != ctx->width || height != ctx->height) {
		ctx->width = width;
		ctx->height = height;
		resize(ctx);
	}
}
