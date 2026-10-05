#include "w3d.h"
#include <proto/exec.h>

struct Warp3DBase {
	struct Library base;
};

#pragma libbase Warp3DBase

W3D_Driver driver;
W3D_Driver *drivers[] = {&driver, NULL};

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *P96Base;

char *bp, b;
int i;



void INIT_0_Warp3D(void) {
	glInit();
	LOG;
	driver.ChipID = W3D_CHIP_RADEON;
	driver.formats = W3D_FMT_CLUT | W3D_FMT_R5G5B5 | W3D_FMT_B5G5R5 | W3D_FMT_R5G5B5PC | W3D_FMT_B5G5R5PC |
		W3D_FMT_R5G6B5 | W3D_FMT_B5G6R5 | W3D_FMT_R5G6B5PC | W3D_FMT_B5G6R5PC | W3D_FMT_R8G8B8 |
		W3D_FMT_B8G8R8 | W3D_FMT_A8R8G8B8 | W3D_FMT_A8B8G8R8 | W3D_FMT_R8G8B8A8 | W3D_FMT_B8G8R8A8;
	driver.name = QT_PRODUCT;
	driver.swdriver = W3D_FALSE;
	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39L);
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 39L);
	P96Base = OpenLibrary("Picasso96API.library", 2);
}

void EXIT_0_Warp3D(void) {
	LOG;
	glExit();
	if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
	if (GfxBase) CloseLibrary((struct Library *)GfxBase);
	if (P96Base) CloseLibrary(P96Base);
}

/*
 * Drawing on the host (gl/w3dcmd.h): one DRAW command for up to
 * QT_W3D_MAX_VERTICES vertices, DRAW_BEGIN/VERTICES/DRAW_END beyond that.
 * The vertices are copied as they are; the copy is most of the 68k work per
 * primitive, so it uses movem: count W3D_Vertex structures (64 bytes each)
 * from source to dest.
 */
void qt_copy_vertices(ULONG *dest, const void *source, ULONG count);

__asm__(
	"	.text\n"
	"	.even\n"
	"	.globl	_qt_copy_vertices\n"
	"_qt_copy_vertices:\n"
	"	movem.l	d2-d7/a2,-(sp)\n"
	"	move.l	32(sp),a1\n"
	"	move.l	36(sp),a0\n"
	"	move.l	40(sp),d1\n"
	"	beq.s	2f\n"
	"1:	movem.l	(a0)+,d0/d2-d7/a2\n"
	"	movem.l	d0/d2-d7/a2,(a1)\n"
	"	lea	32(a1),a1\n"
	"	movem.l	(a0)+,d0/d2-d7/a2\n"
	"	movem.l	d0/d2-d7/a2,(a1)\n"
	"	lea	32(a1),a1\n"
	"	subq.l	#1,d1\n"
	"	bne.s	1b\n"
	"2:	movem.l	(sp)+,d2-d7/a2\n"
	"	rts\n"
);

/* The six arguments shared by DRAW_BEGIN and DRAW. The texture is only
 * looked at when texturing is on, as before: applications may leave the
 * pointer unset otherwise. */
static ULONG *drawArguments(ULONG *w, W3D_Context* context, ULONG primitive, W3D_Texture* tex) {
	if (!(context->state & W3D_TEXMAPPING)) tex = NULL;
	w[0] = primitive;
	w[1] = context->state;
	w[2] = tex != NULL;
	w[3] = tex ? ((Texture*) tex->driver)->glID : 0;
	w[4] = tex ? tex->texwidth : 0;
	w[5] = tex ? tex->texheight : 0;
	return w + 6;
}

/* count vertices stored one after the other (v) or given by pointers (list). */
static void draw(W3D_Context* context, ULONG primitive, W3D_Texture* tex, W3D_Vertex* v, W3D_Vertex** list, int count) {
	ULONG *w;
	int n, i;
	if (count < 0) count = 0;
	if (count <= QT_W3D_MAX_VERTICES) {
		w = drawArguments(w3d_command(context, QT_W3D_DRAW, 7 + count * QT_W3D_VERTEX_WORDS), context, primitive, tex);
		*w++ = count;
		if (v) qt_copy_vertices(w, v, count);
		else for (i = 0; i < count; ++i, w += QT_W3D_VERTEX_WORDS) qt_copy_vertices(w, list[i], 1);
		return;
	}
	drawArguments(w3d_command(context, QT_W3D_DRAW_BEGIN, 6), context, primitive, tex);
	while (count > 0) {
		n = count < QT_W3D_MAX_VERTICES ? count : QT_W3D_MAX_VERTICES;
		w = w3d_command(context, QT_W3D_VERTICES, 1 + n * QT_W3D_VERTEX_WORDS);
		*w++ = n;
		if (v) {
			qt_copy_vertices(w, v, n);
			v += n;
		}
		else for (i = 0; i < n; ++i, w += QT_W3D_VERTEX_WORDS) qt_copy_vertices(w, *list++, 1);
		count -= n;
	}
	w3d_command(context, QT_W3D_DRAW_END, 0);
}

void drawPrimitive(W3D_Context* context, ULONG primitive, W3D_Texture* tex, W3D_Vertex* v, int count) {
	draw(context, primitive, tex, v, NULL, count);
}

void drawPrimitiveList(W3D_Context* context, ULONG primitive, W3D_Texture* tex, W3D_Vertex** v, int count) {
	draw(context, primitive, tex, NULL, v, count);
}
