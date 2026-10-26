#include "w3d.h"

#include <proto/graphics.h>
#include <proto/picasso96.h>
/*
typedef struct
{
    ULONG ModeID;               // ModeID for OpenScreen
    ULONG Width, Height;        // Width and Height for visible screen
    ULONG Depth;                // Pixel Depth
    char DisplayName[DISPLAYNAMELEN];   // readable name of this display mode 
    W3D_Driver *Driver PACKED;  // Matching driver for this mode
    void *Next PACKED;          // Next W3D_ScreenMode in list
}
W3D_ScreenMode;

struct P96Mode {
	struct Node	Node;
	char			Description[MODENAMELENGTH];
	UWORD			Width;
	UWORD			Height;
	UWORD			Depth;
	ULONG			DisplayID;
};
*/
/* Picasso96's modes of 15 bits or more (the ones the host can write frames
 * into, phase 8), as a NULL-terminated list; NULL if there is none. The
 * list used to end in an extra, uninitialised entry. */
W3D_ScreenMode *W3D_GetScreenmodeList(void) {
	struct List *ml;
	W3D_ScreenMode *first = NULL, **link = &first;
	LOG;
	if ((ml = p96AllocModeListTags(P96MA_MinDepth, 15, TAG_DONE))) {
		struct P96Mode *mn;
		for (mn = (struct P96Mode *) ml->lh_Head; mn->Node.ln_Succ; mn = (struct P96Mode *) mn->Node.ln_Succ) {
			W3D_ScreenMode *sm = calloc(1, sizeof(W3D_ScreenMode));
			int i;
			if (!sm) break;
			sm->ModeID = mn->DisplayID;
			sm->Width = mn->Width;
			sm->Height = mn->Height;
			sm->Depth = mn->Depth;
			sm->Driver = drivers[0];
			for (i = 0; i < MODENAMELENGTH && i < DISPLAYNAMELEN - 1 && mn->Description[i]; ++i) sm->DisplayName[i] = mn->Description[i];
			*link = sm;
			link = (W3D_ScreenMode **) &sm->Next;
		}
		p96FreeModeList(ml);
	}
	return first;
}

void W3D_FreeScreenmodeList(__REGA0(W3D_ScreenMode *list)) {
	W3D_ScreenMode* current;
	W3D_ScreenMode* next = list;
	LOG;
	while (next) {
		current = next;
		next = current->Next;
		free(current);
	}
}

/* The Warp3D tags as Picasso96's, in a copy (the caller's list stays as it
 * is). A board whose true colour modes are all 32-bit (WinUAE's uaegfx) has
 * none of depth 24 for Picasso96, so 24 is tried as 32 too. */
ULONG W3D_BestModeID(__REGA0(struct TagItem *tags)) {
	struct TagItem copy[16];
	ULONG mode;
	int i, n = 0, depth = -1;
	LOG;
	for (i = 0; tags && tags[i].ti_Tag != TAG_DONE && n < 15; ++i) {
		switch (tags[i].ti_Tag) {
		case W3D_BMI_DRIVER: continue;
		case W3D_BMI_WIDTH: copy[n].ti_Tag = P96BIDTAG_NominalWidth; break;
		case W3D_BMI_HEIGHT: copy[n].ti_Tag = P96BIDTAG_NominalHeight; break;
		case W3D_BMI_DEPTH: copy[n].ti_Tag = P96BIDTAG_Depth; depth = n; break;
		default: copy[n].ti_Tag = tags[i].ti_Tag;
		}
		copy[n++].ti_Data = tags[i].ti_Data;
	}
	copy[n].ti_Tag = TAG_DONE;
	mode = p96BestModeIDTagList(copy);
	if (mode == INVALID_ID && depth >= 0 && copy[depth].ti_Data == 24) {
		copy[depth].ti_Data = 32;
		mode = p96BestModeIDTagList(copy);
	}
	return mode;
}
