#include "w3d.h"
#include <exec/memory.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/picasso96.h>
#include <intuition/intuitionbase.h>

/*
 * Contexts. Each one has its own host context (OpenGL context, state); the
 * library keeps them in a list, newest first.
 *
 * On a Picasso96 bitmap of a format the host writes (gl/gl.h) a context is
 * offscreen (phase 8): the host draws into a picture of its own and its frame
 * is written into Amiga display memory, where the emulator shows it in a
 * window or fullscreen. A windowed context's frame goes into its window when
 * the application ClipBlits into that window or finishes the frame
 * (W3D_FlushFrame); a fullscreen one's into the draw region's bitmap at its y
 * offset when the application shows that bitmap (ChangeScreenBuffer,
 * ScrollVPort, patched to write the frame first and then do their work),
 * changes the draw region or finishes the frame.
 *
 * Otherwise (planar screens) the host draws into a window of its own over the
 * Amiga one, as QuarkTex 0.53 did: windowed contexts present on ClipBlit,
 * fullscreen ones on W3D_SetDrawRegion, with the display functions patched
 * to do nothing. That only shows while the emulator runs in a window.
 */
static QtContext *contexts = NULL;
static int windowed = 0, fullscreens = 0, offscreenFullscreens = 0;

typedef ULONG (*osFunc)(VOID);

ULONG blub(VOID) { return 1; }

ULONG (*oldscrollvport)(VOID) = NULL;
ULONG (*olderaserect)(VOID) = NULL;
VOID (*oldRectFill)(__REGA1(struct RastPort *rp), __REGD0(LONG xMin), __REGD1(LONG yMin), __REGD2(LONG xMax), __REGD3(LONG yMax)) = NULL;
ULONG (*oldchangescreenbuffer)(VOID) = NULL;
ULONG (*oldClipBlit)(VOID) = NULL;

/* Calls a library function patched over, with its base in a6 and its
 * arguments in a0 and a1. */
static ULONG callOriginal(ULONG (*function)(VOID), struct Library *base, APTR first, APTR second) {
	register APTR a0 __asm("a0") = first;
	register APTR a1 __asm("a1") = second;
	register struct Library *a6 __asm("a6") = base;
	register ULONG d0 __asm("d0");
	__asm volatile ("jsr (%4)" : "=r" (d0), "+r" (a0), "+r" (a1), "+r" (a6) : "a" (function) : "d1", "fp0", "fp1", "cc", "memory");
	return d0;
}

/* The W3D_Context fields that describe the draw region, filled as Warp3D
 * fills them (0.53 left them 0): applications and MiniGL read them. */
void w3d_describe(W3D_Context *context, struct BitMap *bitmap, int yoffset) {
	/* Picasso96's RGBFormats (RGBFB_NONE to RGBFB_B5G5R5PC) as W3D_FMT_*. */
	static const ULONG formats[14] = {0, W3D_FMT_CLUT, W3D_FMT_R8G8B8, W3D_FMT_B8G8R8, W3D_FMT_R5G6B5PC, W3D_FMT_R5G5B5PC,
		W3D_FMT_A8R8G8B8, W3D_FMT_A8B8G8R8, W3D_FMT_R8G8B8A8, W3D_FMT_B8G8R8A8, W3D_FMT_R5G6B5, W3D_FMT_R5G5B5,
		W3D_FMT_B5G6R5PC, W3D_FMT_B5G5R5PC};
	if (!bitmap) return;
	context->drawregion = bitmap;
	context->yoffset = yoffset;
	context->supportedfmt = driver.formats;
	if (P96Base && p96GetBitMapAttr(bitmap, P96BMA_ISP96)) {
		ULONG format = p96GetBitMapAttr(bitmap, P96BMA_RGBFORMAT);
		context->width = (int) p96GetBitMapAttr(bitmap, P96BMA_WIDTH);
		context->height = (int) p96GetBitMapAttr(bitmap, P96BMA_HEIGHT);
		context->bprow = (int) p96GetBitMapAttr(bitmap, P96BMA_BYTESPERROW);
		context->depth = (int) p96GetBitMapAttr(bitmap, P96BMA_BITSPERPIXEL);
		context->format = format < 14 ? formats[format] : 0;
	}
	else {
		context->width = (int) GetBitMapAttr(bitmap, BMA_WIDTH);
		context->height = (int) GetBitMapAttr(bitmap, BMA_HEIGHT);
		context->bprow = bitmap->BytesPerRow;
		context->depth = (int) GetBitMapAttr(bitmap, BMA_DEPTH);
		context->format = W3D_FMT_CLUT;
	}
	context->chunky = context->format == W3D_FMT_CLUT ? W3D_TRUE : W3D_FALSE;
	context->scissor.left = 0;
	context->scissor.top = 0;
	context->scissor.width = context->width;
	context->scissor.height = context->height;
	/* As W3D_Query answers (Hardware.c). */
	context->maxtexwidth = context->maxtexheight = 2048;
	context->maxtexwidthp = context->maxtexheightp = 2048;
}

void w3d_present(QtContext *qt) {
	if (!qt->offscreen || !qt->dirty) return;
	qt->dirty = 0;
	selectContext(qt->host);
	if (qt->window) presentInto(qt->window->RPort->BitMap, qt->window->WLayer, qt->left, qt->top, qt->width, qt->height, 1);
	else presentInto(qt->bitmap, NULL, 0, qt->yoffset, qt->width, qt->height, 1);
}

/* The windowed context drawing into the destination RastPort; if there is
 * none, the newest windowed one (0.53 swapped its only context on every
 * ClipBlit). */
static ULONG W3D_ClipBlit(__REGA0(struct RastPort *source), __REGA1(struct RastPort *destination)) {
	QtContext *c, *found = NULL;
	LOG;
	for (c = contexts; c; c = c->next) {
		if (!c->window) continue;
		if (c->window->RPort == destination) {
			found = c;
			break;
		}
		if (!found) found = c;
	}
	if (!found) return 1;
	if (found->left != found->window->LeftEdge + found->window->BorderLeft || found->top != found->window->TopEdge + found->window->BorderTop) {
		found->left = found->window->LeftEdge + found->window->BorderLeft;
		found->top = found->window->TopEdge + found->window->BorderTop;
		if (!found->offscreen) {
			selectContext(found->host);
			moveWindow(found->left, found->top, found->width, found->height);
		}
	}
	if (found->offscreen) w3d_present(found);
	else {
		selectContext(found->host);
		swapBuffers();
	}
	return 1;
}

/* An offscreen fullscreen context's frame goes into the bitmap before it is
 * shown, then the original function shows it. */
static ULONG W3D_ChangeScreenBuffer(__REGA0(struct Screen *screen), __REGA1(struct ScreenBuffer *buffer)) {
	QtContext *c;
	for (c = contexts; c; c = c->next) {
		if (!c->window && c->offscreen && c->dirty) {
			if (buffer && buffer->sb_BitMap != c->bitmap && presentable(buffer->sb_BitMap)) c->bitmap = buffer->sb_BitMap;
			w3d_present(c);
		}
	}
	return callOriginal(oldchangescreenbuffer, (struct Library *) IntuitionBase, screen, buffer);
}

static ULONG W3D_ScrollVPort(__REGA0(struct ViewPort *viewport)) {
	QtContext *c;
	for (c = contexts; c; c = c->next) if (!c->window && c->offscreen) w3d_present(c);
	return callOriginal(oldscrollvport, (struct Library *) GfxBase, viewport, NULL);
}

VOID W3D_RectFill(__REGA1(struct RastPort *rp), __REGD0(LONG xMin), __REGD1(LONG yMin), __REGD2(LONG xMax), __REGD3(LONG yMax)) {
	//if (task != FindTask(NULL)) oldRectFill(rp, xMin, yMin, xMax, yMax);
}

static void patch(QtContext *c) {
	if (c->fullscreen && c->offscreen) {
		if (offscreenFullscreens++) return;
		oldscrollvport = SetFunction((struct Library *) GfxBase, -588, (osFunc) W3D_ScrollVPort);
		oldchangescreenbuffer = SetFunction((struct Library *) IntuitionBase, -780, (osFunc) W3D_ChangeScreenBuffer);
	}
	else if (c->fullscreen) {
		if (fullscreens++) return;
		oldscrollvport = SetFunction((struct Library *)GfxBase, -588, blub);
		olderaserect = SetFunction((struct Library *)GfxBase, -810, blub);
		oldRectFill = SetFunction((struct Library *)GfxBase, -306, (osFunc) W3D_RectFill);
		oldchangescreenbuffer = SetFunction((struct Library *)IntuitionBase, -780, blub);
	}
	else if (!windowed++) oldClipBlit = SetFunction((struct Library *) GfxBase, -552, (osFunc) W3D_ClipBlit);
}

static void unpatch(QtContext *c) {
	if (c->fullscreen && c->offscreen) {
		if (--offscreenFullscreens) return;
		SetFunction((struct Library *) GfxBase, -588, oldscrollvport);
		SetFunction((struct Library *) IntuitionBase, -780, oldchangescreenbuffer);
	}
	else if (c->fullscreen) {
		if (--fullscreens) return;
		SetFunction((struct Library *)GfxBase, -588, oldscrollvport);
		SetFunction((struct Library *)GfxBase, -810, olderaserect);
		SetFunction((struct Library *)GfxBase, -306, (osFunc) oldRectFill);
		SetFunction((struct Library *)IntuitionBase, -780, oldchangescreenbuffer);
	}
	else if (!--windowed) SetFunction((struct Library *) GfxBase, -552, oldClipBlit);
}

/************************** Context functions ***********************************/

W3D_Context *W3D_CreateContext(__REGA0(ULONG *error),__REGA1(struct TagItem *CCTags)) {
	ULONG modeid = 0;
	struct DimensionInfo dinfo;
	QtContext *qt;
	W3D_Context *context;
	LOG;

	qt = (QtContext*) AllocVec(sizeof(QtContext), MEMF_ANY | MEMF_CLEAR);
	if (!qt) {
		if (error) *error = W3D_NOMEMORY;
		return NULL;
	}
	context = &qt->context;
	context->queue = (W3D_Queue*) malloc(sizeof(W3D_Queue));
	context->drawmem = malloc(4 * 800 * 600);
	context->globaltexenvcolor[0] = 0.0; context->globaltexenvcolor[1] = 0.0; context->globaltexenvcolor[2] = 0.0; context->globaltexenvcolor[3] = 0.0;

	for (; CCTags->ti_Tag != TAG_DONE; ++CCTags) {
		if (CCTags->ti_Tag == W3D_CC_MODEID) {
			qt->fullscreen = 1;
			modeid = CCTags->ti_Data;
		}
		else if (CCTags->ti_Tag == W3D_CC_BITMAP) qt->bitmap = (struct BitMap *) CCTags->ti_Data;
		else if (CCTags->ti_Tag == W3D_CC_YOFFSET) qt->yoffset = (int) CCTags->ti_Data;
	}

	if (!qt->fullscreen) {
		/* The window is not passed (W3D_CC_BITMAP is the screen's on RTG):
		 * the active one. */
		struct Window *window = IntuitionBase->ActiveWindow;
		qt->window = window;
		qt->left = window->LeftEdge + window->BorderLeft;
		qt->top = window->TopEdge + window->BorderTop;
		qt->width = window->Width - (window->BorderLeft + window->BorderRight);
		qt->height = window->Height - (window->BorderTop + window->BorderBottom);
		qt->offscreen = presentable(window->RPort->BitMap);
		if (qt->offscreen) qt->host = createContext(0, 0, qt->width, qt->height, QT_CONTEXT_CORE | QT_CONTEXT_OFFSCREEN);
		else qt->host = createContext(qt->left, qt->top, qt->width, qt->height, QT_CONTEXT_CORE);
	}
	else {
		/* The display mode's size. Since phase 6 (several contexts) this
		 * branch tested qt instead of the fullscreen flag, so a fullscreen
		 * context got no host context and W3D_CreateContext failed. */
		GetDisplayInfoData(NULL, (UBYTE*)&dinfo, sizeof(dinfo), DTAG_DIMS, modeid);
		qt->width = dinfo.Nominal.MaxX-dinfo.Nominal.MinX+1;
		qt->height = dinfo.Nominal.MaxY-dinfo.Nominal.MinY+1;
		qt->offscreen = presentable(qt->bitmap);
		if (qt->offscreen) qt->host = createContext(0, 0, qt->width, qt->height, QT_CONTEXT_CORE | QT_CONTEXT_OFFSCREEN);
		else qt->host = createContext(0, 0, 0, 0, QT_CONTEXT_CORE);
	}

	/* No host library (native_code off, DLL missing or wrong version). */
	if (!qt->host) {
		free(context->drawmem);
		free(context->queue);
		FreeVec(qt);
		if (error) *error = W3D_NODRIVER;
		return NULL;
	}
	w3d_describe(context, qt->bitmap ? qt->bitmap : qt->window ? qt->window->RPort->BitMap : NULL, qt->yoffset);
	patch(qt);
	qt->next = contexts;
	contexts = qt;

	//default states
	context->state |= W3D_AUTOTEXMANAGEMENT;
	context->state |= W3D_TEXMAPPING;
	context->state |= W3D_GOURAUD;
	w3d_command(context, QT_W3D_INIT_CONTEXT, 0);
	context->state |= W3D_ZBUFFERUPDATE; //qlDepthMask(GL_FALSE);

	if (error) *error = W3D_SUCCESS;
	return context;
}

void W3D_DestroyContext(__REGA0(W3D_Context *context)) {
	QtContext **link;
	LOG;
	for (link = &contexts; *link; link = &(*link)->next) {
		if (*link == QT(context)) {
			*link = QT(context)->next;
			break;
		}
	}
	unpatch(QT(context));
	/* The textures the application left (0.53 kept them allocated). */
	W3D_FreeAllTexObj(context);
	w3d_select(context);
	freeContext();
	free(context->drawmem);
	free(context->queue);
	FreeVec(QT(context));
}
ULONG W3D_GetState(__REGA0(W3D_Context *context), __REGD1(ULONG state)) {
	LOG;
	if (context->state & state) return W3D_ENABLED;
	return W3D_DISABLED;
}
/* The host makes the OpenGL calls (host/w3d.cpp). */
ULONG W3D_SetState(W3D_Context *context __asm("a0"), ULONG state __asm("d0"), ULONG action __asm("d1")) {
	ULONG *w;
	LOG;
	if (action == W3D_ENABLE) context->state |= state;
	else context->state &= ~state;
	w = w3d_command(context, QT_W3D_SET_STATE, 2);
	w[0] = state;
	w[1] = action == W3D_ENABLE ? W3D_ENABLE : W3D_DISABLE;
	return W3D_SUCCESS;
}
ULONG W3D_Hint(__REGA0(W3D_Context *context), __REGD0(ULONG mode), __REGD1(ULONG quality)) {
	LOG;
	return W3D_SUCCESS;
}
