#include "mgl.h"
#include <exec/memory.h>
#include <proto/exec.h>

/*
 * The GL entries mglgen.py cannot generate. Enum arguments are MiniGL's
 * numbers (mgl_enum translates them). Pointers to floats and integers that
 * the host would read go as byte-swapped copies, since the host reads Amiga
 * memory as it is; results the host writes are turned around afterwards.
 * Vertex arrays are read here, on the 68k, and sent as immediate mode (the
 * host has no access to them in their byte order); drawing them on the host
 * is stage 4 of docs/phase7-minigl.md.
 */

/* --- Byte order ----------------------------------------------------------- */

static ULONG swap32(ULONG v) {
	return (v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24);
}

/* count 32-bit values into dest, turned around. */
static void copySwapped32(void *dest, const void *src, int count) {
	const ULONG *s = (const ULONG *) src;
	ULONG *d = (ULONG *) dest;
	while (count-- > 0) *d++ = swap32(*s++);
}

static void swapInPlace32(void *data, int count) {
	copySwapped32(data, data, count);
}

/* count doubles, each turned around as a whole. */
static void copySwapped64(void *dest, const void *src, int count) {
	const ULONG *s = (const ULONG *) src;
	ULONG *d = (ULONG *) dest;
	for (; count > 0; --count, s += 2, d += 2) {
		ULONG high = s[0], low = s[1];
		d[0] = swap32(low);
		d[1] = swap32(high);
	}
}

/* --- Immediate mode with vectors ------------------------------------------ */

void mgl_GLColor3fv(GLcontext context, GLfloat *v) { _glColor3f(v[0], v[1], v[2]); }
void mgl_GLColor4fv(GLcontext context, GLfloat *v) { _glColor4f(v[0], v[1], v[2], v[3]); }
void mgl_GLColor3ubv(GLcontext context, GLubyte *v) { _glColor3ub(v[0], v[1], v[2]); }
void mgl_GLColor4ubv(GLcontext context, GLubyte *v) { _glColor4ub(v[0], v[1], v[2], v[3]); }
void mgl_GLTexCoord2fv(GLcontext context, GLfloat *v) { _glTexCoord2f(v[0], v[1]); }
void mgl_GLTexCoord4fv(GLcontext context, GLfloat *v) { _glTexCoord4f(v[0], v[1], v[2], v[3]); }
void mgl_GLVertex2fv(GLcontext context, GLfloat *v) { _glVertex2f(v[0], v[1]); }
void mgl_GLVertex3fv(GLcontext context, GLfloat *v) { _glVertex3f(v[0], v[1], v[2]); }
void mgl_GLVertex4fv(GLcontext context, GLfloat *v) { _glVertex4f(v[0], v[1], v[2], v[3]); }
void mgl_GLEdgeFlagv(GLcontext context, const GLboolean *flag) { _glEdgeFlag(*flag != 0); }
void mgl_GLIndexiv(GLcontext context, const GLint *c) { _glIndexi(*c); }

/* One texture unit (GL_ARB_multitexture is not offered): unit 0 only. */
void mgl_GLActiveTextureARB(GLcontext context, GLenum unit) {}
void mgl_GLClientActiveTextureARB(GLcontext context, GLenum unit) {}

void mgl_GLMultiTexCoord2fARB(GLcontext context, GLenum unit, GLfloat s, GLfloat t) {
	if (mgl_enum(unit) == QGL_TEXTURE0_ARB) _glTexCoord2f(s, t);
}

void mgl_GLMultiTexCoord2fvARB(GLcontext context, GLenum unit, GLfloat *v) {
	if (mgl_enum(unit) == QGL_TEXTURE0_ARB) _glTexCoord2f(v[0], v[1]);
}

/* --- Matrices -------------------------------------------------------------- */

void mgl_GLLoadMatrixf(GLcontext context, const GLfloat *m) {
	GLfloat copy[16];
	copySwapped32(copy, m, 16);
	_glLoadMatrixf(copy);
}

void mgl_GLMultMatrixf(GLcontext context, const GLfloat *m) {
	GLfloat copy[16];
	copySwapped32(copy, m, 16);
	_glMultMatrixf(copy);
}

void mgl_GLLoadMatrixd(GLcontext context, const GLdouble *m) {
	GLdouble copy[16];
	copySwapped64(copy, m, 16);
	_glLoadMatrixd(copy);
}

void mgl_GLMultMatrixd(GLcontext context, const GLdouble *m) {
	GLdouble copy[16];
	copySwapped64(copy, m, 16);
	_glMultMatrixd(copy);
}

/* sin and cos of x (radians) by their series, after reducing x to -pi..pi:
 * the library has no maths library. */
static void sinCos(double x, double *s, double *c) {
	const double pi = 3.14159265358979323846;
	double term, sum;
	int n;
	while (x > pi) x -= 2 * pi;
	while (x < -pi) x += 2 * pi;
	term = x; sum = x;
	for (n = 1; n < 12; ++n) {
		term *= -x * x / ((2 * n) * (2 * n + 1));
		sum += term;
	}
	*s = sum;
	term = 1; sum = 1;
	for (n = 1; n < 12; ++n) {
		term *= -x * x / ((2 * n - 1) * (2 * n));
		sum += term;
	}
	*c = sum;
}

static GLfloat squareRoot(GLfloat x) {
	GLfloat r = x > 1 ? x : 1;
	int i;
	if (x <= 0) return 0;
	for (i = 0; i < 30; ++i) r = 0.5f * (r + x / r);
	return r;
}

/* gluPerspective: fovy in degrees. */
void mgl_GLUPerspective(GLfloat fovy, GLfloat aspect, GLfloat znear, GLfloat zfar) {
	double s, c, f;
	GLfloat m[16];
	int i;
	sinCos(fovy * 3.14159265358979323846 / 360.0, &s, &c);
	f = c / s;
	for (i = 0; i < 16; ++i) m[i] = 0;
	m[0] = (GLfloat) (f / aspect);
	m[5] = (GLfloat) f;
	m[10] = (zfar + znear) / (znear - zfar);
	m[11] = -1;
	m[14] = 2 * zfar * znear / (znear - zfar);
	mgl_GLMultMatrixf(mgl_current, m);
}

/* gluLookAt. */
void mgl_GLULookAt(GLfloat ex, GLfloat ey, GLfloat ez, GLfloat cx, GLfloat cy, GLfloat cz, GLfloat ux, GLfloat uy, GLfloat uz) {
	GLfloat f[3] = {cx - ex, cy - ey, cz - ez}, s[3], u[3], m[16], length;
	int i;
	length = squareRoot(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
	if (length > 0) for (i = 0; i < 3; ++i) f[i] /= length;
	s[0] = f[1] * uz - f[2] * uy;
	s[1] = f[2] * ux - f[0] * uz;
	s[2] = f[0] * uy - f[1] * ux;
	length = squareRoot(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
	if (length > 0) for (i = 0; i < 3; ++i) s[i] /= length;
	u[0] = s[1] * f[2] - s[2] * f[1];
	u[1] = s[2] * f[0] - s[0] * f[2];
	u[2] = s[0] * f[1] - s[1] * f[0];
	for (i = 0; i < 16; ++i) m[i] = 0;
	m[0] = s[0]; m[4] = s[1]; m[8] = s[2];
	m[1] = u[0]; m[5] = u[1]; m[9] = u[2];
	m[2] = -f[0]; m[6] = -f[1]; m[10] = -f[2];
	m[15] = 1;
	mgl_GLMultMatrixf(mgl_current, m);
	_glTranslatef(-ex, -ey, -ez);
}

/* MiniGL's rotation about one axis: xyz 0 = x, 1 = y, 2 = z. */
void mgl_GLRotatefEXT(GLcontext context, GLfloat angle, const GLint xyz) {
	_glRotatef(angle, xyz == 0, xyz == 1, xyz == 2);
}

/* The same with the angle given as its sine and cosine. */
void mgl_GLRotatefEXTs(GLcontext context, GLfloat sin_an, GLfloat cos_an, const GLint xyz) {
	GLfloat m[16];
	int i, a = xyz == 0 ? 1 : 0, b = xyz == 2 ? 1 : 2;
	for (i = 0; i < 16; ++i) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
	m[a * 4 + a] = cos_an;
	m[b * 4 + b] = cos_an;
	m[a * 4 + b] = sin_an;
	m[b * 4 + a] = -sin_an;
	mgl_GLMultMatrixf(context, m);
}

/* --- State ----------------------------------------------------------------- */

/* glEnable/glDisable go through MGLSetState. MiniGL's own capabilities
 * (MGL_PERSPECTIVE_MAPPING, MGL_Z_OFFSET, ...) have no OpenGL meaning. */
void mgl_MGLSetState(GLcontext context, GLenum cap, GLboolean state) {
	unsigned int gl = mgl_enum(cap);
	if ((gl & QT_MGL_ONLY) == QT_MGL_ONLY) return;
	if (state) _glEnable(gl);
	else _glDisable(gl);
}

GLboolean mgl_GLIsEnabled(GLcontext context, GLenum cap) {
	unsigned int gl = mgl_enum(cap);
	if ((gl & QT_MGL_ONLY) == QT_MGL_ONLY) return GL_FALSE;
	return _glIsEnabled(gl) ? GL_TRUE : GL_FALSE;
}

GLenum mgl_GLGetError(GLcontext context) {
	return mgl_enum_back(_glGetError());
}

void mgl_GLTexEnvi(GLcontext context, GLenum target, GLenum pname, GLint param) {
	_glTexEnvi(mgl_enum(target), mgl_enum(pname), (int) mgl_enum(param));
}

void mgl_GLTexParameteri(GLcontext context, GLenum target, GLenum pname, GLint param) {
	_glTexParameteri(mgl_enum(target), mgl_enum(pname), (int) mgl_enum(param));
}

/* The host's swap byte modes are the inverse of the application's (see
 * attach in context.c). */
void mgl_GLPixelStorei(GLcontext context, GLenum pname, GLint param) {
	unsigned int gl = mgl_enum(pname);
	if (gl == QGL_UNPACK_SWAP_BYTES || gl == QGL_PACK_SWAP_BYTES) param = !param;
	_glPixelStorei(gl, param);
}

void mgl_GLFogf(GLcontext context, GLenum pname, GLfloat param) {
	unsigned int gl = mgl_enum(pname);
	if (gl == QGL_FOG_MODE) param = (GLfloat) mgl_enum((unsigned int) param);
	_glFogf(gl, param);
}

void mgl_GLFogfv(GLcontext context, GLenum pname, GLfloat *param) {
	unsigned int gl = mgl_enum(pname);
	if (gl == QGL_FOG_COLOR) {
		GLfloat copy[4];
		copySwapped32(copy, param, 4);
		_glFogfv(gl, copy);
	}
	else mgl_GLFogf(context, pname, param[0]);
}

void mgl_GLTexGenfv(GLcontext context, GLenum coord, GLenum pname, const GLfloat *params) {
	unsigned int gl = mgl_enum(pname);
	if (gl == QGL_OBJECT_PLANE || gl == QGL_EYE_PLANE) {
		GLfloat copy[4];
		copySwapped32(copy, params, 4);
		_glTexGenfv(mgl_enum(coord), gl, copy);
	}
	else _glTexGeni(mgl_enum(coord), gl, (int) mgl_enum((unsigned int) params[0]));
}

/* --- Queries ---------------------------------------------------------------- */

/* Values glGet* returns for pname. */
static int getCount(unsigned int pname) {
	switch (pname) {
	case QGL_MODELVIEW_MATRIX: case QGL_PROJECTION_MATRIX: case QGL_TEXTURE_MATRIX:
		return 16;
	case QGL_COLOR_CLEAR_VALUE: case QGL_COLOR_WRITEMASK: case QGL_CURRENT_COLOR: case QGL_CURRENT_RASTER_COLOR:
	case QGL_CURRENT_RASTER_POSITION: case QGL_CURRENT_RASTER_TEXTURE_COORDS: case QGL_CURRENT_TEXTURE_COORDS:
	case QGL_FOG_COLOR: case QGL_LIGHT_MODEL_AMBIENT: case QGL_SCISSOR_BOX: case QGL_TEXTURE_ENV_COLOR: case QGL_VIEWPORT:
	case QGL_ACCUM_CLEAR_VALUE:
		return 4;
	case QGL_CURRENT_NORMAL:
		return 3;
	case QGL_DEPTH_RANGE: case QGL_LINE_WIDTH_RANGE: case QGL_MAX_VIEWPORT_DIMS: case QGL_POINT_SIZE_RANGE:
	case QGL_POLYGON_MODE:
		return 2;
	}
	return 1;
}

/* Results that are constants: given back as MiniGL's numbers. */
static int enumResult(unsigned int pname) {
	switch (pname) {
	case QGL_MATRIX_MODE: case QGL_SHADE_MODEL: case QGL_BLEND_SRC: case QGL_BLEND_DST: case QGL_DEPTH_FUNC:
	case QGL_ALPHA_TEST_FUNC: case QGL_FRONT_FACE: case QGL_CULL_FACE_MODE: case QGL_FOG_MODE: case QGL_DRAW_BUFFER:
	case QGL_READ_BUFFER: case QGL_POLYGON_MODE: case QGL_PERSPECTIVE_CORRECTION_HINT: case QGL_FOG_HINT:
		return 1;
	}
	return 0;
}

void mgl_GLGetIntegerv(GLcontext context, GLenum pname, GLint *params) {
	unsigned int gl = mgl_enum(pname);
	GLint values[16];
	int i, count = getCount(gl);
	for (i = 0; i < 16; ++i) values[i] = 0;
	if (gl == QGL_MAX_TEXTURE_UNITS_ARB) {
		*params = 1;
		return;
	}
	_glGetIntegerv(gl, values);
	swapInPlace32(values, count);
	for (i = 0; i < count; ++i) params[i] = enumResult(gl) ? (GLint) mgl_enum_back((unsigned int) values[i]) : values[i];
}

void mgl_GLGetFloatv(GLcontext context, GLenum pname, GLfloat *params) {
	unsigned int gl = mgl_enum(pname);
	GLfloat values[16];
	int count = getCount(gl);
	_glGetFloatv(gl, values);
	copySwapped32(params, values, count);
}

void mgl_GLGetDoublev(GLcontext context, GLenum pname, GLdouble *params) {
	unsigned int gl = mgl_enum(pname);
	GLdouble values[16];
	int count = getCount(gl);
	_glGetDoublev(gl, values);
	copySwapped64(params, values, count);
}

/* MiniGL's GLboolean is 4 bytes, the host's 1. */
void mgl_GLGetBooleanv(GLcontext context, GLenum pname, GLboolean *params) {
	unsigned int gl = mgl_enum(pname);
	unsigned char values[16];
	int i, count = getCount(gl);
	_glGetBooleanv(gl, values);
	for (i = 0; i < count; ++i) params[i] = values[i] ? GL_TRUE : GL_FALSE;
}

void mgl_GLGetPointerv(GLcontext context, GLenum pname, GLvoid **params) {
	*params = NULL;
}

static const char vendor[] = "QuarkTex";
static const char renderer[] = "QuarkTex minigl.library (host OpenGL)";
static const char version[] = "1.2";
static const char extensions[] = "GL_EXT_compiled_vertex_array";

const GLubyte *mgl_GLGetString(GLcontext context, GLenum name) {
	switch (mgl_enum(name)) {
	case QGL_VENDOR: return (const GLubyte *) vendor;
	case QGL_RENDERER: return (const GLubyte *) renderer;
	case QGL_VERSION: return (const GLubyte *) version;
	case QGL_EXTENSIONS: return (const GLubyte *) extensions;
	}
	return (const GLubyte *) "";
}

/* --- Textures --------------------------------------------------------------- */

/* internalformat is a number of components (1 to 4) or a format. */
void mgl_GLTexImage2D(GLcontext context, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
		GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
	int components = internalformat >= 1 && internalformat <= 4 ? internalformat : (int) mgl_enum((unsigned int) internalformat);
	_glTexImage2D(mgl_enum(target), level, components, width, height, border, mgl_enum(format), mgl_enum(type), (void *) pixels);
}

/* The host writes the names in its byte order. */
void mgl_GLGenTextures(GLcontext context, GLsizei n, GLuint *textures) {
	_glGenTextures(n, textures);
	swapInPlace32(textures, n);
}

/* Up to 64 names on the stack, more in a buffer. */
static GLuint *swappedNames(GLuint *buffer, const GLuint *names, GLsizei n) {
	GLuint *copy = n <= 64 ? buffer : (GLuint *) AllocVec(n * 4, MEMF_ANY);
	if (copy) copySwapped32(copy, names, n);
	return copy;
}

static void releaseNames(GLuint *buffer, GLuint *copy) {
	if (copy && copy != buffer) FreeVec(copy);
}

void mgl_GLDeleteTextures(GLcontext context, GLsizei n, const GLuint *textures) {
	GLuint buffer[64], *copy = swappedNames(buffer, textures, n);
	if (!copy) return;
	_glDeleteTextures(n, copy);
	releaseNames(buffer, copy);
}

void mgl_GLPrioritizeTextures(GLcontext context, GLsizei n, const GLuint *textures, const GLclampf *priorities) {
	GLuint buffer[64], buffer2[64], *copy = swappedNames(buffer, textures, n);
	GLuint *copy2 = swappedNames(buffer2, (const GLuint *) priorities, n);
	if (copy && copy2) _glPrioritizeTextures(n, copy, (float *) copy2);
	releaseNames(buffer, copy);
	releaseNames(buffer2, copy2);
}

GLboolean mgl_GLAreTexturesResident(GLcontext context, GLsizei n, const GLuint *textures, GLboolean *residences) {
	int i;
	for (i = 0; i < n; ++i) residences[i] = GL_TRUE;
	return GL_TRUE;
}

/* Paletted textures (GL_EXT_color_table) are not offered. */
void mgl_GLColorTable(GLcontext context, GLenum target, GLenum internalformat, GLint width, GLenum format, GLenum type, GLvoid *data) {
	mgl_missing("GLColorTable");
}

/* --- Vertex arrays (read on the 68k) ----------------------------------------- */

typedef struct {
	BOOL enabled;
	int size;
	unsigned int type; /* OpenGL's */
	int stride;        /* in bytes, never 0 */
	const UBYTE *pointer;
} Array;

static Array vertices, colors, texCoords;

static int typeSize(unsigned int type) {
	switch (type) {
	case QGL_UNSIGNED_BYTE: case QGL_BYTE: return 1;
	case QGL_UNSIGNED_SHORT: case QGL_SHORT: return 2;
	case QGL_DOUBLE: return 8;
	}
	return 4;
}

static void setArray(Array *a, int size, GLenum type, GLsizei stride, const GLvoid *pointer) {
	a->size = size;
	a->type = mgl_enum(type);
	a->stride = stride ? stride : size * typeSize(a->type);
	a->pointer = (const UBYTE *) pointer;
}

void mgl_resetArrays(void) {
	vertices.enabled = colors.enabled = texCoords.enabled = FALSE;
}

void mgl_GLVertexPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
	setArray(&vertices, size, type, stride, pointer);
}

void mgl_GLColorPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
	setArray(&colors, size, type, stride, pointer);
}

void mgl_GLTexCoordPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
	setArray(&texCoords, size, type, stride, pointer);
}

static Array *clientArray(GLenum cap) {
	switch (mgl_enum(cap)) {
	case QGL_VERTEX_ARRAY: return &vertices;
	case QGL_COLOR_ARRAY: return &colors;
	case QGL_TEXTURE_COORD_ARRAY: return &texCoords;
	}
	return NULL;
}

void mgl_GLEnableClientState(GLcontext context, GLenum cap) {
	Array *a = clientArray(cap);
	if (a) a->enabled = TRUE;
}

void mgl_GLDisableClientState(GLcontext context, GLenum cap) {
	Array *a = clientArray(cap);
	if (a) a->enabled = FALSE;
}

/* Component k of element i as a float. */
static GLfloat component(const Array *a, int i, int k) {
	const UBYTE *p = a->pointer + i * a->stride;
	switch (a->type) {
	case QGL_FLOAT: return ((const GLfloat *) p)[k];
	case QGL_DOUBLE: return (GLfloat) ((const GLdouble *) p)[k];
	case QGL_INT: return (GLfloat) ((const LONG *) p)[k];
	case QGL_SHORT: return (GLfloat) ((const WORD *) p)[k];
	}
	return 0;
}

static void element(int i) {
	if (colors.enabled && colors.pointer) {
		if (colors.type == QGL_UNSIGNED_BYTE) {
			const UBYTE *p = colors.pointer + i * colors.stride;
			if (colors.size == 3) _glColor3ub(p[0], p[1], p[2]);
			else _glColor4ub(p[0], p[1], p[2], p[3]);
		}
		else if (colors.size == 3) _glColor3f(component(&colors, i, 0), component(&colors, i, 1), component(&colors, i, 2));
		else _glColor4f(component(&colors, i, 0), component(&colors, i, 1), component(&colors, i, 2), component(&colors, i, 3));
	}
	if (texCoords.enabled && texCoords.pointer) {
		if (texCoords.size == 4) {
			_glTexCoord4f(component(&texCoords, i, 0), component(&texCoords, i, 1), component(&texCoords, i, 2), component(&texCoords, i, 3));
		}
		else _glTexCoord2f(component(&texCoords, i, 0), component(&texCoords, i, 1));
	}
	if (vertices.enabled && vertices.pointer) {
		if (vertices.size == 2) _glVertex2f(component(&vertices, i, 0), component(&vertices, i, 1));
		else if (vertices.size == 4) {
			_glVertex4f(component(&vertices, i, 0), component(&vertices, i, 1), component(&vertices, i, 2), component(&vertices, i, 3));
		}
		else _glVertex3f(component(&vertices, i, 0), component(&vertices, i, 1), component(&vertices, i, 2));
	}
}

void mgl_GLArrayElement(GLcontext context, GLint i) {
	element(i);
}

void mgl_GLDrawArrays(GLcontext context, GLenum mode, GLint first, GLsizei count) {
	int i;
	_glBegin(mgl_enum(mode));
	for (i = first; i < first + count; ++i) element(i);
	_glEnd();
}

void mgl_GLMultiDrawArrays(GLcontext context, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount) {
	int i;
	for (i = 0; i < primcount; ++i) mgl_GLDrawArrays(context, mode, first[i], count[i]);
}

void mgl_GLDrawElements(GLcontext context, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {
	unsigned int t = mgl_enum(type);
	int n;
	_glBegin(mgl_enum(mode));
	for (n = 0; n < count; ++n) {
		if (t == QGL_UNSIGNED_BYTE) element(((const UBYTE *) indices)[n]);
		else if (t == QGL_UNSIGNED_SHORT) element(((const UWORD *) indices)[n]);
		else element(((const ULONG *) indices)[n]);
	}
	_glEnd();
}

/* GL_EXT_compiled_vertex_array: nothing to compile here. */
void mgl_GLLockArrays(GLcontext context, GLuint first, GLsizei count) {}
void mgl_GLUnlockArrays(GLcontext context) {}
