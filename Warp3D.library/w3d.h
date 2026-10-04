#ifndef UAE_W3D
#define UAE_W3D

#include <stdlib.h>
#include <exec/libraries.h>
#include <proto/asl.h>

#include "Warp3D.h"
#include "../gl/gl.h"
#include "../gl/w3dcmd.h"

/* Starts a Warp3D command with the given number of argument words in the
 * command buffer and returns where the arguments go. */
static inline ULONG *w3d_command(ULONG opcode, ULONG words) {
	ULONG *w = qt_reserve(words + 1);
	w[0] = (opcode << 16) | (words + 1);
	return w + 1;
}

typedef struct {
	GLuint glID;
	ULONG envparam;
	W3D_Color envcolor;
	ULONG MinFilter, MagFilter;
	ULONG s_mode, t_mode;
	W3D_Color bordercolor;
} Texture;

void drawVertex(W3D_Context* context, W3D_Vertex* v, W3D_Texture* tex);
void bindTexture(W3D_Texture* tex);

/* Drawing on the host (w3d.c): count vertices stored one after the other,
 * or given by an array of pointers. */
void drawPrimitive(W3D_Context* context, ULONG primitive, W3D_Texture* tex, W3D_Vertex* v, int count);
void drawPrimitiveList(W3D_Context* context, ULONG primitive, W3D_Texture* tex, W3D_Vertex** v, int count);

extern W3D_Driver driver;
extern W3D_Driver *drivers[];

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *P96Base;

extern char *bp, b;
extern int i;

extern int fullscreen;
extern int width, height;

#endif