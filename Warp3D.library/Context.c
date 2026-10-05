#include "w3d.h"
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <intuition/intuitionbase.h>

/*
 * Contexts. Each one has its own host context (window, OpenGL context,
 * state); the library keeps them in a list, newest first. Windowed contexts
 * present their frame when the application ClipBlits into their window,
 * which QuarkTex catches by patching graphics.library ClipBlit while there
 * is a windowed context. Fullscreen contexts patch some display functions
 * instead, while there is one. (0.53 kept one context's window and size in
 * globals, patched again for every context and swapped on any ClipBlit.)
 */
static QtContext *contexts = NULL;
static int windowed = 0, fullscreens = 0;

typedef ULONG (*osFunc)(VOID);

ULONG blub(VOID) { return 1; }

ULONG (*oldscrollvport)(VOID) = NULL;
ULONG (*olderaserect)(VOID) = NULL;
VOID (*oldRectFill)(__REGA1(struct RastPort *rp), __REGD0(LONG xMin), __REGD1(LONG yMin), __REGD2(LONG xMax), __REGD3(LONG yMax)) = NULL;
ULONG (*oldchangescreenbuffer)(VOID) = NULL;
ULONG (*oldClipBlit)(VOID) = NULL;

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
	selectContext(found->host);
	if (found->left != found->window->LeftEdge + found->window->BorderLeft || found->top != found->window->TopEdge + found->window->BorderTop) {
		found->left = found->window->LeftEdge + found->window->BorderLeft;
		found->top = found->window->TopEdge + found->window->BorderTop;
		moveWindow(found->left, found->top, found->width, found->height);
	}
	swapBuffers();
	return 1;
}

VOID W3D_RectFill(__REGA1(struct RastPort *rp), __REGD0(LONG xMin), __REGD1(LONG yMin), __REGD2(LONG xMax), __REGD3(LONG yMax)) {
	//if (task != FindTask(NULL)) oldRectFill(rp, xMin, yMin, xMax, yMax);
}

static void patch(QtContext *c) {
	if (c->fullscreen) {
		if (fullscreens++) return;
		oldscrollvport = SetFunction((struct Library *)GfxBase, -588, blub);
		olderaserect = SetFunction((struct Library *)GfxBase, -810, blub);
		oldRectFill = SetFunction((struct Library *)GfxBase, -306, (osFunc) W3D_RectFill);
		oldchangescreenbuffer = SetFunction((struct Library *)IntuitionBase, -780, blub);
	}
	else if (!windowed++) oldClipBlit = SetFunction((struct Library *) GfxBase, -552, (osFunc) W3D_ClipBlit);
}

static void unpatch(QtContext *c) {
	if (c->fullscreen) {
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
	int modeid = 0;
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

	for (; CCTags->ti_Tag != TAG_DONE; ++CCTags) { if (CCTags->ti_Tag == W3D_CC_MODEID) { qt->fullscreen = 1; modeid = CCTags->ti_Data; } }

	if (!qt->fullscreen) {
		/* The window is not passed (W3D_CC_BITMAP is the screen's on RTG):
		 * the active one. */
		struct Window *window = IntuitionBase->ActiveWindow;
		qt->window = window;
		qt->left = window->LeftEdge + window->BorderLeft;
		qt->top = window->TopEdge + window->BorderTop;
		qt->width = window->Width - (window->BorderLeft + window->BorderRight);
		qt->height = window->Height - (window->BorderTop + window->BorderBottom);
		qt->host = createContext(qt->left, qt->top, qt->width, qt->height, QT_CONTEXT_CORE);
	}
	if (!qt) {
		qt->host = createContext(0, 0, 0, 0, QT_CONTEXT_CORE);
		GetDisplayInfoData(NULL, (UBYTE*)&dinfo, sizeof(dinfo), DTAG_DIMS, modeid);
		qt->width = dinfo.Nominal.MaxX-dinfo.Nominal.MinX+1;
		qt->height = dinfo.Nominal.MaxY-dinfo.Nominal.MinY+1;
	}

	/* No host library (native_code off, DLL missing or wrong version). */
	if (!qt->host) {
		free(context->drawmem);
		free(context->queue);
		FreeVec(qt);
		if (error) *error = W3D_NODRIVER;
		return NULL;
	}
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
