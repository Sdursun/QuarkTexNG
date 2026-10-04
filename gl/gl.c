#include "gl.h"

/*
 * Bridge to the host side (QuarkTex.alib and opengl32.dll) through the
 * WinUAE native call trap at 0xF0FFC0:
 *   d0 = 100  open DLL        (a0 = name)                    -> d0 handle
 *   d0 = 101  find function   (d1 = handle, a0 = name)       -> d0 function
 *   d0 = 102  call function   (a0 = function, d1-d7/a1-a5 = arguments) -> d0
 *   d0 = 103  close DLL       (d1 = handle)
 *   d0 = 105  memory offset of the Amiga address space on the host
 */

ULONG qt_trap(ULONG d0, ULONG d1, ULONG a0);
ULONG qt_call(ULONG func, const ULONG regs[12]);

__asm__(
	"	.text\n"
	"	.even\n"
	"	.globl	_qt_trap\n"
	"_qt_trap:\n"
	"	movem.l	d2-d7/a2-a6,-(sp)\n"
	"	move.l	48(sp),d0\n"
	"	move.l	52(sp),d1\n"
	"	move.l	56(sp),a0\n"
	"	jsr	0xf0ffc0\n"
	"	movem.l	(sp)+,d2-d7/a2-a6\n"
	"	rts\n"
	"	.globl	_qt_call\n"
	"_qt_call:\n"
	"	movem.l	d2-d7/a2-a6,-(sp)\n"
	"	move.l	52(sp),a0\n"
	"	movem.l	(a0),d1-d7/a1-a5\n"
	"	move.l	48(sp),a0\n"
	"	moveq	#102,d0\n"
	"	jsr	0xf0ffc0\n"
	"	movem.l	(sp)+,d2-d7/a2-a6\n"
	"	rts\n"
);

static inline ULONG qt_f2l(float f) {
	union { float f; ULONG l; } u;
	u.f = f;
	return u.l;
}

/* The host is little endian: the low word of a double comes first. */
static inline ULONG qt_dlo(double d) {
	union { double d; ULONG l[2]; } u;
	u.d = d;
	return u.l[1];
}

static inline ULONG qt_dhi(double d) {
	union { double d; ULONG l[2]; } u;
	u.d = d;
	return u.l[0];
}

static ULONG
#include "glstatichandles.auto.c"
qt_create, qt_move, qt_free, qt_swap, qt_log;
static ULONG w3d, gl;
long memoffset;
char *bp, b;
int i;

static long memOffset(void) {
	return (long) qt_trap(105, 0, 0);
}

static ULONG DLLopen(char* dll) {
	return qt_trap(100, 0, (ULONG) dll);
}

static ULONG DLLfunc(ULONG dll, char* func) {
	return qt_trap(101, dll, (ULONG) func);
}

static ULONG DLLclose(ULONG dll) {
	return qt_trap(103, dll, 0);
}

void glInit(void) {
	memoffset = memOffset();
	w3d = DLLopen("alib\\QuarkTex.alib");
	if (!w3d) w3d = DLLopen("winuae_dll\\QuarkTex.alib");
	if (!w3d) w3d = DLLopen("QuarkTex.alib");

	qt_create = DLLfunc(w3d, "createContext");
	qt_move = DLLfunc(w3d, "moveWindow");
	qt_free = DLLfunc(w3d, "freeContext");
	qt_swap = DLLfunc(w3d, "swapBuffers");
	qt_log = DLLfunc(w3d, "logString");
	gl = DLLopen("opengl32.dll");

	#include "glDLLfunc.auto.c"
}

void glExit(void) {
	DLLclose(gl);
	DLLclose(w3d);
}

void createContext(int left, int top, int width, int height) {
	ULONG r[12] = {(ULONG) left, (ULONG) top, (ULONG) width, (ULONG) height};
	qt_call(qt_create, r);
}

void moveWindow(int left, int top, int width, int height) {
	ULONG r[12] = {(ULONG) left, (ULONG) top, (ULONG) width, (ULONG) height};
	qt_call(qt_move, r);
}

void freeContext(void) {
	ULONG r[12] = {0};
	qt_call(qt_free, r);
}

void swapBuffers(void) {
	ULONG r[12] = {0};
	qt_call(qt_swap, r);
}

void logString(char* c) {
	ULONG r[12] = {(ULONG) c};
	qt_call(qt_log, r);
}

#include "gldefinitions.auto.c"
