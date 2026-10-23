#include "mgl.h"
#include <proto/exec.h>

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

/*
 * The dispatch table, per program: MiniGL 29's applications keep their
 * current context in the variable the table's currentContext points to,
 * reading and writing it themselves (MGLD_CTX in <libraries/minigl_
 * dispatch.h>). With one table, two programs running at once would draw with
 * each other's contexts (docs/multitasking.md), so each task that asks for
 * the table gets a copy of its own, pointing to a variable of its own. A
 * task that has ended leaves its copy to the next one; with all of them
 * taken by running programs, the shared table (mgl_dispatch) is the
 * fallback, as before.
 */
#define QT_MGL_CLIENTS 16

typedef struct {
	struct Task *task;
	MGLDispatchTable table;
	GLcontext current;
} Client;

static Client clients[QT_MGL_CLIENTS];
GLcontext mgl_sharedCurrent = NULL;

/* The calling task's client; NULL if it has none (and make: none free). */
static Client *client(int make) {
	struct Task *me = FindTask(NULL);
	Client *c = NULL;
	int i;
	Forbid();
	for (i = 0; i < QT_MGL_CLIENTS && !c; ++i) if (clients[i].task == me) c = &clients[i];
	for (i = 0; make && i < QT_MGL_CLIENTS && !c; ++i) {
		if (!clients[i].task || !qt_alive(clients[i].task)) {
			c = &clients[i];
			c->task = me;
			c->table = mgl_dispatch;
			c->current = NULL;
			c->table.currentContext = &c->current;
		}
	}
	Permit();
	return c;
}

GLcontext *mgl_currentFor(void) {
	Client *c = client(0);
	return c ? &c->current : &mgl_sharedCurrent;
}


void INIT_0_MiniGL(void) {
	glInit();
}

void EXIT_0_MiniGL(void) {
	mgl_gluExit();
	glExit();
}

const MGLDispatchTable *MGLGetDispatchTable(void) {
	Client *c = client(1);
	return c ? &c->table : &mgl_dispatch;
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
