/*
 * StormC compatibility for m68k-amigaos-gcc (bebbo).
 * Force-included into every Amiga source file by the Makefile.
 */
#ifndef QUARKTEX_COMPAT_H
#define QUARKTEX_COMPAT_H

/* Register arguments of library entry points (StormC macros). */
#define __REGD0(x) x __asm("d0")
#define __REGD1(x) x __asm("d1")
#define __REGD2(x) x __asm("d2")
#define __REGD3(x) x __asm("d3")
#define __REGD4(x) x __asm("d4")
#define __REGD5(x) x __asm("d5")
#define __REGD6(x) x __asm("d6")
#define __REGD7(x) x __asm("d7")
#define __REGA0(x) x __asm("a0")
#define __REGA1(x) x __asm("a1")
#define __REGA2(x) x __asm("a2")
#define __REGA3(x) x __asm("a3")
#define __REGA4(x) x __asm("a4")
#define __REGA5(x) x __asm("a5")
#define __REGA6(x) x __asm("a6")

#ifndef __FUNC__
#define __FUNC__ __func__
#endif

#endif
