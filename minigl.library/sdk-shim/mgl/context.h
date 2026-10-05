/*
 * Stands in for mgl/context.h of MiniGL's SDK, which describes MiniGL's own
 * context and includes backend headers the SDK does not ship. Applications
 * and minigl_dispatch.h only need the public types below; QuarkTex's
 * context is its own (minigl.library/mgl.h).
 */
#ifndef __CONTEXT_H
#define __CONTEXT_H

#include "mgl/matrix.h"
#include "mgl/config.h"
#include "mgl/vertexbuffer.h"

struct Window;
struct BitMap;

struct GLcontext_t;
typedef struct GLcontext_t *GLcontext;
typedef void (*DrawFn)(struct GLcontext_t *);

typedef enum {
	MGLKEY_F1, MGLKEY_F2, MGLKEY_F3, MGLKEY_F4, MGLKEY_F5, MGLKEY_F6, MGLKEY_F7, MGLKEY_F8,
	MGLKEY_F9, MGLKEY_F10,
	MGLKEY_CUP, MGLKEY_CDOWN, MGLKEY_CLEFT, MGLKEY_CRIGHT
} MGLspecial;

typedef void (*KeyHandlerFn)(char key);
typedef void (*SpecialHandlerFn)(MGLspecial special_key);
typedef void (*MouseHandlerFn)(GLint x, GLint y, GLbitfield buttons);
typedef void (*IdleFn)(void);

#endif
