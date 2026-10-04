#include "w3d.h"
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <intuition/intuitionbase.h>

struct Window* window = NULL;
int left, top;

//struct Task *task = NULL;

typedef ULONG (*osFunc)(VOID);

ULONG blub(VOID) { return 1; }

ULONG (*oldscrollvport)(VOID) = NULL;
ULONG (*olderaserect)(VOID) = NULL;
VOID (*oldRectFill)(__REGA1(struct RastPort *rp), __REGD0(LONG xMin), __REGD1(LONG yMin), __REGD2(LONG xMax), __REGD3(LONG yMax)) = NULL;
ULONG (*oldchangescreenbuffer)(VOID) = NULL;
ULONG (*oldClipBlit)(VOID) = NULL;

static ULONG W3D_ClipBlit(VOID) {
	LOG;
	if (left != window->LeftEdge + window->BorderLeft || top != window->TopEdge + window->BorderTop) {
		left = window->LeftEdge + window->BorderLeft;
		top = window->TopEdge + window->BorderTop;
		moveWindow(left, top, width, height);
	}
	swapBuffers();
	return 1;
}

VOID W3D_RectFill(__REGA1(struct RastPort *rp), __REGD0(LONG xMin), __REGD1(LONG yMin), __REGD2(LONG xMax), __REGD3(LONG yMax)) {
	//if (task != FindTask(NULL)) oldRectFill(rp, xMin, yMin, xMax, yMax);
}

/************************** Context functions ***********************************/
void W3D_DestroyContext(__REGA0(W3D_Context *context));

W3D_Context *W3D_CreateContext(__REGA0(ULONG *error),__REGA1(struct TagItem *CCTags)) {
	struct Window* firstWindow;
	struct BitMap* bitmap;
	unsigned int bla;
	void *fake;
	int modeid;
	int created;
	struct DimensionInfo dinfo;
	W3D_Context *context;
	LOG;
	firstWindow = NULL;
	bitmap = NULL;
	bla = 4 * 800 * 600;
	fake = malloc(bla);

	context = (W3D_Context*) malloc(sizeof(W3D_Context));
	context->driver = NULL; context->gfxdriver = NULL; context->drivertype = 0;
	context->regbase = NULL; context->vmembase = NULL;
	context->zbuffer = NULL; context->stencilbuffer = NULL;
	context->state = 0;
	context->drawregion = NULL;
	context->supportedfmt = 0; context->format = 0;
	context->yoffset = 0; context->bprow = 0;
	context->width = 0; context->height = 0; context->depth = 0;
	context->chunky = W3D_FALSE; context->destalpha = W3D_FALSE; context->zbufferalloc = W3D_FALSE;
	context->stbufferalloc = W3D_FALSE; context->HWlocked = W3D_FALSE; context->w3dbitmap = W3D_FALSE;
	context->zbufferlost = W3D_FALSE; context->reserved3 = W3D_FALSE;
	//struct MinList restex;
	//struct MinList tex;
	context->maxtexwidth = 0; context->maxtexheight = 0; context->maxtexwidthp = 0; context->maxtexheightp = 0;
	//W3D_Scissor scissor;
	//W3D_Fog fog;
	context->envsupmask = 0; context->queue = (W3D_Queue*) malloc(sizeof(W3D_Queue)); context->drawmem = fake;
	context->globaltexenvmode = 0;
	context->globaltexenvcolor[0] = 0.0; context->globaltexenvcolor[1] = 0.0; context->globaltexenvcolor[2] = 0.0; context->globaltexenvcolor[3] = 0.0;
	context->DriverBase = NULL;
	context->EnableMask = 0; context->DisableMask = 0;
	context->CurrentChip = 0; context->DriverVersion = 0;
	context->VertexPointer = NULL; context->VPStride = 0; context->VPMode = 0; context->VPFlags = 0;
	context->TexCoordPointer[0] = NULL; context->TPStride[0] = 0; context->CurrentTex[0] = NULL; context->TPVOffs[0] = 0; context->TPWOffs[0] = 0; context->TPFlags[0] = 0;
	context->ColorPointer = NULL; context->CPStride = 0; context->CPMode = 0; context->CPFlags = 0;
	context->FrontFaceOrder = 0; context->specialbuffer = 0;

	for (; CCTags->ti_Tag != TAG_DONE; ++CCTags) { if (CCTags->ti_Tag == W3D_CC_MODEID) { fullscreen = 1; modeid = CCTags->ti_Data; } }

	if (fullscreen) {
		oldscrollvport = SetFunction((struct Library *)GfxBase, -588, blub);
		olderaserect = SetFunction((struct Library *)GfxBase, -810, blub);
		oldRectFill = SetFunction((struct Library *)GfxBase, -306, (osFunc) W3D_RectFill);
		oldchangescreenbuffer = SetFunction((struct Library *)IntuitionBase, -780, blub);
	}
	else oldClipBlit = SetFunction((struct Library *) GfxBase, -552, W3D_ClipBlit);

	//bitmap = GetTagData(W3D_CC_BITMAP, NULL, CCTags);
	if (!fullscreen) {
		window = firstWindow = IntuitionBase->ActiveWindow;
		/*if (window != NULL && bitmap != window->RPort->BitMap) {
			logString((char*) (memoffset + (int)("Oh no")));
			window = window->NextWindow;
			while (bitmap != window->RPort->BitMap && window != NULL && window != firstWindow) window = window->NextWindow;
		}*/
		created = createContext(window->LeftEdge + window->BorderLeft, window->TopEdge + window->BorderTop,
			window->Width - (window->BorderLeft + window->BorderRight), window->Height - (window->BorderTop + window->BorderBottom), QT_CONTEXT_CORE);
		left = window->LeftEdge + window->BorderLeft;
		top = window->TopEdge + window->BorderTop;
		width = window->Width - (window->BorderLeft + window->BorderRight);
		height = window->Height - (window->BorderTop + window->BorderBottom);
	}
	else {
		created = createContext(0, 0, 0, 0, QT_CONTEXT_CORE);
		GetDisplayInfoData(NULL, (UBYTE*)&dinfo, sizeof(dinfo), DTAG_DIMS, modeid);
		left = 0;
		top = 0;
		width = dinfo.Nominal.MaxX-dinfo.Nominal.MinX+1;
		height = dinfo.Nominal.MaxY-dinfo.Nominal.MinY+1;
	}

	/* No host library (native_code off, DLL missing or wrong version). */
	if (!created) {
		W3D_DestroyContext(context);
		if (error) *error = W3D_NODRIVER;
		return NULL;
	}

	//task = FindTask(NULL);

	//default states
	context->state |= W3D_AUTOTEXMANAGEMENT;
	context->state |= W3D_TEXMAPPING;
	context->state |= W3D_GOURAUD;
	w3d_command(QT_W3D_INIT_CONTEXT, 0);
	context->state |= W3D_ZBUFFERUPDATE; //qlDepthMask(GL_FALSE);
	
	if (error) *error = W3D_SUCCESS;
	return context;
}

void W3D_DestroyContext(__REGA0(W3D_Context *context)) {
	LOG;
	if (fullscreen) {
		SetFunction((struct Library *)GfxBase, -588, oldscrollvport);
		SetFunction((struct Library *)GfxBase, -810, olderaserect);
		SetFunction((struct Library *)GfxBase, -306, (osFunc) oldRectFill);
		SetFunction((struct Library *)IntuitionBase, -780, oldchangescreenbuffer);
	}
	else SetFunction((struct Library *) GfxBase, -552, oldClipBlit);
	free(context->drawmem);
	free(context->queue);
	free(context);
	freeContext();
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
	w = w3d_command(QT_W3D_SET_STATE, 2);
	w[0] = state;
	w[1] = action == W3D_ENABLE ? W3D_ENABLE : W3D_DISABLE;
	return W3D_SUCCESS;
}
ULONG W3D_Hint(__REGA0(W3D_Context *context), __REGD0(ULONG mode), __REGD1(ULONG quality)) {
	LOG;
	return W3D_SUCCESS;
}