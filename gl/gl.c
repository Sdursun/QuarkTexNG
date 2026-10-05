#include "gl.h"
#include <exec/memory.h>
#include <proto/exec.h>
#include <inline/macros.h>

/*
 * Bridge to the host library (quarktex-windows-x86[-64].dll), reached through
 * uaenative.library, which the emulator provides when native_code=true:
 *   -30 open_library  (a1 = name, d0 = minimum version) -> handle
 *   -36 close_library (a1 = handle)
 *   -42 get_function  (a0 = library, a1 = name)         -> handle
 *   -48 call_function (a0 = function, d1-d7/a1-a5 = arguments) -> d0
 * Valid handles have bit 31 set.
 *
 * OpenGL calls are written to a command buffer (gl/glencode.auto.c, format in
 * gl/glgen.cpp) and executed by qt_execute on the host when the buffer is
 * flushed. Pointers are passed as Amiga addresses, so memoffset stays 0.
 */

/* Must match QT_PROTOCOL_VERSION in host/quarktex.cpp. */
#define QT_PROTOCOL_VERSION 8

#define QT_BUFFER_BYTES (256 * 1024)

ULONG qt_uni_call(struct Library *base, ULONG func, const ULONG regs[12]);

__asm__(
	"	.text\n"
	"	.even\n"
	"	.globl	_qt_uni_call\n"
	"_qt_uni_call:\n"
	"	movem.l	d2-d7/a2-a6,-(sp)\n"
	"	move.l	56(sp),a0\n"
	"	movem.l	(a0),d1-d7/a1-a5\n"
	"	move.l	48(sp),a6\n"
	"	move.l	52(sp),a0\n"
	"	jsr	-48(a6)\n"
	"	movem.l	(sp)+,d2-d7/a2-a6\n"
	"	rts\n"
);

static struct Library *qt_UniBase;

#define uni_open_library(name, min_version) \
	LP2(30, ULONG, uni_open_library, const char *, name, a1, ULONG, min_version, d0, , qt_UniBase)
#define uni_close_library(library) \
	LP1NR(36, uni_close_library, ULONG, library, a1, , qt_UniBase)
#define uni_get_function(library, name) \
	LP2(42, ULONG, uni_get_function, ULONG, library, a0, const char *, name, a1, , qt_UniBase)

#define UNI_VALID(handle) (((handle) & 0x80000000) != 0)

static ULONG qt_host, qt_execute, qt_create, qt_move, qt_free, qt_swap, qt_log;
long memoffset;
char *bp, b;
int i;

static ULONG hostFunction(const char *name) {
	ULONG handle = uni_get_function(qt_host, name);
	return UNI_VALID(handle) ? handle : 0;
}

static ULONG hostCall(ULONG func, ULONG d1, ULONG d2, ULONG d3, ULONG d4, ULONG d5, ULONG a1) {
	ULONG regs[12] = {d1, d2, d3, d4, d5, 0, 0, a1};
	return func ? qt_uni_call(qt_UniBase, func, regs) : 0;
}

/* Opens the host library; leaves the qt_* handles at 0 if that fails. */
static void openHost(void) {
	ULONG version;
	qt_UniBase = OpenLibrary("uaenative.library", 1);
	if (!qt_UniBase) return;
	qt_host = uni_open_library("quarktex", 0);
	if (!UNI_VALID(qt_host)) {
		qt_host = 0;
		return;
	}
	version = hostFunction("qt_protocol_version");
	if (!version || hostCall(version, 0, 0, 0, 0, 0, 0) != QT_PROTOCOL_VERSION) return;
	qt_execute = hostFunction("qt_execute");
	qt_create = hostFunction("qt_create_context");
	qt_move = hostFunction("qt_move_window");
	qt_free = hostFunction("qt_free_context");
	qt_swap = hostFunction("qt_swap_buffers");
	qt_log = hostFunction("qt_log");
}

/* --- Command buffer ------------------------------------------------------ */

static ULONG *qt_buffer;
static ULONG qt_used;
static ULONG qt_scratch[32]; /* takes the commands while there is no buffer */
static ULONG qt_context; /* host id of the context the buffer is for, 0 = none */

/* Executes the buffered commands; returns the result of the last one. */
ULONG qt_flush(void) {
	ULONG bytes = qt_used * 4;
	qt_used = 0;
	if (!bytes || !qt_buffer) return 0;
	return hostCall(qt_execute, bytes, qt_context, 0, 0, 0, (ULONG) qt_buffer);
}

ULONG *qt_reserve(ULONG words) {
	ULONG *w;
	if (!qt_buffer) return qt_scratch;
	if (qt_used + words > QT_BUFFER_BYTES / 4) qt_flush();
	w = qt_buffer + qt_used;
	qt_used += words;
	return w;
}

static inline ULONG qt_f2l(float f) {
	union { float f; ULONG l; } u;
	u.f = f;
	return u.l;
}

static inline ULONG qt_dhi(double d) {
	union { double d; ULONG l[2]; } u;
	u.d = d;
	return u.l[0];
}

static inline ULONG qt_dlo(double d) {
	union { double d; ULONG l[2]; } u;
	u.d = d;
	return u.l[1];
}

#define QT_ADDRESS(p) ((ULONG) (p))

/* --- Library life cycle and context ------------------------------------- */

void glInit(void) {
	openHost();
	if (qt_execute) qt_buffer = AllocVec(QT_BUFFER_BYTES, MEMF_ANY);
	qt_used = 0;
}

void glExit(void) {
	qt_flush();
	if (qt_buffer) FreeVec(qt_buffer);
	if (qt_host) uni_close_library(qt_host);
	if (qt_UniBase) CloseLibrary(qt_UniBase);
	qt_buffer = NULL;
	qt_host = qt_execute = qt_create = qt_move = qt_free = qt_swap = qt_log = 0;
	qt_UniBase = NULL;
}

ULONG createContext(int left, int top, int width, int height, int flags) {
	ULONG id;
	if (!qt_buffer || !qt_create) return 0;
	id = hostCall(qt_create, left, top, width, height, flags, 0);
	if (id) selectContext(id);
	return id;
}

void selectContext(ULONG id) {
	if (id == qt_context) return;
	qt_flush();
	qt_context = id;
}

void moveWindow(int left, int top, int width, int height) {
	qt_flush();
	hostCall(qt_move, left, top, width, height, qt_context, 0);
}

void freeContext(void) {
	qt_flush();
	hostCall(qt_free, qt_context, 0, 0, 0, 0, 0);
	qt_context = 0;
}

void swapBuffers(void) {
	qt_flush();
	hostCall(qt_swap, qt_context, 0, 0, 0, 0, 0);
}

/* c is an Amiga address. */
void logString(char* c) {
	hostCall(qt_log, 0, 0, 0, 0, 0, (ULONG) c);
}

#include "glencode.auto.c"
