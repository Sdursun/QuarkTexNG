/*
 * m01_minigl: minigl.library's own checks, through MiniGL's shared library
 * interface as an application built on MiniGL's SDK uses it (with MiniGL's
 * numbers for the GL constants). Not one of the reference tests (QuarkTex
 * 0.53 had no minigl.library): it reads its pixels back, prints PASS or FAIL
 * per check and returns 5 if one failed. Run it with tests/run-app.ps1.
 *
 * The four columns of a 320 x 240 window, left to right:
 *  - a paletted 2 x 2 texture (glColorTable, GL_COLOR_INDEX, unpack
 *    alignment 4): red, green / blue, red;
 *  - glInterleavedArrays(GL_T2F_C4UB_V3F) in yellow, texturing off;
 *  - white over grey 0.25 with glBlendEquation(GL_FUNC_SUBTRACT): 0.75;
 *  - two texture units modulated, coordinates from arrays of each unit:
 *    (255, 128, 0) x (128, 255, 255) = (128, 128, 0).
 *
 * After the last frame glFinish has it written into the display (phase 8),
 * and the same points are checked there, through Picasso96.
 *
 * Built only when MiniGL's SDK headers are there (tests/Makefile).
 */
#include <libraries/minigl_dispatch.h>
#include <libraries/minigl.h>
#include <proto/exec.h>
#include <proto/Picasso96.h>
#include <intuition/intuition.h>
#include <stdio.h>

#define GL_COLOR_INDEX8_EXT 0x80E5 /* as applications define it */

struct Library *MiniGLBase;
struct Library *P96Base;
const MGLDispatchTable *MiniGLDispatch;

/* What libminigl.a does: the library's one entry, LVO -30. */
static const MGLDispatchTable *getDispatch(struct Library *base) {
	register struct Library *a6 __asm("a6") = base;
	register const MGLDispatchTable *d0 __asm("d0");
	__asm volatile ("jsr -30(a6)" : "=r" (d0) : "r" (a6) : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
	return d0;
}

static int failures;

/* What the Amiga display shows at (x, y) of the picture (from the bottom, as
 * glReadPixels): the frame written into display memory (phase 8). 15- and
 * 16-bit screens round the channels, hence the wider tolerance. */
static void expectShown(const char *what, int x, int y, int r, int g, int b) {
	struct Window *w = (struct Window *) mglGetWindowHandle();
	ULONG argb;
	int pr, pg, pb, ok;
	if (!P96Base || !w) {
		printf("m01: SKIP shown %s: no Picasso96 window\n", what);
		return;
	}
	argb = p96ReadPixel(w->RPort, (UWORD) (x + w->BorderLeft), (UWORD) (239 - y + w->BorderTop));
	pr = (argb >> 16) & 0xFF;
	pg = (argb >> 8) & 0xFF;
	pb = argb & 0xFF;
	ok = pr >= r - 8 && pr <= r + 8 && pg >= g - 8 && pg <= g + 8 && pb >= b - 8 && pb <= b + 8;
	if (!ok) ++failures;
	printf("m01: %s shown %s: (%d, %d, %d), expected (%d, %d, %d)\n", ok ? "PASS" : "FAIL", what, pr, pg, pb, r, g, b);
}

static void expect(const char *what, int x, int y, int r, int g, int b) {
	GLubyte p[4];
	int ok;
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, p);
	ok = p[0] >= r - 3 && p[0] <= r + 3 && p[1] >= g - 3 && p[1] <= g + 3 && p[2] >= b - 3 && p[2] <= b + 3;
	if (!ok) ++failures;
	printf("m01: %s %s: (%d, %d, %d), expected (%d, %d, %d)\n", ok ? "PASS" : "FAIL", what, p[0], p[1], p[2], r, g, b);
}

static void rectangle(float x0, float x1) {
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(x0, 0);
	glTexCoord2f(1, 0); glVertex2f(x1, 0);
	glTexCoord2f(1, 1); glVertex2f(x1, 240);
	glTexCoord2f(0, 1); glVertex2f(x0, 240);
	glEnd();
}

static GLuint texture(GLenum format, int width, const GLubyte *pixels) {
	GLuint name;
	glGenTextures(1, &name);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (format == GL_COLOR_INDEX) glTexImage2D(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, width, 2, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, (GLvoid *) pixels);
	else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *) pixels);
	return name;
}

static void draw(GLuint paletted, GLuint orange, GLuint cyan) {
	/* Interleaved: s, t, then r, g, b, a, then x, y, z per vertex. */
	static struct { GLfloat st[2]; GLubyte rgba[4]; GLfloat xyz[3]; } yellow[4] = {
		{{0, 0}, {255, 255, 0, 255}, {80, 0, 0}}, {{1, 0}, {255, 255, 0, 255}, {160, 0, 0}},
		{{1, 1}, {255, 255, 0, 255}, {160, 240, 0}}, {{0, 1}, {255, 255, 0, 255}, {80, 240, 0}}
	};
	static const GLfloat quad[8] = {240, 0, 320, 0, 320, 240, 240, 240};
	static const GLfloat st[8] = {0, 0, 1, 0, 1, 1, 0, 1};

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor4f(1, 1, 1, 1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, paletted);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	rectangle(0, 80);
	glDisable(GL_TEXTURE_2D);

	glInterleavedArrays(GL_T2F_C4UB_V3F, 0, yellow);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glColor4f(0.25f, 0.25f, 0.25f, 1);
	rectangle(160, 240);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_SUBTRACT);
	glColor4f(1, 1, 1, 1);
	rectangle(160, 240);
	glBlendEquation(GL_FUNC_ADD);
	glDisable(GL_BLEND);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, orange);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, cyan);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, (GLvoid *) quad);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, (GLvoid *) st);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, (GLvoid *) st);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);
}

int main(void) {
	/* Palette entries 1 to 3; the index rows are padded to 4 bytes. */
	static const GLubyte palette[4][3] = {{0, 0, 0}, {255, 0, 0}, {0, 255, 0}, {0, 0, 255}};
	static const GLubyte indices[8] = {1, 2, 9, 9, 3, 1, 9, 9};
	static const GLubyte orange[16] = {255, 128, 0, 255, 255, 128, 0, 255, 255, 128, 0, 255, 255, 128, 0, 255};
	static const GLubyte cyan[16] = {128, 255, 255, 255, 128, 255, 255, 255, 128, 255, 255, 255, 128, 255, 255, 255};
	GLuint paletted, orangeTexture, cyanTexture;
	GLint units = 0;
	const char *extensions;
	int frame;

	MiniGLBase = OpenLibrary("minigl.library", MINIGL_VERSION);
	P96Base = OpenLibrary("Picasso96API.library", 2);
	if (!MiniGLBase) {
		printf("m01: FAIL no minigl.library\n");
		return 20;
	}
	MiniGLDispatch = getDispatch(MiniGLBase);
	printf("m01: ABI %lu, %lu bytes, flags 0x%lx\n", (unsigned long) MiniGLDispatch->abiVersion, (unsigned long) MiniGLDispatch->structSize,
		(unsigned long) MiniGLDispatch->backendFlags);
	if (!mglCreateContext(0, 0, 320, 240)) {
		printf("m01: FAIL no context\n");
		CloseLibrary(MiniGLBase);
		return 20;
	}

	extensions = (const char *) glGetString(GL_EXTENSIONS);
	printf("m01: extensions %s\n", extensions ? extensions : "(none)");
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &units);
	if (units != 2) ++failures;
	printf("m01: %s texture units: %ld\n", units == 2 ? "PASS" : "FAIL", (long) units);

	glViewport(0, 0, 320, 240);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 320, 0, 240, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glColorTableEXT(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 4, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *) palette);
	glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
	if (!glIsEnabled(GL_SHARED_TEXTURE_PALETTE_EXT)) ++failures;
	paletted = texture(GL_COLOR_INDEX, 2, indices);
	orangeTexture = texture(GL_RGBA, 2, orange);
	cyanTexture = texture(GL_RGBA, 2, cyan);

	for (frame = 0; frame < 3; ++frame) {
		mglLockDisplay();
		draw(paletted, orangeTexture, cyanTexture);
		if (frame == 2) {
			expect("palette index 1", 20, 60, 255, 0, 0);
			expect("palette index 2", 60, 60, 0, 255, 0);
			expect("palette index 3", 20, 180, 0, 0, 255);
			expect("interleaved arrays", 120, 120, 255, 255, 0);
			expect("blend equation", 200, 120, 191, 191, 191);
			expect("multitexture", 280, 120, 128, 128, 0);
		}
		mglUnlockDisplay();
		mglSwitchDisplay();
	}

	/* glFinish has the last frame written into display memory. */
	glFinish();
	expectShown("palette index 1", 20, 60, 255, 0, 0);
	expectShown("palette index 3", 20, 180, 0, 0, 255);
	expectShown("interleaved arrays", 120, 120, 255, 255, 0);
	expectShown("blend equation", 200, 120, 191, 191, 191);
	expectShown("multitexture", 280, 120, 128, 128, 0);

	printf("m01: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	mglDeleteContext();
	CloseLibrary(MiniGLBase);
	if (P96Base) CloseLibrary(P96Base);
	return failures ? 5 : 0;
}
