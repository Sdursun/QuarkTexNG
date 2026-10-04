/*
 * Library skeleton (resident tag, Open/Close/Expunge) replacing StormC's
 * library_startup.o. Configured by the Makefile:
 *   LIB_NAME, LIB_VERSION, LIB_REVISION, LIB_IDSTRING  identification
 *   LIB_INIT, LIB_EXIT   called once when the library is loaded/expunged
 *   functable.h          QT_FUNC(name) per .fd entry, in LVO order
 */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <proto/exec.h>

/* Running the library from the shell just fails. */
__asm__(
	"	.text\n"
	"	moveq	#-1,d0\n"
	"	rts\n"
);

struct QuarkTexBase {
	struct Library lib;
	BPTR segList;
};

struct ExecBase *SysBase;
struct Library *UtilityBase;

void LIB_INIT(void);
void LIB_EXIT(void);

#define QT_FUNC(name) void name(void);
#define QT_PAD
#include "functable.h"
#undef QT_FUNC
#undef QT_PAD

static const char libName[] = LIB_NAME;
static const char libId[] = LIB_IDSTRING;
const char libVersionString[] __attribute__((used)) = "$VER: " LIB_IDSTRING;

static struct QuarkTexBase *LibOpen(__REGA6(struct QuarkTexBase *base)) {
	base->lib.lib_OpenCnt++;
	base->lib.lib_Flags &= ~LIBF_DELEXP;
	return base;
}

static BPTR LibExpunge(__REGA6(struct QuarkTexBase *base)) {
	BPTR segList;
	if (base->lib.lib_OpenCnt) {
		base->lib.lib_Flags |= LIBF_DELEXP;
		return 0;
	}
	segList = base->segList;
	Remove(&base->lib.lib_Node);
	LIB_EXIT();
	if (UtilityBase) CloseLibrary(UtilityBase);
	FreeMem((UBYTE *) base - base->lib.lib_NegSize, base->lib.lib_NegSize + base->lib.lib_PosSize);
	return segList;
}

static BPTR LibClose(__REGA6(struct QuarkTexBase *base)) {
	if (--base->lib.lib_OpenCnt == 0 && (base->lib.lib_Flags & LIBF_DELEXP)) return LibExpunge(base);
	return 0;
}

static ULONG LibNull(void) {
	return 0;
}

static const APTR LibFuncTable[] = {
	(APTR) LibOpen,
	(APTR) LibClose,
	(APTR) LibExpunge,
	(APTR) LibNull,
#define QT_FUNC(name) (APTR) name,
#define QT_PAD (APTR) LibNull,
#include "functable.h"
#undef QT_FUNC
#undef QT_PAD
	(APTR) -1
};

static struct QuarkTexBase *LibInit(__REGD0(struct QuarkTexBase *base), __REGA0(BPTR segList), __REGA6(struct ExecBase *sysBase)) {
	SysBase = sysBase;
	base->lib.lib_Node.ln_Type = NT_LIBRARY;
	base->lib.lib_Node.ln_Name = (char *) libName;
	base->lib.lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
	base->lib.lib_Version = LIB_VERSION;
	base->lib.lib_Revision = LIB_REVISION;
	base->lib.lib_IdString = (APTR) libId;
	base->segList = segList;
	UtilityBase = OpenLibrary("utility.library", 37);
	LIB_INIT();
	return base;
}

static const ULONG LibInitTable[4] = {
	sizeof(struct QuarkTexBase),
	(ULONG) LibFuncTable,
	0,
	(ULONG) LibInit
};

const struct Resident RomTag __attribute__((used)) = {
	RTC_MATCHWORD,
	(struct Resident *) &RomTag,
	(APTR) (&RomTag + 1),
	RTF_AUTOINIT,
	LIB_VERSION,
	NT_LIBRARY,
	0,
	(char *) libName,
	(char *) libId,
	(APTR) LibInitTable
};
