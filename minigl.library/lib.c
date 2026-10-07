#include "mgl.h"

/*
 * The library: one entry point (LVO -30) returning the dispatch table
 * (dispatch.auto.c, generated in the order of MiniGL's minigl_dispatch.h).
 *
 * Built against MiniGL's 29 SDK (docs/phase7-minigl.md): its GL tokens carry
 * OpenGL's own values, which mgl_enum passes through (and marks MiniGL's
 * private ones, 0x7000-0x7FFF). A client built on the older SDK (27), whose
 * tokens were auto-numbered positions, is not supported: there is no way to
 * tell the two apart at OpenLibrary or GetDispatchTable time (neither passes
 * the client's requested version down to the library -- confirmed against a
 * real OpenLibrary call, whose D0 at the Open() vector is not it), and the
 * 29 SDK's own release notes call this the same break: "the game needs to
 * get recompiled... with the latest SDK".
 */

extern const MGLDispatchTable mgl_dispatch;

GLcontext mgl_current = NULL;

void INIT_0_MiniGL(void) {
	glInit();
}

void EXIT_0_MiniGL(void) {
	mgl_gluExit();
	glExit();
}

const MGLDispatchTable *MGLGetDispatchTable(void) {
	return &mgl_dispatch;
}

/* An entry the library does not implement yet: logged once per name. */
void mgl_missing(const char *name) {
	static const char *logged[64];
	static int count;
	int i;
	for (i = 0; i < count; ++i) if (logged[i] == name) return;
	if (count < 64) logged[count++] = name;
	logString("minigl.library: not implemented:");
	logString((char *) name);
}

/* A call with a constant MiniGL does not know (applications define values of
 * their own for what MiniGL lacks): skipped, as MiniGL skips it; logged once
 * per entry. */
void mgl_unknown(const char *name) {
	static const char *logged[64];
	static int count;
	int i;
	for (i = 0; i < count; ++i) if (logged[i] == name) return;
	if (count < 64) logged[count++] = name;
	logString("minigl.library: skipped a call with an unknown constant:");
	logString((char *) name);
}
