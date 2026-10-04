/*
 * The NDK of m68k-amigaos-gcc ships this header as proto/Picasso96.h.
 * Its contents are repeated here because on case-insensitive file systems
 * (Windows, macOS) an #include of that name would find this file again.
 */
#ifndef QUARKTEX_PROTO_PICASSO96_H
#define QUARKTEX_PROTO_PICASSO96_H
#include <exec/types.h>
#include <libraries/Picasso96.h>
#include <clib/Picasso96_protos.h>
extern struct Library *P96Base;
#include <inline/Picasso96.h>
#endif
