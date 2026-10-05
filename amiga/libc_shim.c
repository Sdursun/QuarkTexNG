/*
 * The libraries are linked without a C runtime (there is no process to
 * initialise it), so malloc/free come straight from exec.
 */
#include <stddef.h>
#include <exec/memory.h>
#include <proto/exec.h>

void *malloc(size_t size) {
	return AllocVec(size, MEMF_ANY);
}

/* GCC turns malloc followed by memset to 0 into calloc. */
void *calloc(size_t count, size_t size) {
	return AllocVec(count * size, MEMF_ANY | MEMF_CLEAR);
}

void free(void *ptr) {
	if (ptr) FreeVec(ptr);
}

/* GCC emits calls to these for structure copies and initialisers. */
void *memcpy(void *dest, const void *src, size_t n) {
	CopyMem((APTR) src, dest, n);
	return dest;
}

/* Without the attribute GCC turns the loop back into a call to memset. */
__attribute__((optimize("no-tree-loop-distribute-patterns")))
void *memset(void *dest, int c, size_t n) {
	unsigned char *p = dest;
	while (n--) *p++ = (unsigned char) c;
	return dest;
}
