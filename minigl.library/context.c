#include "mgl.h"
#include <exec/memory.h>
#include <graphics/displayinfo.h>
#include <graphics/modeid.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/picasso96.h>

/*
 * Contexts: a screen and a backdrop window (fullscreen) or a window on the
 * default public screen, the application's own window, or its bitmap; each
 * gets a host OpenGL compatibility context (QT_CONTEXT_PLAIN: no 0.53 model
 * view matrix, a 24-bit depth buffer). MGLSwitchDisplay presents a frame.
 *
 * On a Picasso96 bitmap of a format the host writes, the context is
 * offscreen (phase 8): the host draws into a picture of its own and writes
 * it into the bitmap at each MGLSwitchDisplay, so the emulator shows it like
 * any other Amiga graphics, also fullscreen. Otherwise (planar or 8-bit
 * screens) the host draws into a window of its own over the Amiga window, as
 * before, which only shows when the emulator runs in a window.
 *
 * MiniGL's GLUT-like main loop is there for its demos.
 */

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *P96Base;

/* The mglChoose* settings, for the next context. MiniGL's defaults. */
static GLboolean windowMode = GL_FALSE;
static int pixelDepth = 16;

static void openLibraries(void) {
	if (!IntuitionBase) IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 39);
	if (!GfxBase) GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 39);
	if (!P96Base) P96Base = OpenLibrary("Picasso96API.library", 2);
}

#define QT_IDCMP (IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE)

/* The inner area of a window, in screen coordinates. */
static void innerArea(QtMglContext *c) {
	struct Window *w = c->window;
	c->left = w->LeftEdge + w->BorderLeft;
	c->top = w->TopEdge + w->BorderTop;
	c->width = w->Width - (w->BorderLeft + w->BorderRight);
	c->height = w->Height - (w->BorderTop + w->BorderBottom);
}

static void destroy(QtMglContext *c) {
	if (c->host) {
		selectContext(c->host);
		freeContext();
	}
	if (c->window && c->ownWindow) CloseWindow(c->window);
	if (c->screen) CloseScreen(c->screen);
	if (c->emptyPointer) FreeVec(c->emptyPointer);
	if (mgl_current == (GLcontext) c) mgl_current = NULL;
	FreeVec(c);
}

/* The bitmap a context's frames go to. */
static struct BitMap *targetBitMap(QtMglContext *c) {
	return c->window ? c->window->RPort->BitMap : c->bitmap;
}

/* Whether the host can write frames into the bitmap: Picasso96's, in one of
 * the formats RGBFB_R8G8B8 to RGBFB_B5G5R5PC (host/present.h). */
static BOOL presentable(struct BitMap *bitmap) {
	ULONG format;
	if (!P96Base || !bitmap || !p96GetBitMapAttr(bitmap, P96BMA_ISP96)) return FALSE;
	format = p96GetBitMapAttr(bitmap, P96BMA_RGBFORMAT);
	return format >= RGBFB_R8G8B8 && format <= RGBFB_B5G5R5PC;
}

/* The host context; the context becomes the current one. */
static void *attach(QtMglContext *c) {
	c->offscreen = presentable(targetBitMap(c));
	if (c->offscreen) c->host = createContext(0, 0, c->width, c->height, QT_CONTEXT_PLAIN | QT_CONTEXT_OFFSCREEN);
	/* A host window in fullscreen: the screen's size at the top left of the
	 * display, as the display mode can be larger than the screen (JK2 got a
	 * 640 x 480 screen in a 1024 x 768 mode) and the screen shows at its top
	 * left. */
	else if (c->fullscreen) c->host = createContext(0, 0, c->width, c->height, QT_CONTEXT_PLAIN);
	else if (c->window) c->host = createContext(c->left, c->top, c->width, c->height, QT_CONTEXT_PLAIN);
	if (!c->host) {
		destroy(c);
		return NULL;
	}
	/* Pixel data stays big-endian in Amiga memory; the host turns 16- and
	 * 32-bit components around (as agl.library does, see glPixelStorei). */
	_glPixelStorei(QGL_UNPACK_SWAP_BYTES, 1);
	_glPixelStorei(QGL_PACK_SWAP_BYTES, 1);
	_glViewport(0, 0, c->width, c->height);
	mgl_current = (GLcontext) c;
	mgl_resetArrays();
	return c;
}

static QtMglContext *allocate(void) {
	openLibraries();
	if (!IntuitionBase || !GfxBase) return NULL;
	return (QtMglContext *) AllocVec(sizeof(QtMglContext), MEMF_ANY | MEMF_CLEAR);
}

/* Appends n in decimal (hex: in hexadecimal) to the text at *p. */
static void appendNumber(char **p, ULONG n, BOOL hex) {
	char digits[12];
	int count = 0;
	do {
		ULONG digit = hex ? n & 15 : n % 10;
		digits[count++] = (char) (digit < 10 ? '0' + digit : 'A' + digit - 10);
		n = hex ? n >> 4 : n / 10;
	} while (n);
	while (count) *(*p)++ = digits[--count];
}

static void appendText(char **p, const char *text) {
	while (*text) *(*p)++ = *text++;
}

/* Logs the screen opened and the display mode it is in (their sizes can
 * differ: then only part of the screen shows). */
static void logScreen(int width, int height, ULONG mode) {
	struct DimensionInfo dims;
	char line[96], *p = line;
	appendText(&p, "minigl.library: screen ");
	appendNumber(&p, (ULONG) width, FALSE);
	appendText(&p, "x");
	appendNumber(&p, (ULONG) height, FALSE);
	appendText(&p, " in mode 0x");
	appendNumber(&p, mode, TRUE);
	if (GetDisplayInfoData(NULL, (UBYTE *) &dims, sizeof(dims), DTAG_DIMS, mode)) {
		appendText(&p, " (");
		appendNumber(&p, (ULONG) (dims.Nominal.MaxX - dims.Nominal.MinX + 1), FALSE);
		appendText(&p, "x");
		appendNumber(&p, (ULONG) (dims.Nominal.MaxY - dims.Nominal.MinY + 1), FALSE);
		appendText(&p, ")");
	}
	*p = 0;
	logString(line);
}

/* A screen of width x height in the mode (INVALID_ID: the best one for that
 * size and the chosen depth) and a backdrop window on it for the input. */
static void *openScreen(QtMglContext *c, ULONG mode, int width, int height) {
	int depth = pixelDepth > 16 ? 24 : pixelDepth > 8 ? 16 : 8;
	if (mode == INVALID_ID) {
		/* Picasso96 picks the RTG mode of that size; graphics.library's
		 * BestModeID gave RTCW's 1024 x 768 screen a 640 x 480 mode (also
		 * with the Desired tags), of which only the top left showed. It
		 * stays for systems without Picasso96. */
		mode = INVALID_ID;
		if (P96Base) {
			mode = p96BestModeIDTags(P96BIDTAG_NominalWidth, width, P96BIDTAG_NominalHeight, height,
				P96BIDTAG_Depth, depth, TAG_DONE);
		}
		if (mode == INVALID_ID) {
			mode = BestModeID(BIDTAG_DesiredWidth, width, BIDTAG_DesiredHeight, height,
				BIDTAG_NominalWidth, width, BIDTAG_NominalHeight, height, BIDTAG_Depth, depth, TAG_DONE);
		}
		if (mode == INVALID_ID) {
			destroy(c);
			return NULL;
		}
	}
	c->screen = OpenScreenTags(NULL, SA_DisplayID, mode, SA_Width, width, SA_Height, height, SA_Depth, depth,
		SA_Quiet, TRUE, SA_ShowTitle, FALSE, SA_Type, CUSTOMSCREEN, SA_Title, (ULONG) "MiniGL", TAG_DONE);
	if (!c->screen) {
		destroy(c);
		return NULL;
	}
	logScreen(width, height, mode);
	c->window = OpenWindowTags(NULL, WA_CustomScreen, (ULONG) c->screen, WA_Left, 0, WA_Top, 0,
		WA_Width, width, WA_Height, height, WA_Backdrop, TRUE, WA_Borderless, TRUE, WA_Activate, TRUE,
		WA_RMBTrap, TRUE, WA_ReportMouse, TRUE, WA_IDCMP, QT_IDCMP, TAG_DONE);
	if (!c->window) {
		destroy(c);
		return NULL;
	}
	c->ownWindow = TRUE;
	c->fullscreen = TRUE;
	c->width = width;
	c->height = height;
	return attach(c);
}

static void *openWindow(QtMglContext *c, int left, int top, int width, int height) {
	c->window = OpenWindowTags(NULL, WA_Left, left, WA_Top, top, WA_InnerWidth, width, WA_InnerHeight, height,
		WA_Title, (ULONG) "MiniGL", WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE,
		WA_Activate, TRUE, WA_RMBTrap, TRUE, WA_ReportMouse, TRUE, WA_IDCMP, QT_IDCMP, TAG_DONE);
	if (!c->window) {
		destroy(c);
		return NULL;
	}
	c->ownWindow = TRUE;
	innerArea(c);
	return attach(c);
}

void *mgl_MGLCreateContext(int offx, int offy, int w, int h) {
	QtMglContext *c = allocate();
	if (!c) return NULL;
	if (windowMode) return openWindow(c, offx, offy, w, h);
	return openScreen(c, INVALID_ID, w, h);
}

/* ID from mglGetSupportedScreenModes, MGL_SM_BESTMODE or MGL_SM_WINDOWMODE;
 * *w and *h are the size asked for and get the one opened. */
void *mgl_MGLCreateContextFromID(GLint ID, GLint *w, GLint *h) {
	QtMglContext *c = allocate();
	struct DimensionInfo dims;
	void *result;
	if (!c) return NULL;
	if ((ULONG) ID == MGL_SM_WINDOWMODE) return openWindow(c, 0, 0, *w, *h);
	if ((ULONG) ID != MGL_SM_BESTMODE && GetDisplayInfoData(NULL, (UBYTE *) &dims, sizeof(dims), DTAG_DIMS, ID)) {
		*w = dims.Nominal.MaxX - dims.Nominal.MinX + 1;
		*h = dims.Nominal.MaxY - dims.Nominal.MinY + 1;
		result = openScreen(c, ID, *w, *h);
	}
	else result = openScreen(c, INVALID_ID, *w, *h);
	return result;
}

/* The application's window, which must outlive the context. */
void *mgl_MGLCreateContextFromWindow(struct Window *window) {
	QtMglContext *c;
	if (!window || !(c = allocate())) return NULL;
	c->window = window;
	innerArea(c);
	return attach(c);
}

/* A bitmap the application presents itself, which must outlive the context:
 * MGLSwitchDisplay writes the frame into it. Only a Picasso96 one the host
 * can write. */
void *mgl_MGLCreateContextFromBitMap(struct BitMap *bitmap) {
	QtMglContext *c;
	if (!bitmap || !(c = allocate())) return NULL;
	if (!presentable(bitmap)) {
		FreeVec(c);
		return NULL;
	}
	c->bitmap = bitmap;
	c->width = (int) p96GetBitMapAttr(bitmap, P96BMA_WIDTH);
	c->height = (int) p96GetBitMapAttr(bitmap, P96BMA_HEIGHT);
	return attach(c);
}

void mgl_MGLDeleteContext(GLcontext context) {
	if (context) destroy(QT_MGL(context));
}

/* Writes the frame into the bitmap, locked, over the window's inner area
 * (the window's layer locked as well, so nothing draws there meanwhile). */
static void presentFrame(QtMglContext *c) {
	struct BitMap *bitmap = targetBitMap(c);
	struct RenderInfo info;
	QtTarget target;
	LONG lock;
	if (c->window) LockLayerRom(c->window->WLayer);
	lock = p96LockBitMap(bitmap, (UBYTE *) &info, sizeof(info));
	if (lock) {
		target.address = (ULONG) info.Memory;
		target.bytesPerRow = (ULONG) info.BytesPerRow;
		target.format = (ULONG) info.RGBFormat;
		target.bitmapWidth = p96GetBitMapAttr(bitmap, P96BMA_WIDTH);
		target.bitmapHeight = p96GetBitMapAttr(bitmap, P96BMA_HEIGHT);
		target.left = c->window ? c->left : 0;
		target.top = c->window ? c->top : 0;
		target.width = c->width;
		target.height = c->height;
		swapBuffersTo(&target);
		p96UnlockBitMap(bitmap, lock);
	}
	else swapBuffersTo(NULL);
	if (c->window) UnlockLayerRom(c->window->WLayer);
}

/* glFinish completes the presentation too: an offscreen frame is in display
 * memory when it returns. */
void mgl_GLFinish(GLcontext context) {
	QtMglContext *c = QT_MGL(context);
	_glFinish();
	if (c && c->offscreen) finishFrame();
}

/* Presents the frame, following the window if it moved or changed size. */
void mgl_MGLSwitchDisplay(GLcontext context) {
	QtMglContext *c = QT_MGL(context);
	if (!c) return;
	selectContext(c->host);
	if (!c->fullscreen && c->window) {
		struct Window *w = c->window;
		if (c->left != w->LeftEdge + w->BorderLeft || c->top != w->TopEdge + w->BorderTop
				|| c->width != w->Width - (w->BorderLeft + w->BorderRight) || c->height != w->Height - (w->BorderTop + w->BorderBottom)) {
			innerArea(c);
			moveWindow(c->left, c->top, c->width, c->height);
		}
	}
	if (c->offscreen) presentFrame(c);
	else swapBuffers();
}

void mgl_MGLResizeContext(GLcontext context, GLsizei width, GLsizei height) {
	QtMglContext *c = QT_MGL(context);
	if (!c || c->fullscreen || !c->ownWindow) return;
	ChangeWindowBox(c->window, c->window->LeftEdge, c->window->TopEdge,
		width + c->window->BorderLeft + c->window->BorderRight, height + c->window->BorderTop + c->window->BorderBottom);
}

void *mgl_MGLGetWindowHandle(GLcontext context) {
	return context ? QT_MGL(context)->window : NULL;
}

void *mgl_MGLGetInputWindowHandle(GLcontext context) {
	return context ? QT_MGL(context)->window : NULL;
}

/* The host draws; there is no display memory to lock. */
GLboolean mgl_MGLLockDisplay(GLcontext context) { return GL_TRUE; }
void mgl_MGLUnlockDisplay(GLcontext context) {}
void mgl_MGLLockMode(GLcontext context, GLenum lockMode) {}
GLboolean mgl_MGLLockBack(GLcontext context, MGLLockInfo *info) { return GL_FALSE; }
void mgl_MGLEnableSync(GLcontext context, GLboolean enable) {}
void mgl_MGLMinTriArea(GLcontext context, GLfloat area) {}
void mgl_MGLSetZOffset(GLcontext context, GLfloat offset) {}
void mgl_MGLPrintMatrix(GLcontext context, int mode) {}
void mgl_MGLPrintMatrixStack(GLcontext context, int mode) {}
void mgl_MGLWriteShotPPM(GLcontext context, char *filename) {}
void mgl_MGLDrawMultitexBuffer(GLcontext context, GLenum BSrc, GLenum BDst, GLenum TexEnv) {}

void mgl_MGLTexMemStat(GLcontext context, GLint *Current, GLint *Peak) {
	if (Current) *Current = 0;
	if (Peak) *Peak = 0;
}

/* MGLClearPointer hides the mouse pointer, MGLSetPointer shows it again. */
void mgl_MGLClearPointer(GLcontext context) {
	QtMglContext *c = QT_MGL(context);
	if (!c || !c->window) return;
	if (!c->emptyPointer) c->emptyPointer = (UWORD *) AllocVec(16, MEMF_CHIP | MEMF_CLEAR);
	if (c->emptyPointer) SetPointer(c->window, c->emptyPointer, 1, 1, 0, 0);
}

void mgl_MGLSetPointer(GLcontext context) {
	QtMglContext *c = QT_MGL(context);
	if (c && c->window) ClearPointer(c->window);
}

/* --- Settings for the next context --------------------------------------- */

void mgl_mglChooseWindowMode(GLboolean flag) { windowMode = flag; }
void mgl_mglChoosePixelDepth(int depth) { pixelDepth = depth; }
void mgl_mglChooseZBufferDepth(int bits) {}
void mgl_mglChooseGuardBand(GLboolean flag) {}
void mgl_mglChooseMtexBufferSize(int size) {}
void mgl_mglChooseNumberOfBuffers(int number) {}
void mgl_mglChooseTextureBufferSize(int size) {}
void mgl_mglChooseVertexBufferSize(int size) {}
void mgl_mglProhibitAlphaFallback(GLboolean flag) {}
void mgl_mglProhibitMipMapping(GLboolean flag) {}
void mgl_mglProposeCloseDesktop(GLboolean closeme) {}

/* Offers the RTG modes (15 bits and more) to the callback until it returns
 * GL_TRUE; returns that mode's id, MGL_SM_BESTMODE if none was taken. */
GLint mgl_mglGetSupportedScreenModes(MGLScreenModeCallback CallbackFn) {
	ULONG id = INVALID_ID;
	openLibraries();
	if (!GfxBase || !CallbackFn) return (GLint) MGL_SM_BESTMODE;
	while ((id = NextDisplayInfo(id)) != INVALID_ID) {
		struct DimensionInfo dims;
		struct NameInfo name;
		MGLScreenMode mode;
		if (!GetDisplayInfoData(NULL, (UBYTE *) &dims, sizeof(dims), DTAG_DIMS, id) || dims.MaxDepth < 15) continue;
		mode.id = (GLint) id;
		mode.width = dims.Nominal.MaxX - dims.Nominal.MinX + 1;
		mode.height = dims.Nominal.MaxY - dims.Nominal.MinY + 1;
		mode.bit_depth = dims.MaxDepth;
		mode.mode_name[0] = 0;
		if (GetDisplayInfoData(NULL, (UBYTE *) &name, sizeof(name), DTAG_NAME, id)) {
			int i;
			for (i = 0; i < MGL_MAX_MODE - 1 && name.Name[i]; ++i) mode.mode_name[i] = name.Name[i];
			mode.mode_name[i] = 0;
		}
		if (CallbackFn(&mode)) return (GLint) id;
	}
	return (GLint) MGL_SM_BESTMODE;
}

/* --- MiniGL's GLUT-like main loop ---------------------------------------- */

void mgl_MGLIdleFunc(GLcontext context, IdleFn i) { if (context) QT_MGL(context)->idle = i; }
void mgl_MGLKeyFunc(GLcontext context, KeyHandlerFn k) { if (context) QT_MGL(context)->key = k; }
void mgl_MGLMouseFunc(GLcontext context, MouseHandlerFn m) { if (context) QT_MGL(context)->mouse = m; }
void mgl_MGLSpecialFunc(GLcontext context, SpecialHandlerFn s) { if (context) QT_MGL(context)->special = s; }

void mgl_MGLExit(GLcontext context) {
	if (context) QT_MGL(context)->running = FALSE;
}

/* Raw key codes of F1-F10 and the cursor keys, in MGLspecial's order. */
static int specialKey(UWORD code) {
	if (code >= 0x50 && code <= 0x59) return MGLKEY_F1 + (code - 0x50);
	switch (code) {
	case 0x4C: return MGLKEY_CUP;
	case 0x4D: return MGLKEY_CDOWN;
	case 0x4F: return MGLKEY_CLEFT;
	case 0x4E: return MGLKEY_CRIGHT;
	}
	return -1;
}

void mgl_MGLMainLoop(GLcontext context) {
	QtMglContext *c = QT_MGL(context);
	GLbitfield buttons = 0;
	if (!c || !c->window) return;
	c->running = TRUE;
	while (c->running) {
		struct IntuiMessage *message;
		while ((message = (struct IntuiMessage *) GetMsg(c->window->UserPort))) {
			ULONG class = message->Class;
			UWORD code = message->Code;
			WORD x = message->MouseX, y = message->MouseY;
			ReplyMsg((struct Message *) message);
			switch (class) {
			case IDCMP_CLOSEWINDOW:
				c->running = FALSE;
				break;
			case IDCMP_VANILLAKEY:
				if (c->key) c->key((char) code);
				else if (code == 27) c->running = FALSE; /* Esc */
				break;
			case IDCMP_RAWKEY:
				if (c->special && specialKey(code) >= 0) c->special((MGLspecial) specialKey(code));
				break;
			case IDCMP_MOUSEBUTTONS:
				if (code == SELECTDOWN) buttons |= MGL_BUTTON_LEFT;
				else if (code == SELECTUP) buttons &= ~MGL_BUTTON_LEFT;
				else if (code == MENUDOWN) buttons |= MGL_BUTTON_RIGHT;
				else if (code == MENUUP) buttons &= ~MGL_BUTTON_RIGHT;
				if (c->mouse) c->mouse(x, y, buttons);
				break;
			case IDCMP_MOUSEMOVE:
				if (c->mouse) c->mouse(x, y, buttons);
				break;
			}
		}
		if (!c->running) break;
		if (c->idle) c->idle();
		else {
			/* Nothing follows until an event: the last frame now. */
			if (c->offscreen) {
				selectContext(c->host);
				finishFrame();
			}
			WaitPort(c->window->UserPort);
		}
	}
}
