#include "mgl.h"

/*
 * The library: one entry point (LVO -30) returning the dispatch table
 * (dispatch.auto.c, generated in the order of MiniGL's minigl_dispatch.h).
 */

extern const MGLDispatchTable mgl_dispatch;

GLcontext mgl_current = NULL;

void INIT_0_MiniGL(void) {
	glInit();
}

void EXIT_0_MiniGL(void) {
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
