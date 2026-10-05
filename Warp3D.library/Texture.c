#include "w3d.h"

/*
struct Node {
    struct  Node *ln_Succ;	// Pointer to next (successor)
    struct  Node *ln_Pred;	// Pointer to previous (predecessor)
    UBYTE   ln_Type;
    BYTE    ln_Pri;		// Priority, for sorting
    char    *ln_Name;		// ID string, null terminated
};
*/


/* Stored in the Texture structure and passed to the host as they are. The
 * texture formats are mapped on the host (host/w3d.cpp). */
long filter[] = {0, GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR};
long wrap[] = {0, GL_REPEAT, GL_CLAMP };

/* Deletes the OpenGL texture on the host. */
static void freeTexture(W3D_Context *context, W3D_Texture *texture) {
	ULONG *w = w3d_command(context, QT_W3D_TEX_FREE, 1);
	w[0] = ((Texture*) texture->driver)->glID;
}

W3D_Texture *W3D_AllocTexObj(__REGA0(W3D_Context *context), __REGA1(ULONG *error), __REGA2(struct TagItem *ATOTags)) {
	W3D_Texture *tex;
	LOG;
	tex = (W3D_Texture*) malloc(sizeof(W3D_Texture));
	if (!tex) {
		if (error) *error = W3D_NOMEMORY;
		return NULL;
	}
	/* All of it: the fields below were set one by one, but mipmaps[0-15]
	 * kept what malloc left (and mipmaps[16] was written past the array).
	 * Applications and MiniGL may read them. */
	memset(tex, 0, sizeof(W3D_Texture));

	if (!QT(context)->textures) {
		QT(context)->textures = (struct Node*) tex;
		tex->link.ln_Pred = NULL;
		tex->link.ln_Succ = NULL;
	}
	else {
		tex->link.ln_Pred = NULL;
		tex->link.ln_Succ = QT(context)->textures;
		QT(context)->textures->ln_Pred = &tex->link;
		QT(context)->textures = &tex->link;
	}
	tex->resident = W3D_TRUE;
	tex->mipmap = W3D_FALSE;
	tex->dirty = W3D_FALSE;
	tex->matchfmt = W3D_TRUE;
	tex->reserved1 = W3D_TRUE;
	tex->reserved2 = W3D_TRUE;
	tex->mipmapmask = 0;
	tex->texsource = NULL;
	tex->texfmtsrc = 0;
	tex->palette = NULL;
	tex->texdata = NULL;
	tex->texdest = NULL;
	tex->texdestsize = 0;
	tex->texwidth = 0;
	tex->texwidthexp = 0;
	tex->texheight = 0;
	tex->texheightexp = 0;
	tex->bytesperpix = 0;
	tex->bytesperrow = 0;

	for (; ATOTags->ti_Tag != TAG_DONE; ++ATOTags) {
		switch (ATOTags->ti_Tag) {
		case W3D_ATO_IMAGE:
			tex->texsource = (void*) ATOTags->ti_Data;
			break;
		case W3D_ATO_FORMAT:
			tex->texfmtsrc = ATOTags->ti_Data;
			break;
		case W3D_ATO_WIDTH:
			tex->texwidth = ATOTags->ti_Data;
			break;
		case W3D_ATO_HEIGHT:
			tex->texheight = ATOTags->ti_Data;
			break;
		case W3D_ATO_PALETTE:
			tex->palette = (ULONG*) ATOTags->ti_Data;
			break;
		//W3D_ATO_MIPMAPPTRS
		}
	}
	tex->driver = malloc(sizeof(Texture));

	((Texture*) tex->driver)->envparam = 0;
	((Texture*) tex->driver)->envcolor.r = 0; ((Texture*) tex->driver)->envcolor.g = 0; ((Texture*) tex->driver)->envcolor.b = 0; ((Texture*) tex->driver)->envcolor.a = 0;
	((Texture*) tex->driver)->MinFilter = 0; ((Texture*) tex->driver)->MagFilter = 0;
	((Texture*) tex->driver)->s_mode = 0; ((Texture*) tex->driver)->t_mode = 0;
	((Texture*) tex->driver)->bordercolor.r = 0; ((Texture*) tex->driver)->bordercolor.g = 0; ((Texture*) tex->driver)->bordercolor.b = 0; ((Texture*) tex->driver)->bordercolor.a = 0;
	/* The host creates the texture and returns its OpenGL name (host/w3d.cpp). */
	{
		ULONG *w = w3d_command(context, QT_W3D_TEX_ALLOC, 5);
		w[0] = tex->texfmtsrc;
		w[1] = tex->texwidth;
		w[2] = tex->texheight;
		w[3] = (ULONG) tex->texsource;
		w[4] = (ULONG) tex->palette;
		((Texture*) tex->driver)->glID = qt_flush();
	}
	if (error) *error = W3D_SUCCESS;
	return tex;
}

void W3D_FreeTexObj(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture)) {
	LOG;
	if (texture->link.ln_Pred == NULL) { QT(context)->textures = texture->link.ln_Succ; if (QT(context)->textures != NULL) QT(context)->textures->ln_Pred = NULL; }
	if (texture->link.ln_Succ == NULL && texture->link.ln_Pred != NULL) texture->link.ln_Pred->ln_Succ = NULL;
	if (texture->link.ln_Pred != NULL && texture->link.ln_Succ != NULL) {
		(texture->link.ln_Pred)->ln_Succ = texture->link.ln_Succ;
		(texture->link.ln_Succ)->ln_Pred = texture->link.ln_Pred;
	}
	freeTexture(context, texture);
	free(texture->driver);
	free(texture);
}

void W3D_ReleaseTexture(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture)) {
	//free from video RAM
	LOG;
}

void W3D_FlushTextures(__REGA0(W3D_Context *context)) {
	//Release all textures from video ram
	LOG;
}

ULONG W3D_SetFilter(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGD0(ULONG MinFilter), __REGD1(ULONG MagFilter)) {
	ULONG *w;
	LOG;
	((Texture*) texture->driver)->MinFilter = filter[MinFilter];
	((Texture*) texture->driver)->MagFilter = filter[MagFilter];
	w = w3d_command(context, QT_W3D_TEX_FILTER, 3);
	w[0] = ((Texture*) texture->driver)->glID;
	w[1] = ((Texture*) texture->driver)->MinFilter;
	w[2] = ((Texture*) texture->driver)->MagFilter;
	return W3D_SUCCESS;
}

static inline ULONG f2l(float f) {
	union { float f; ULONG l; } u;
	u.f = f;
	return u.l;
}

ULONG W3D_SetTexEnv(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGD1(ULONG envparam), __REGA2(W3D_Color *envcolor)) {
	Texture *t = (Texture*) texture->driver;
	ULONG *w;
	LOG;
	t->envparam = envparam;
	t->envcolor = *envcolor;
	w = w3d_command(context, QT_W3D_TEX_ENV, 6);
	w[0] = t->glID;
	w[1] = envparam;
	w[2] = f2l(t->envcolor.r);
	w[3] = f2l(t->envcolor.g);
	w[4] = f2l(t->envcolor.b);
	w[5] = f2l(t->envcolor.a);
	return W3D_SUCCESS;
}

ULONG W3D_SetWrapMode(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGD0(ULONG s_mode), __REGD1(ULONG t_mode), __REGA2(W3D_Color *bordercolor)) {
	Texture *t = (Texture*) texture->driver;
	ULONG *w;
	LOG;
	t->s_mode = wrap[s_mode];
	t->t_mode = wrap[t_mode];
	t->bordercolor = *bordercolor;
	w = w3d_command(context, QT_W3D_TEX_WRAP, 7);
	w[0] = t->glID;
	w[1] = t->s_mode;
	w[2] = t->t_mode;
	w[3] = f2l(t->bordercolor.r);
	w[4] = f2l(t->bordercolor.g);
	w[5] = f2l(t->bordercolor.b);
	w[6] = f2l(t->bordercolor.a);
	return W3D_SUCCESS;
}

/* The host reads the image right away (synchronous command). bytesPerRow 0
 * means rows of width pixels without gaps. */
static void updateTexture(W3D_Context *context, W3D_Texture *texture, void *image, ULONG x, ULONG y, ULONG width, ULONG height, ULONG bytesPerRow) {
	ULONG *w = w3d_command(context, QT_W3D_TEX_UPDATE, 9);
	w[0] = ((Texture*) texture->driver)->glID;
	w[1] = texture->texfmtsrc;
	w[2] = x;
	w[3] = y;
	w[4] = width;
	w[5] = height;
	w[6] = (ULONG) image;
	w[7] = bytesPerRow;
	w[8] = (ULONG) texture->palette;
	qt_flush();
}

ULONG W3D_UpdateTexImage(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGA2(void *teximage), __REGD1(int level), __REGA3(ULONG *palette)) {
	LOG;
	texture->texsource = teximage;
	if (palette) texture->palette = palette;
	updateTexture(context, texture, teximage, 0, 0, texture->texwidth, texture->texheight, 0);
	return W3D_SUCCESS;
}

/* teximage holds the rectangle only, srcbpr bytes per row (0: packed). 0.53
 * uploaded texsource instead and ignored calls with srcbpr != 0. */
ULONG W3D_UpdateTexSubImage(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGA2(void *teximage), __REGD1(ULONG level), __REGA3(ULONG *palette), __REGA4(W3D_Scissor* scissor), __REGD0(ULONG srcbpr)) {
	LOG;
	if (palette) texture->palette = palette;
	updateTexture(context, texture, teximage, scissor->left, scissor->top, scissor->width, scissor->height, srcbpr);
	return W3D_SUCCESS;
}

ULONG W3D_UploadTexture(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture)) {
	LOG;
	return W3D_SUCCESS;
}

/* 0.53 stepped to the next node before freeing, so it freed every node but
 * the first, and finally the driver data of NULL. */
ULONG W3D_FreeAllTexObj(__REGA0(W3D_Context *context)) {
	W3D_Texture *n, *next;
	LOG;
	for (n = (W3D_Texture*) QT(context)->textures; n != NULL; n = next) {
		next = (W3D_Texture*) n->link.ln_Succ;
		freeTexture(context, n);
		free(n->driver);
		free(n);
	}
	QT(context)->textures = NULL;
	return W3D_SUCCESS;
}

/* The bounds are ARGB like the other Warp3D colours; the host compares red,
 * green and blue (0.53 did not support the chroma test). */
ULONG W3D_SetChromaTestBounds(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGD0(ULONG rgba_lower), __REGD1(ULONG rgba_upper), __REGD2(ULONG mode)) {
	ULONG *w;
	LOG;
	if (!texture || mode < W3D_CHROMATEST_NONE || mode > W3D_CHROMATEST_EXCLUSIVE) return W3D_ILLEGALINPUT;
	w = w3d_command(context, QT_W3D_CHROMA, 4);
	w[0] = ((Texture*) texture->driver)->glID;
	w[1] = rgba_lower;
	w[2] = rgba_upper;
	w[3] = mode;
	return W3D_SUCCESS;
}

/*
long swap[] = {0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0};
long formats[] = {0, GL_RGB, GL_BGRA, GL_RGB, GL_RGB, GL_BGRA, GL_BGRA, GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_INTENSITY, GL_RGBA};
long types[] = {0, GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_4_4_4_4_REV,
		GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE};


W3D_Texture *W3D_AllocTexObj(__REGA0(W3D_Context *context), __REGA1(ULONG *error), __REGA2(struct TagItem *ATOTags)) {
	W3D_Texture *tex;
	tex = (W3D_Texture*) malloc(sizeof(W3D_Texture));

	tex->resident = W3D_TRUE;
	tex->mipmap = W3D_FALSE;
	tex->dirty = W3D_FALSE;
	tex->matchfmt = W3D_TRUE;
	tex->reserved1 = W3D_TRUE;
	tex->reserved2 = W3D_TRUE;
	tex->mipmapmask = 0;
	tex->texsource = NULL;
	tex->mipmaps[16] = NULL;
	tex->texfmtsrc = 0;
	tex->palette = NULL;
	tex->texdata = NULL;
	tex->texdest = NULL;
	tex->texdestsize = 0;
	tex->texwidth = 0;
	tex->texwidthexp = 0;
	tex->texheight = 0;
	tex->texheightexp = 0;
	tex->bytesperpix = 0;
	tex->bytesperrow = 0;
	tex->driver = NULL;

	for (; ATOTags->ti_Tag != TAG_DONE; ++ATOTags) {
		switch (ATOTags->ti_Tag) {
		case W3D_ATO_IMAGE:
			tex->texsource = (void*) ATOTags->ti_Data;
			break;
		case W3D_ATO_FORMAT:
			tex->texfmtsrc = ATOTags->ti_Data;
			break;
		case W3D_ATO_WIDTH:
			tex->texwidth = ATOTags->ti_Data;
			break;
		case W3D_ATO_HEIGHT:
			tex->texheight = ATOTags->ti_Data;
			break;
		}
	}
	_glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	if (swap[tex->texfmtsrc]) _glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
	else _glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	tex->mipmapmask = loadTexture(tex->texsource, tex->texwidth, tex->texheight, formats[tex->texfmtsrc], types[tex->texfmtsrc]);
	if (error) *error = W3D_SUCCESS;
	return tex;
}
void W3D_FreeTexObj(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture)) {
	if (texture->mipmapmask) deleteTexture((long)texture->mipmapmask);
	//if (texture->texsource) free(texture->texsource);
	free(texture);
}
void W3D_ReleaseTexture(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture)) {
	
}
void            W3D_FlushTextures(__REGA0(W3D_Context *context)) { }
ULONG           W3D_SetFilter(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGD0(ULONG MinFilter), __REGD1(ULONG MagFilter)) { return W3D_SUCCESS; }
ULONG W3D_SetTexEnv(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGD1(ULONG envparam), __REGA2(W3D_Color *envcolor)) {
	W3D_Color *color = (W3D_Color*) malloc(sizeof(W3D_Color));
	*color = *envcolor;
	texture->bytesperpix = envparam;
	texture->driver = color;
	return W3D_SUCCESS;
}
ULONG           W3D_SetWrapMode(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGD0(ULONG s_mode), __REGD1(ULONG t_mode), __REGA2(W3D_Color *bordercolor)) { return W3D_SUCCESS; }
ULONG W3D_UpdateTexImage(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGA2(void *teximage), __REGD1(int level), __REGA3(ULONG *palette)) {
	deleteTexture((long)texture->mipmapmask);
	texture->mipmapmask = loadTexture(texture->texsource, texture->texwidth, texture->texheight, formats[texture->texfmtsrc], types[texture->texfmtsrc]);
	return W3D_SUCCESS;
}
ULONG           W3D_UpdateTexSubImage(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGA2(void *teximage), __REGD1(ULONG level), __REGA3(ULONG *palette), __REGA4(W3D_Scissor* scissor), __REGD0(ULONG srcbpr)) { return W3D_SUCCESS; }
ULONG           W3D_UploadTexture(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture)) { return W3D_SUCCESS; }
ULONG           W3D_FreeAllTexObj(__REGA0(W3D_Context *context)) { return W3D_SUCCESS; }
ULONG           W3D_SetChromaTestBounds(__REGA0(W3D_Context *context), __REGA1(W3D_Texture *texture), __REGD0(ULONG rgba_lower), __REGD1(ULONG rgba_upper), __REGD2(ULONG mode)) { return W3D_SUCCESS; }
*/