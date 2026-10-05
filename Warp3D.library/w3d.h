#ifndef UAE_W3D
#define UAE_W3D

#include <stdlib.h>
#include <string.h>
#include <exec/libraries.h>
#include <proto/asl.h>

#include "Warp3D.h"
#include "../gl/gl.h"
#include "../gl/w3dcmd.h"

/* What QuarkTex keeps per context. W3D_CreateContext allocates it; the
 * application sees its first member. */
typedef struct QtContext {
	W3D_Context context;
	struct QtContext *next; /* the library's contexts (Context.c) */
	ULONG host;             /* host context id (gl/gl.h) */
	struct Window *window;  /* NULL in fullscreen */
	int fullscreen;
	int left, top, width, height;
	struct Node *textures;  /* allocated W3D_Textures (Texture.c) */
	/* Phase 8: on a Picasso96 bitmap the host draws offscreen and the frame
	 * is written into Amiga display memory (Context.c, w3d_present): into the
	 * window, or in fullscreen into the draw region's bitmap at its y offset.
	 * dirty: something was drawn since the frame was last written. */
	int offscreen, dirty;
	struct BitMap *bitmap;
	int yoffset;
} QtContext;

#define QT(context) ((QtContext*) (context))

/* Sends what follows to the context's host context. */
static inline void w3d_select(W3D_Context *context) {
	selectContext(QT(context)->host);
}

/* Starts a Warp3D command for the context with the given number of argument
 * words in the command buffer and returns where the arguments go. */
static inline ULONG *w3d_command(W3D_Context *context, ULONG opcode, ULONG words) {
	ULONG *w;
	if (opcode == QT_W3D_DRAW_BEGIN || opcode == QT_W3D_DRAW || opcode == QT_W3D_DRAW_ARRAY || opcode == QT_W3D_CLEAR) {
		QT(context)->dirty = 1;
	}
	w3d_select(context);
	w = qt_reserve(words + 1);
	w[0] = (opcode << 16) | (words + 1);
	return w + 1;
}

/* A float as its bit pattern, for a command word. */
static inline ULONG w3d_float(float f) {
	union { float f; ULONG l; } u;
	u.f = f;
	return u.l;
}

typedef struct {
	GLuint glID;
	ULONG envparam;
	W3D_Color envcolor;
	ULONG MinFilter, MagFilter;
	ULONG s_mode, t_mode;
	W3D_Color bordercolor;
} Texture;


/* Drawing on the host (w3d.c): count vertices stored one after the other,
 * or given by an array of pointers. */
void drawPrimitive(W3D_Context* context, ULONG primitive, W3D_Texture* tex, W3D_Vertex* v, int count);
void drawPrimitiveList(W3D_Context* context, ULONG primitive, W3D_Texture* tex, W3D_Vertex** v, int count);

/* Writes an offscreen context's frame into display memory if something was
 * drawn since it was last written (Context.c). */
void w3d_present(QtContext *qt);
/* Fills the W3D_Context fields describing the draw region (Context.c). */
void w3d_describe(W3D_Context *context, struct BitMap *bitmap, int yoffset);

/* Texture.c */
ULONG W3D_FreeAllTexObj(__REGA0(W3D_Context *context));

extern W3D_Driver driver;
extern W3D_Driver *drivers[];

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *P96Base;

extern char *bp, b;
extern int i;


#endif