#include "mgl.h"
#include <exec/memory.h>
#include <devices/timer.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/timer.h>

/*
 * The GLU and GLUT entries MiniGL 29 added to the dispatch table: GLU's
 * quadrics and gluBuild2DMipmaps, and the subset of GLUT that <mgl/glut.h>
 * declares (one window, the main loop, game mode, the solid shapes). Their
 * constants are GLU's and GLUT's own values, not translated. Shapes are drawn
 * in immediate mode with OpenGL's constants (QGL_*); the host does the rest.
 */

#define PI 3.14159265358979323846

void mgl_GLTexImage2D(GLcontext context, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
	GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void mgl_MGLSwitchDisplay(GLcontext context);
void mgl_mglChoosePixelDepth(int depth);

/* --- GLU ------------------------------------------------------------------ */

/* The host builds the mipmaps (GL_GENERATE_MIPMAP) of the image as it is:
 * its OpenGL takes any size, so nothing is scaled. */
GLint mgl_GLUBuild2DMipmaps(GLcontext context, GLenum target, GLint internalFormat, GLsizei width, GLsizei height,
		GLenum format, GLenum type, const GLvoid *data) {
	unsigned int t = mgl_enum(target);
	if ((t & QT_MGL_ONLY) == QT_MGL_ONLY || width < 1 || height < 1 || !data) return GLU_INVALID_VALUE;
	_glTexParameteri(t, QGL_GENERATE_MIPMAP, 1);
	mgl_GLTexImage2D(context, target, 0, internalFormat, width, height, 0, format, type, data);
	return 0;
}

const GLubyte *mgl_GLUErrorString(GLenum errCode) {
	switch (errCode) {
	case GLU_INVALID_ENUM: return (const GLubyte *) "invalid enumerant";
	case GLU_INVALID_VALUE: return (const GLubyte *) "invalid value";
	case GLU_OUT_OF_MEMORY: return (const GLubyte *) "out of memory";
	case GLU_INCOMPATIBLE_GL_VERSION: return (const GLubyte *) "incompatible gl version";
	case GLU_INVALID_OPERATION: return (const GLubyte *) "invalid operation";
	}
	switch (mgl_enum(errCode)) {
	case 0: return (const GLubyte *) "no error";
	case QGL_INVALID_ENUM: return (const GLubyte *) "invalid enumerant";
	case QGL_INVALID_VALUE: return (const GLubyte *) "invalid value";
	case QGL_INVALID_OPERATION: return (const GLubyte *) "invalid operation";
	case QGL_STACK_OVERFLOW: return (const GLubyte *) "stack overflow";
	case QGL_STACK_UNDERFLOW: return (const GLubyte *) "stack underflow";
	case QGL_OUT_OF_MEMORY: return (const GLubyte *) "out of memory";
	}
	return NULL;
}

struct GLUquadricObj_t {
	GLenum normals, drawStyle, orientation;
	GLboolean texture;
};

GLUquadricObj *mgl_GLUNewQuadric(void) {
	GLUquadricObj *q = (GLUquadricObj *) AllocVec(sizeof(GLUquadricObj), MEMF_ANY | MEMF_CLEAR);
	if (!q) return NULL;
	q->normals = GLU_SMOOTH;
	q->drawStyle = GLU_FILL;
	q->orientation = GLU_OUTSIDE;
	return q;
}

void mgl_GLUDeleteQuadric(GLUquadricObj *q) { if (q) FreeVec(q); }
void mgl_GLUQuadricNormals(GLUquadricObj *q, GLenum normals) { if (q) q->normals = normals; }
void mgl_GLUQuadricTexture(GLUquadricObj *q, GLboolean textureCoords) { if (q) q->texture = textureCoords; }
void mgl_GLUQuadricDrawStyle(GLUquadricObj *q, GLenum drawStyle) { if (q) q->drawStyle = drawStyle; }
void mgl_GLUQuadricOrientation(GLUquadricObj *q, GLenum orientation) { if (q) q->orientation = orientation; }
/* Errors are not reported through it. */
void mgl_GLUQuadricCallback(GLUquadricObj *q, GLenum which, MGLUfuncptr fn) {}

/* Lines and points by the polygon mode; begins the quadric's drawing. */
static void beginQuadric(const GLUquadricObj *q) {
	_glPushAttrib(QGL_POLYGON_BIT);
	if (q->drawStyle == GLU_LINE || q->drawStyle == GLU_SILHOUETTE) _glPolygonMode(QGL_FRONT_AND_BACK, QGL_LINE);
	else if (q->drawStyle == GLU_POINT) _glPolygonMode(QGL_FRONT_AND_BACK, QGL_POINT);
}

static void endQuadric(void) {
	_glPopAttrib();
}

static void normal(const GLUquadricObj *q, GLfloat x, GLfloat y, GLfloat z) {
	if (q->normals == GLU_NONE) return;
	if (q->orientation == GLU_INSIDE) _glNormal3f(-x, -y, -z);
	else _glNormal3f(x, y, z);
}

static void texCoord(const GLUquadricObj *q, GLfloat s, GLfloat t) {
	if (q->texture) _glTexCoord2f(s, t);
}

/* Along z from 0 to height, radius base at 0 and top at height. */
void mgl_GLUCylinder(GLUquadricObj *q, GLdouble base, GLdouble top, GLdouble height, GLint slices, GLint stacks) {
	GLfloat nz, nxy, length;
	int i, j;
	if (!q || slices < 2 || stacks < 1 || height == 0) return;
	length = mgl_squareRoot((GLfloat) (height * height + (base - top) * (base - top)));
	nxy = (GLfloat) (height / length);
	nz = (GLfloat) ((base - top) / length);
	beginQuadric(q);
	for (j = 0; j < stacks; ++j) {
		GLfloat z0 = (GLfloat) (height * j / stacks), z1 = (GLfloat) (height * (j + 1) / stacks);
		GLfloat r0 = (GLfloat) (base + (top - base) * j / stacks), r1 = (GLfloat) (base + (top - base) * (j + 1) / stacks);
		_glBegin(QGL_QUAD_STRIP);
		for (i = 0; i <= slices; ++i) {
			double s, c;
			GLfloat u = (GLfloat) i / slices;
			mgl_sinCos(2 * PI * (i % slices) / slices, &s, &c);
			normal(q, (GLfloat) c * nxy, (GLfloat) s * nxy, nz);
			/* Counter-clockwise from the side the quadric faces. */
			if (q->orientation == GLU_INSIDE) {
				texCoord(q, u, (GLfloat) j / stacks);
				_glVertex3f(r0 * (GLfloat) c, r0 * (GLfloat) s, z0);
				texCoord(q, u, (GLfloat) (j + 1) / stacks);
				_glVertex3f(r1 * (GLfloat) c, r1 * (GLfloat) s, z1);
			}
			else {
				texCoord(q, u, (GLfloat) (j + 1) / stacks);
				_glVertex3f(r1 * (GLfloat) c, r1 * (GLfloat) s, z1);
				texCoord(q, u, (GLfloat) j / stacks);
				_glVertex3f(r0 * (GLfloat) c, r0 * (GLfloat) s, z0);
			}
		}
		_glEnd();
	}
	endQuadric();
}

/* Around the origin, the poles on the z axis. */
void mgl_GLUSphere(GLUquadricObj *q, GLdouble radius, GLint slices, GLint stacks) {
	int i, j;
	if (!q || slices < 2 || stacks < 2) return;
	beginQuadric(q);
	for (j = 0; j < stacks; ++j) {
		double s0, c0, s1, c1;
		mgl_sinCos(PI * j / stacks, &s0, &c0);
		mgl_sinCos(PI * (j + 1) / stacks, &s1, &c1);
		_glBegin(QGL_QUAD_STRIP);
		for (i = 0; i <= slices; ++i) {
			double s, c;
			GLfloat u = (GLfloat) i / slices, x0, y0, x1, y1;
			mgl_sinCos(2 * PI * (i % slices) / slices, &s, &c);
			x0 = (GLfloat) (c * s0); y0 = (GLfloat) (s * s0);
			x1 = (GLfloat) (c * s1); y1 = (GLfloat) (s * s1);
			if (q->orientation == GLU_INSIDE) {
				normal(q, x1, y1, (GLfloat) c1);
				texCoord(q, u, 1 - (GLfloat) (j + 1) / stacks);
				_glVertex3f(x1 * (GLfloat) radius, y1 * (GLfloat) radius, (GLfloat) (c1 * radius));
				normal(q, x0, y0, (GLfloat) c0);
				texCoord(q, u, 1 - (GLfloat) j / stacks);
				_glVertex3f(x0 * (GLfloat) radius, y0 * (GLfloat) radius, (GLfloat) (c0 * radius));
			}
			else {
				normal(q, x0, y0, (GLfloat) c0);
				texCoord(q, u, 1 - (GLfloat) j / stacks);
				_glVertex3f(x0 * (GLfloat) radius, y0 * (GLfloat) radius, (GLfloat) (c0 * radius));
				normal(q, x1, y1, (GLfloat) c1);
				texCoord(q, u, 1 - (GLfloat) (j + 1) / stacks);
				_glVertex3f(x1 * (GLfloat) radius, y1 * (GLfloat) radius, (GLfloat) (c1 * radius));
			}
		}
		_glEnd();
	}
	endQuadric();
}

/* In the plane z = 0, facing +z (GLU_OUTSIDE). */
void mgl_GLUDisk(GLUquadricObj *q, GLdouble inner, GLdouble outer, GLint slices, GLint loops) {
	int i, j;
	if (!q || slices < 2 || loops < 1 || outer <= 0) return;
	beginQuadric(q);
	for (j = 0; j < loops; ++j) {
		GLfloat r0 = (GLfloat) (inner + (outer - inner) * j / loops), r1 = (GLfloat) (inner + (outer - inner) * (j + 1) / loops);
		_glBegin(QGL_QUAD_STRIP);
		for (i = 0; i <= slices; ++i) {
			double s, c;
			int k;
			mgl_sinCos(2 * PI * (i % slices) / slices, &s, &c);
			normal(q, 0, 0, 1);
			for (k = 0; k < 2; ++k) {
				/* Inner first, counter-clockwise from +z; outer first from -z. */
				GLfloat r = (k == 0) == (q->orientation != GLU_INSIDE) ? r0 : r1;
				texCoord(q, (GLfloat) (0.5 + r * c / (2 * outer)), (GLfloat) (0.5 + r * s / (2 * outer)));
				_glVertex3f(r * (GLfloat) c, r * (GLfloat) s, 0);
			}
		}
		_glEnd();
	}
	endQuadric();
}

/* --- GLUT ----------------------------------------------------------------- */

static struct {
	GLcontext context; /* the window's (or game mode's) */
	void (*display)(void);
	void (*idle)(void);
	void (*keyboard)(unsigned char key, int x, int y);
	void (*reshape)(int width, int height);
	BOOL redisplay;
	int reshapedWidth, reshapedHeight;
	unsigned int displayMode;
	int initX, initY, initWidth, initHeight;
	BOOL gameMode;
	int gameWidth, gameHeight, gameDepth, gameRefresh;
} glut = {NULL, NULL, NULL, NULL, NULL, FALSE, -1, -1, 0, -1, -1, 300, 300, FALSE, 0, 0, 0, 0};

void mgl_glutForget(GLcontext context) {
	if (glut.context == context) {
		glut.context = NULL;
		glut.gameMode = FALSE;
	}
}

/* --- The clock (GLUT_ELAPSED_TIME) --- */

struct Device *TimerBase;
static struct timerequest timer;
static struct timeval start;

static void readClock(struct timeval *now) {
	if (!TimerBase && !OpenDevice((STRPTR) TIMERNAME, UNIT_MICROHZ, (struct IORequest *) &timer, 0)) {
		TimerBase = timer.tr_node.io_Device;
	}
	if (TimerBase) GetSysTime(now);
	else now->tv_secs = now->tv_micro = 0;
}

void mgl_gluExit(void) {
	if (TimerBase) CloseDevice((struct IORequest *) &timer);
	TimerBase = NULL;
}

static int elapsed(void) {
	struct timeval now;
	readClock(&now);
	return (int) ((now.tv_secs - start.tv_secs) * 1000 + ((LONG) now.tv_micro - (LONG) start.tv_micro) / 1000);
}

void mgl_GLUTInit(int *argcp, char **argv) {
	readClock(&start);
}

void mgl_GLUTInitDisplayMode(unsigned int mode) { glut.displayMode = mode; }
void mgl_GLUTInitWindowSize(int width, int height) { glut.initWidth = width; glut.initHeight = height; }
void mgl_GLUTInitWindowPosition(int x, int y) { glut.initX = x; glut.initY = y; }

/* One window: a context as MGLCreateContext makes it (a screen unless
 * mglChooseWindowMode asked for a window, as with MiniGL). */
int mgl_GLUTCreateWindow(const char *title) {
	void *mgl_MGLCreateContext(int offx, int offy, int w, int h);
	QtMglContext *c;
	if (glut.context) return 1;
	glut.context = (GLcontext) mgl_MGLCreateContext(glut.initX < 0 ? 0 : glut.initX, glut.initY < 0 ? 0 : glut.initY,
		glut.initWidth, glut.initHeight);
	if (!glut.context) return 0;
	c = QT_MGL(glut.context);
	if (c->ownWindow && !c->fullscreen && title) SetWindowTitles(c->window, (UBYTE *) title, (UBYTE *) ~0);
	glut.redisplay = TRUE;
	glut.reshapedWidth = glut.reshapedHeight = -1;
	return 1;
}

void mgl_GLUTDisplayFunc(void (*func)(void)) { glut.display = func; glut.redisplay = TRUE; }
void mgl_GLUTIdleFunc(void (*func)(void)) { glut.idle = func; }
void mgl_GLUTKeyboardFunc(void (*func)(unsigned char key, int x, int y)) { glut.keyboard = func; }
void mgl_GLUTReshapeFunc(void (*func)(int width, int height)) { glut.reshape = func; glut.reshapedWidth = -1; }
void mgl_GLUTPostRedisplay(void) { glut.redisplay = TRUE; }

void mgl_GLUTSwapBuffers(void) {
	GLcontext c = glut.context ? glut.context : mgl_current;
	if (c) mgl_MGLSwitchDisplay(c);
}

/* GLUT's keyboard callback takes the mouse position too, which a key message
 * does not carry here: 0, 0 (as MiniGL 29 does). */
static void glutKey(char key) {
	if (glut.keyboard) glut.keyboard((unsigned char) key, 0, 0);
}

void mgl_GLUTMainLoop(void) {
	QtMglContext *c = QT_MGL(glut.context);
	if (!c || !c->window) return;
	c->running = TRUE;
	c->buttons = 0;
	if (glut.keyboard) c->key = glutKey;
	while (mgl_handleEvents(c)) {
		if (glut.reshape && (c->width != glut.reshapedWidth || c->height != glut.reshapedHeight)) {
			glut.reshapedWidth = c->width;
			glut.reshapedHeight = c->height;
			glut.reshape(c->width, c->height);
			glut.redisplay = TRUE;
		}
		if (glut.redisplay && glut.display) {
			glut.redisplay = FALSE;
			glut.display();
			/* Single buffered: what was drawn shows without glutSwapBuffers. */
			if (!(glut.displayMode & GLUT_DOUBLE) && glut.context) mgl_MGLSwitchDisplay(glut.context);
		}
		if (!glut.context) break;
		if (glut.idle) glut.idle();
		else if (!glut.redisplay) mgl_waitForEvent(c);
	}
}

static int screenSize(BOOL height) {
	QtMglContext *c = QT_MGL(glut.context);
	struct Screen *screen;
	int size = 0;
	if (c && c->window) return height ? c->window->WScreen->Height : c->window->WScreen->Width;
	if ((screen = LockPubScreen(NULL))) {
		size = height ? screen->Height : screen->Width;
		UnlockPubScreen(NULL, screen);
	}
	return size;
}

int mgl_GLUTGet(GLenum state) {
	QtMglContext *c = QT_MGL(glut.context);
	switch (state) {
	case GLUT_ELAPSED_TIME: return elapsed();
	case GLUT_WINDOW_X: return c ? c->left : 0;
	case GLUT_WINDOW_Y: return c ? c->top : 0;
	case GLUT_WINDOW_WIDTH: return c ? c->width : 0;
	case GLUT_WINDOW_HEIGHT: return c ? c->height : 0;
	case GLUT_WINDOW_BUFFER_SIZE: return c ? 32 : 0;
	case GLUT_WINDOW_STENCIL_SIZE: return c ? 8 : 0;
	case GLUT_WINDOW_DEPTH_SIZE: return c ? 24 : 0;
	case GLUT_WINDOW_RED_SIZE: case GLUT_WINDOW_GREEN_SIZE: case GLUT_WINDOW_BLUE_SIZE: case GLUT_WINDOW_ALPHA_SIZE:
		return c ? 8 : 0;
	case GLUT_WINDOW_DOUBLEBUFFER: case GLUT_WINDOW_RGBA: return c ? 1 : 0;
	case GLUT_WINDOW_CURSOR: return GLUT_CURSOR_INHERIT;
	case GLUT_SCREEN_WIDTH: return screenSize(FALSE);
	case GLUT_SCREEN_HEIGHT: return screenSize(TRUE);
	case GLUT_DISPLAY_MODE_POSSIBLE: return 1;
	case GLUT_INIT_WINDOW_X: return glut.initX;
	case GLUT_INIT_WINDOW_Y: return glut.initY;
	case GLUT_INIT_WINDOW_WIDTH: return glut.initWidth;
	case GLUT_INIT_WINDOW_HEIGHT: return glut.initHeight;
	case GLUT_INIT_DISPLAY_MODE: return (int) glut.displayMode;
	}
	return 0;
}

/* --- Game mode --- */

/* A decimal number at *p, if there is one. */
static int number(const char **p, int *value) {
	int n = 0, digits = 0;
	while (**p >= '0' && **p <= '9') {
		n = n * 10 + (*(*p)++ - '0');
		++digits;
	}
	if (digits) *value = n;
	return digits;
}

/* "WIDTHxHEIGHT:BPP@REFRESH", every part optional. */
void mgl_GLUTGameModeString(const char *string) {
	const char *p = string;
	if (!p) return;
	number(&p, &glut.gameWidth);
	if (*p == 'x') { ++p; number(&p, &glut.gameHeight); }
	if (*p == ':') { ++p; number(&p, &glut.gameDepth); }
	if (*p == '@') { ++p; number(&p, &glut.gameRefresh); }
}

int mgl_GLUTEnterGameMode(void) {
	GLcontext c;
	if (glut.gameMode) return 1;
	if (glut.gameDepth) mgl_mglChoosePixelDepth(glut.gameDepth);
	c = mgl_createContext(0, 0, glut.gameWidth > 0 ? glut.gameWidth : 640, glut.gameHeight > 0 ? glut.gameHeight : 480, FALSE);
	if (!c) return 0;
	glut.context = c;
	glut.gameMode = TRUE;
	glut.redisplay = TRUE;
	glut.reshapedWidth = glut.reshapedHeight = -1;
	return 1;
}

/* Ends the main loop (MGLExit), as MiniGL 29 does; the screen goes with the
 * context. */
void mgl_GLUTLeaveGameMode(void) {
	if (!glut.gameMode || !glut.context) return;
	QT_MGL(glut.context)->running = FALSE;
	glut.gameMode = FALSE;
}

int mgl_GLUTGameModeGet(GLenum query) {
	QtMglContext *c = glut.gameMode ? QT_MGL(glut.context) : NULL;
	switch (query) {
	case GLUT_GAME_MODE_ACTIVE: case GLUT_GAME_MODE_DISPLAY_CHANGED: return glut.gameMode ? 1 : 0;
	case GLUT_GAME_MODE_POSSIBLE: return 1;
	case GLUT_GAME_MODE_WIDTH: return c ? c->width : glut.gameWidth;
	case GLUT_GAME_MODE_HEIGHT: return c ? c->height : glut.gameHeight;
	case GLUT_GAME_MODE_PIXEL_DEPTH: return c && c->screen ? c->screen->RastPort.BitMap->Depth : glut.gameDepth;
	case GLUT_GAME_MODE_REFRESH_RATE: return glut.gameRefresh;
	}
	return 0;
}

/* --- Solid shapes --- */

void mgl_GLUTSolidSphere(GLdouble radius, GLint slices, GLint stacks) {
	GLUquadricObj *q = mgl_GLUNewQuadric();
	if (!q) return;
	mgl_GLUSphere(q, radius, slices, stacks);
	mgl_GLUDeleteQuadric(q);
}

/* Its base at z = 0 (closed by a disk facing -z), its tip at z = height. */
void mgl_GLUTSolidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks) {
	GLUquadricObj *q = mgl_GLUNewQuadric();
	if (!q) return;
	mgl_GLUCylinder(q, base, 0, height, slices, stacks);
	q->orientation = GLU_INSIDE;
	mgl_GLUDisk(q, 0, base, slices, 1);
	mgl_GLUDeleteQuadric(q);
}

/* GLUT's: centred, edges of size. */
void mgl_GLUTSolidCube(GLdouble size) {
	static const GLfloat normals[6][3] = {{-1, 0, 0}, {0, 1, 0}, {1, 0, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
	static const UBYTE faces[6][4] = {{0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4}, {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3}};
	GLfloat v[8][3], h = (GLfloat) (size / 2);
	int i, k;
	for (i = 0; i < 8; ++i) {
		v[i][0] = i < 4 ? -h : h;
		v[i][1] = (i & 3) == 0 || (i & 3) == 1 ? -h : h;
		v[i][2] = i == 0 || i == 3 || i == 4 || i == 7 ? -h : h;
	}
	_glBegin(QGL_QUADS);
	for (i = 5; i >= 0; --i) {
		_glNormal3f(normals[i][0], normals[i][1], normals[i][2]);
		for (k = 0; k < 4; ++k) _glVertex3f(v[faces[i][k]][0], v[faces[i][k]][1], v[faces[i][k]][2]);
	}
	_glEnd();
}

/* GLUT's: around the z axis; innerRadius is the tube's, outerRadius the
 * ring's; sides around the tube, rings around the axis. */
void mgl_GLUTSolidTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings) {
	double theta = 0, sinTheta = 0, cosTheta = 1;
	int i, j;
	if (sides < 3 || rings < 3) return;
	for (i = rings - 1; i >= 0; --i) {
		double theta1 = theta + 2 * PI / rings, sinTheta1, cosTheta1, phi = 0;
		mgl_sinCos(theta1, &sinTheta1, &cosTheta1);
		_glBegin(QGL_QUAD_STRIP);
		for (j = sides; j >= 0; --j) {
			double sinPhi, cosPhi, dist;
			phi += 2 * PI / sides;
			mgl_sinCos(phi, &sinPhi, &cosPhi);
			dist = outerRadius + innerRadius * cosPhi;
			_glNormal3f((GLfloat) (cosTheta1 * cosPhi), (GLfloat) (-sinTheta1 * cosPhi), (GLfloat) sinPhi);
			_glVertex3f((GLfloat) (cosTheta1 * dist), (GLfloat) (-sinTheta1 * dist), (GLfloat) (innerRadius * sinPhi));
			_glNormal3f((GLfloat) (cosTheta * cosPhi), (GLfloat) (-sinTheta * cosPhi), (GLfloat) sinPhi);
			_glVertex3f((GLfloat) (cosTheta * dist), (GLfloat) (-sinTheta * dist), (GLfloat) (innerRadius * sinPhi));
		}
		_glEnd();
		theta = theta1;
		sinTheta = sinTheta1;
		cosTheta = cosTheta1;
	}
}

/*
 * GLUT's dodecahedron, its corners at a distance of sqrt(3): (+-1, +-1, +-1),
 * (0, +-1/phi, +-phi) and their cyclic permutations. The faces are worked out
 * once: each face's normal points to a corner of the dual icosahedron,
 * (0, +-phi, +-1) and its permutations, and its five corners are the ones
 * nearest to it, ordered counter-clockwise from outside.
 */
static GLfloat dodecaFaces[12][5][3], dodecaNormals[12][3];
static BOOL dodecaReady;

static void cyclic(GLfloat *v, int shift, GLfloat a, GLfloat b, GLfloat c) {
	GLfloat t[3];
	int k;
	t[0] = a; t[1] = b; t[2] = c;
	for (k = 0; k < 3; ++k) v[k] = t[(k + shift) % 3];
}

static void makeDodecahedron(void) {
	const GLfloat phi = 1.6180339887f, invPhi = 0.6180339887f;
	GLfloat corners[20][3], normals[12][3];
	int n = 0, f, i, k, s;
	for (i = 0; i < 8; ++i) {
		corners[n][0] = i & 1 ? 1 : -1;
		corners[n][1] = i & 2 ? 1 : -1;
		corners[n][2] = i & 4 ? 1 : -1;
		++n;
	}
	for (s = 0; s < 3; ++s)
		for (i = 0; i < 4; ++i) {
			cyclic(corners[n++], s, 0, i & 1 ? invPhi : -invPhi, i & 2 ? phi : -phi);
			cyclic(normals[s * 4 + i], s, 0, i & 1 ? phi : -phi, i & 2 ? 1 : -1);
		}
	for (f = 0; f < 12; ++f) {
		GLfloat *nv = normals[f], best = -100, length, centre[3] = {0, 0, 0};
		int members[5], count = 0, order[5], used[5] = {0, 0, 0, 0, 0};
		for (i = 0; i < 20; ++i) {
			GLfloat d = corners[i][0] * nv[0] + corners[i][1] * nv[1] + corners[i][2] * nv[2];
			if (d > best + 0.01f) best = d;
		}
		for (i = 0; i < 20 && count < 5; ++i) {
			GLfloat d = corners[i][0] * nv[0] + corners[i][1] * nv[1] + corners[i][2] * nv[2];
			if (d > best - 0.01f) members[count++] = i;
		}
		for (i = 0; i < 5; ++i) for (k = 0; k < 3; ++k) centre[k] += corners[members[i]][k] / 5;
		/* Each next corner: an unused neighbour (nearest) turning
		 * counter-clockwise about the normal. */
		order[0] = 0;
		used[0] = 1;
		for (i = 1; i < 5; ++i) {
			const GLfloat *p = corners[members[order[i - 1]]];
			int m, pick = -1;
			GLfloat nearest = 100;
			for (m = 0; m < 5; ++m) {
				const GLfloat *q = corners[members[m]];
				GLfloat a[3], b[3], turn, d;
				if (used[m]) continue;
				for (k = 0; k < 3; ++k) { a[k] = p[k] - centre[k]; b[k] = q[k] - centre[k]; }
				turn = (a[1] * b[2] - a[2] * b[1]) * nv[0] + (a[2] * b[0] - a[0] * b[2]) * nv[1] + (a[0] * b[1] - a[1] * b[0]) * nv[2];
				d = (p[0] - q[0]) * (p[0] - q[0]) + (p[1] - q[1]) * (p[1] - q[1]) + (p[2] - q[2]) * (p[2] - q[2]);
				if (turn > 0 && d < nearest) {
					nearest = d;
					pick = m;
				}
			}
			if (pick < 0) return;
			order[i] = pick;
			used[pick] = 1;
		}
		length = mgl_squareRoot(nv[0] * nv[0] + nv[1] * nv[1] + nv[2] * nv[2]);
		for (k = 0; k < 3; ++k) dodecaNormals[f][k] = nv[k] / length;
		for (i = 0; i < 5; ++i) for (k = 0; k < 3; ++k) dodecaFaces[f][i][k] = corners[members[order[i]]][k];
	}
	dodecaReady = TRUE;
}

void mgl_GLUTSolidDodecahedron(void) {
	int f, i;
	if (!dodecaReady) makeDodecahedron();
	if (!dodecaReady) return;
	for (f = 0; f < 12; ++f) {
		_glBegin(QGL_POLYGON);
		_glNormal3f(dodecaNormals[f][0], dodecaNormals[f][1], dodecaNormals[f][2]);
		for (i = 0; i < 5; ++i) _glVertex3f(dodecaFaces[f][i][0], dodecaFaces[f][i][1], dodecaFaces[f][i][2]);
		_glEnd();
	}
}
