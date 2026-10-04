/*
 * Amiga side of the uaenative.library probe: opens the qtprobe host library,
 * passes a string, a reply buffer and a 1 MB block in fast RAM, and checks
 * the answers.
 */
#include <stdio.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <inline/macros.h>

struct Library *UniBase;

/* uaenative.library 1.0 (uaenative.cpp in UAE) */
#define uni_open_library(name, min_version) \
	LP2(30, ULONG, uni_open_library, const char *, name, a1, ULONG, min_version, d0, , UniBase)
#define uni_close_library(library) \
	LP1NR(36, uni_close_library, ULONG, library, a1, , UniBase)
#define uni_get_function(library, name) \
	LP2(42, ULONG, uni_get_function, ULONG, library, a0, const char *, name, a1, , UniBase)

/* call_function takes the function in a0 and passes d1-d7/a1-a5 on. */
static ULONG uni_call(ULONG function, ULONG d1, ULONG d2, ULONG a1, ULONG a2) {
	register ULONG _d0 __asm("d0");
	register ULONG _a0 __asm("a0") = function;
	register ULONG _d1 __asm("d1") = d1;
	register ULONG _d2 __asm("d2") = d2;
	register ULONG _a1 __asm("a1") = a1;
	register ULONG _a2 __asm("a2") = a2;
	register struct Library *_a6 __asm("a6") = UniBase;
	__asm volatile ("jsr a6@(-48:W)"
		: "=r" (_d0), "+r" (_a0), "+r" (_d1), "+r" (_a1)
		: "r" (_d2), "r" (_a2), "r" (_a6)
		: "cc", "memory");
	return _d0;
}

#define BLOCK (1024 * 1024)

int main(void) {
	static char reply[256];
	ULONG library, probe, sum, result, expected, i;
	UBYTE *block;
	int failed = 0;

	UniBase = OpenLibrary("uaenative.library", 1);
	if (!UniBase) {
		printf("FAIL: uaenative.library not available (native_code=true?)\n");
		return 20;
	}
	library = uni_open_library("qtprobe", 0);
	if (!(library & 0x80000000)) {
		printf("FAIL: open_library qtprobe = %08lx\n", (unsigned long) library);
		CloseLibrary(UniBase);
		return 20;
	}
	probe = uni_get_function(library, "qt_probe");
	sum = uni_get_function(library, "qt_sum");
	if (!(probe & 0x80000000) || !(sum & 0x80000000)) {
		printf("FAIL: get_function qt_probe = %08lx, qt_sum = %08lx\n", (unsigned long) probe, (unsigned long) sum);
		failed = 1;
	}
	else {
		result = uni_call(probe, 41, sizeof(reply), (ULONG) "hello from the Amiga", (ULONG) reply);
		printf("qt_probe returned %lu (%s)\n", (unsigned long) result, result == 42 ? "OK" : "FAIL");
		printf("reply: %s\n", reply);
		failed |= result != 42;

		block = AllocVec(BLOCK, MEMF_FAST);
		if (block) {
			expected = 0;
			for (i = 0; i < BLOCK; ++i) {
				block[i] = (UBYTE) (i * 7 + (i >> 9));
				expected += block[i];
			}
			result = uni_call(sum, BLOCK, 0, (ULONG) block, 0);
			printf("qt_sum over 1 MB fast RAM at %08lx: %08lx, expected %08lx (%s)\n",
				(unsigned long) block, (unsigned long) result, (unsigned long) expected, result == expected ? "OK" : "FAIL");
			failed |= result != expected;
			FreeVec(block);
		}
		else printf("SKIP: no 1 MB of fast RAM\n");
	}
	uni_close_library(library);
	CloseLibrary(UniBase);
	printf("%s\n", failed ? "PROBE FAILED" : "PROBE OK");
	return failed ? 10 : 0;
}
