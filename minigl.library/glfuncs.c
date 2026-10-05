#include "mgl.h"
#include <exec/memory.h>
#include <proto/exec.h>

/*
 * The GL entries mglgen.py cannot generate. Enum arguments are MiniGL's
 * numbers (mgl_enum translates them). Pointers to floats and integers that
 * the host would read go as byte-swapped copies, since the host reads Amiga
 * memory as it is; results the host writes are turned around afterwards.
 * glDrawArrays and glDrawElements leave reading the vertex arrays to the
 * host; glArrayElement reads them here.
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

/* --- GL_ARB_multitexture (QT_MGL_TEXTURE_UNITS units) ---------------------- */

void mgl_GLActiveTextureARB(GLcontext context, GLenum unit) {
	ULONG *w = qt_reserve(QT_MGL_ACTIVE_TEXTURE_WORDS);
	w[0] = ((ULONG) QT_MGL_ACTIVE_TEXTURE << 16) | QT_MGL_ACTIVE_TEXTURE_WORDS;
	w[1] = mgl_enum(unit);
}

static void multiTexCoord(unsigned int unit, GLfloat s, GLfloat t) {
	ULONG *w;
	if (unit == QGL_TEXTURE0_ARB) {
		_glTexCoord2f(s, t);
		return;
	}
	w = qt_reserve(QT_MGL_MULTI_TEX_COORD_WORDS);
	w[0] = ((ULONG) QT_MGL_MULTI_TEX_COORD << 16) | QT_MGL_MULTI_TEX_COORD_WORDS;
	w[1] = unit;
	*(GLfloat *) &w[2] = s;
	*(GLfloat *) &w[3] = t;
}

void mgl_GLMultiTexCoord2fARB(GLcontext context, GLenum unit, GLfloat s, GLfloat t) {
	multiTexCoord(mgl_enum(unit), s, t);
}

void mgl_GLMultiTexCoord2fvARB(GLcontext context, GLenum unit, GLfloat *v) {
	multiTexCoord(mgl_enum(unit), v[0], v[1]);
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

/* GL_SHARED_TEXTURE_PALETTE_EXT: the only palette is the shared one, applied
 * when a texture is loaded (see glColorTable), so the state is only kept. */
static GLboolean sharedPalette;

/* glEnable/glDisable go through MGLSetState. MiniGL's own capabilities
 * (MGL_PERSPECTIVE_MAPPING, MGL_Z_OFFSET, ...) have no OpenGL meaning. */
void mgl_MGLSetState(GLcontext context, GLenum cap, GLboolean state) {
	unsigned int gl = mgl_enum(cap);
	if ((gl & QT_MGL_ONLY) == QT_MGL_ONLY) return;
	if (gl == QGL_SHARED_TEXTURE_PALETTE_EXT) sharedPalette = state ? GL_TRUE : GL_FALSE;
	else if (state) _glEnable(gl);
	else _glDisable(gl);
}

GLboolean mgl_GLIsEnabled(GLcontext context, GLenum cap) {
	unsigned int gl = mgl_enum(cap);
	if ((gl & QT_MGL_ONLY) == QT_MGL_ONLY) return GL_FALSE;
	if (gl == QGL_SHARED_TEXTURE_PALETTE_EXT) return sharedPalette;
	return _glIsEnabled(gl) ? GL_TRUE : GL_FALSE;
}

/* OpenGL 1.2's glBlendEquation and 1.4's glBlendFuncSeparate, which the
 * OpenGL 1.1 command set lacks. */
void mgl_GLBlendEquation(GLcontext context, GLenum mode) {
	ULONG *w = qt_reserve(QT_MGL_BLEND_EQUATION_WORDS);
	w[0] = ((ULONG) QT_MGL_BLEND_EQUATION << 16) | QT_MGL_BLEND_EQUATION_WORDS;
	w[1] = mgl_enum(mode);
}

void mgl_GLBlendFuncSeparate(GLcontext context, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
	ULONG *w = qt_reserve(QT_MGL_BLEND_FUNC_SEPARATE_WORDS);
	w[0] = ((ULONG) QT_MGL_BLEND_FUNC_SEPARATE << 16) | QT_MGL_BLEND_FUNC_SEPARATE_WORDS;
	w[1] = mgl_enum(srcRGB);
	w[2] = mgl_enum(dstRGB);
	w[3] = mgl_enum(srcAlpha);
	w[4] = mgl_enum(dstAlpha);
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
static int unpackAlignment = 4; /* for colour index images, expanded here */

void mgl_GLPixelStorei(GLcontext context, GLenum pname, GLint param) {
	unsigned int gl = mgl_enum(pname);
	if (gl == QGL_UNPACK_SWAP_BYTES || gl == QGL_PACK_SWAP_BYTES) param = !param;
	if (gl == QGL_UNPACK_ALIGNMENT && (param == 1 || param == 2 || param == 4 || param == 8)) unpackAlignment = param;
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
		*params = QT_MGL_TEXTURE_UNITS;
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
/* MiniGL's name for GL_ARB_multitexture, which applications looking for
 * "GL_ARB_multitexture" in the string find as well. */
static const char extensions[] = "GL_MGL_ARB_multitexture GL_EXT_compiled_vertex_array GL_EXT_color_table "
	"GL_EXT_shared_texture_palette";

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

/*
 * Paletted textures as MiniGL has them (GL_EXT_color_table,
 * GL_EXT_shared_texture_palette): one shared palette, set with glColorTable
 * and applied when a GL_COLOR_INDEX image is loaded, which is expanded here
 * into RGBA. Changing the palette does not change textures already loaded.
 */
static UBYTE palette[256][4];
static BOOL paletteAlpha;

void mgl_GLColorTable(GLcontext context, GLenum target, GLenum internalformat, GLint width, GLenum format, GLenum type, GLvoid *data) {
	unsigned int f = mgl_enum(format);
	int components = f == QGL_RGBA ? 4 : f == QGL_RGB ? 3 : 0, i, k;
	const UBYTE *p = (const UBYTE *) data;
	if (!components || mgl_enum(type) != QGL_UNSIGNED_BYTE || !p || width < 1) {
		mgl_unknown("GLColorTable");
		return;
	}
	if (width > 256) width = 256;
	for (i = 0; i < width; ++i, p += components) {
		for (k = 0; k < 3; ++k) palette[i][k] = p[k];
		palette[i][3] = components == 4 ? p[3] : 255;
	}
	paletteAlpha = components == 4;
}

/* A width x height image of colour indices (unsigned bytes, rows aligned to
 * the unpack alignment) as RGBA, in a buffer for FreeVec; NULL if there is
 * no memory. */
static UBYTE *indicesToRgba(const UBYTE *indices, int width, int height) {
	int row = (width + unpackAlignment - 1) & ~(unpackAlignment - 1), x, y;
	UBYTE *rgba = (UBYTE *) AllocVec(width * height * 4, MEMF_ANY), *d = rgba;
	if (!rgba) return NULL;
	for (y = 0; y < height; ++y) {
		const UBYTE *s = indices + y * row;
		for (x = 0; x < width; ++x, d += 4) {
			const UBYTE *c = palette[s[x]];
			d[0] = c[0]; d[1] = c[1]; d[2] = c[2]; d[3] = c[3];
		}
	}
	return rgba;
}

/* internalformat is a number of components (1 to 4) or a format. */
void mgl_GLTexImage2D(GLcontext context, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
		GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
	int components = internalformat >= 1 && internalformat <= 4 ? internalformat : (int) mgl_enum((unsigned int) internalformat);
	if (mgl_enum(format) == QGL_COLOR_INDEX) {
		UBYTE *rgba = pixels ? indicesToRgba((const UBYTE *) pixels, width, height) : NULL;
		if (pixels && !rgba) return;
		_glTexImage2D(mgl_enum(target), level, paletteAlpha ? QGL_RGBA : QGL_RGB, width, height, border, QGL_RGBA, QGL_UNSIGNED_BYTE, rgba);
		if (rgba) FreeVec(rgba);
		return;
	}
	_glTexImage2D(mgl_enum(target), level, components, width, height, border, mgl_enum(format), mgl_enum(type), (void *) pixels);
}

void mgl_GLTexSubImage2D(GLcontext context, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
		GLenum format, GLenum type, const GLvoid *pixels) {
	unsigned int t = mgl_enum(target), f = mgl_enum(format), ty = mgl_enum(type);
	if ((t & QT_MGL_ONLY) == QT_MGL_ONLY || (f & QT_MGL_ONLY) == QT_MGL_ONLY || (ty & QT_MGL_ONLY) == QT_MGL_ONLY) {
		mgl_unknown("GLTexSubImage2D");
		return;
	}
	if (f == QGL_COLOR_INDEX) {
		UBYTE *rgba = pixels ? indicesToRgba((const UBYTE *) pixels, width, height) : NULL;
		if (!rgba) return;
		_glTexSubImage2D(t, level, xoffset, yoffset, width, height, QGL_RGBA, QGL_UNSIGNED_BYTE, rgba);
		FreeVec(rgba);
		return;
	}
	_glTexSubImage2D(t, level, xoffset, yoffset, width, height, f, ty, (void *) pixels);
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

/* --- Vertex arrays (read on the 68k) ----------------------------------------- */

typedef struct {
	BOOL enabled;
	int size;
	unsigned int type; /* OpenGL's */
	int stride;        /* in bytes, never 0 */
	const UBYTE *pointer;
} Array;

/* Texture coordinates per unit; glTexCoordPointer and the client state of
 * GL_TEXTURE_COORD_ARRAY go to the unit glClientActiveTextureARB chose. */
static Array vertices, colors, texCoords[QT_MGL_TEXTURE_UNITS];
static int clientUnit;

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
	int u;
	vertices.enabled = colors.enabled = FALSE;
	for (u = 0; u < QT_MGL_TEXTURE_UNITS; ++u) texCoords[u].enabled = FALSE;
	clientUnit = 0;
}

void mgl_GLVertexPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
	setArray(&vertices, size, type, stride, pointer);
}

void mgl_GLColorPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
	setArray(&colors, size, type, stride, pointer);
}

void mgl_GLTexCoordPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
	setArray(&texCoords[clientUnit], size, type, stride, pointer);
}

void mgl_GLClientActiveTextureARB(GLcontext context, GLenum unit) {
	unsigned int u = mgl_enum(unit) - QGL_TEXTURE0_ARB;
	if (u < QT_MGL_TEXTURE_UNITS) clientUnit = u;
}

static Array *clientArray(GLenum cap) {
	switch (mgl_enum(cap)) {
	case QGL_VERTEX_ARRAY: return &vertices;
	case QGL_COLOR_ARRAY: return &colors;
	case QGL_TEXTURE_COORD_ARRAY: return &texCoords[clientUnit];
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

/*
 * glInterleavedArrays: OpenGL 1.1's table of formats (GL_V2F 0x2A20 to
 * GL_T4F_C4F_N3F_V4F 0x2A2D), as texture coordinate, colour and vertex sizes
 * and offsets in bytes. Normals are skipped: MiniGL has no normal array.
 */
static const struct {
	UBYTE texCoords, colors, colorBytes, colorOffset, vertices, vertexOffset, size;
} interleaved[14] = {
	{0, 0, 0, 0, 2, 0, 8}, {0, 0, 0, 0, 3, 0, 12},
	{0, 4, 1, 0, 2, 4, 12}, {0, 4, 1, 0, 3, 4, 16}, {0, 3, 0, 0, 3, 12, 24},
	{0, 0, 0, 0, 3, 12, 24}, {0, 4, 0, 0, 3, 28, 40},
	{2, 0, 0, 0, 3, 8, 20}, {4, 0, 0, 0, 4, 16, 32},
	{2, 4, 1, 8, 3, 12, 24}, {2, 3, 0, 8, 3, 20, 32}, {2, 0, 0, 0, 3, 20, 32},
	{2, 4, 0, 8, 3, 36, 48}, {4, 4, 0, 16, 4, 44, 60}
};

void mgl_GLInterleavedArrays(GLcontext context, GLenum format, GLsizei stride, const GLvoid *pointer) {
	unsigned int f = mgl_enum(format) - 0x2A20;
	const UBYTE *p = (const UBYTE *) pointer;
	if (f >= 14) {
		mgl_unknown("GLInterleavedArrays");
		return;
	}
	if (!stride) stride = interleaved[f].size;
	setArray(&vertices, interleaved[f].vertices, QGL_FLOAT, stride, p + interleaved[f].vertexOffset);
	vertices.enabled = TRUE;
	colors.enabled = interleaved[f].colors != 0;
	if (colors.enabled) {
		setArray(&colors, interleaved[f].colors, interleaved[f].colorBytes ? QGL_UNSIGNED_BYTE : QGL_FLOAT, stride,
			p + interleaved[f].colorOffset);
	}
	texCoords[clientUnit].enabled = interleaved[f].texCoords != 0;
	if (texCoords[clientUnit].enabled) setArray(&texCoords[clientUnit], interleaved[f].texCoords, QGL_FLOAT, stride, p);
}

/* Edge flags only matter to polygons drawn as lines or points, and colour
 * indices only to colour index mode, which MiniGL does not have; glEdgeFlag
 * is passed on, these arrays are not. */
void mgl_GLEdgeFlagPointer(GLcontext context, GLsizei stride, const GLvoid *pointer) {}
void mgl_GLIndexPointer(GLcontext context, GLenum type, GLsizei stride, const GLvoid *pointer) {}

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
	if (texCoords[0].enabled && texCoords[0].pointer) {
		const Array *t = &texCoords[0];
		if (t->size == 4) _glTexCoord4f(component(t, i, 0), component(t, i, 1), component(t, i, 2), component(t, i, 3));
		else _glTexCoord2f(component(t, i, 0), component(t, i, 1));
	}
	if (texCoords[1].enabled && texCoords[1].pointer) {
		multiTexCoord(QGL_TEXTURE1_ARB, component(&texCoords[1], i, 0), component(&texCoords[1], i, 1));
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

/* glDrawArrays and glDrawElements: the host reads the arrays from Amiga
 * memory (QT_MGL_DRAW, host/mgl.cpp). Synchronous, as the application may
 * change the arrays once the call returns. */
static ULONG *arrayWords(ULONG *w, const Array *a) {
	w[0] = a->enabled && a->pointer;
	w[1] = a->size;
	w[2] = a->type;
	w[3] = a->stride;
	w[4] = (ULONG) a->pointer;
	return w + 5;
}

static void drawHost(GLenum mode, GLint first, GLsizei count, unsigned int indexType, const GLvoid *indices) {
	ULONG *w;
	if (count <= 0 || !vertices.enabled || !vertices.pointer) return;
	w = qt_reserve(QT_MGL_DRAW_WORDS);
	w[0] = ((ULONG) QT_MGL_DRAW << 16) | QT_MGL_DRAW_WORDS;
	w[1] = mgl_enum(mode);
	w[2] = first;
	w[3] = count;
	w[4] = indexType;
	w[5] = (ULONG) indices;
	w = arrayWords(arrayWords(w + 6, &vertices), &colors);
	arrayWords(arrayWords(w, &texCoords[0]), &texCoords[1]);
	qt_flush();
}

void mgl_GLDrawArrays(GLcontext context, GLenum mode, GLint first, GLsizei count) {
	drawHost(mode, first, count, 0, NULL);
}

void mgl_GLMultiDrawArrays(GLcontext context, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount) {
	int i;
	for (i = 0; i < primcount; ++i) mgl_GLDrawArrays(context, mode, first[i], count[i]);
}

void mgl_GLDrawElements(GLcontext context, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {
	unsigned int t = mgl_enum(type);
	if (t != QGL_UNSIGNED_BYTE && t != QGL_UNSIGNED_SHORT) t = QGL_UNSIGNED_INT;
	if (indices) drawHost(mode, 0, count, t, indices);
}

/* GL_EXT_compiled_vertex_array: nothing to compile here. */
void mgl_GLLockArrays(GLcontext context, GLuint first, GLsizei count) {}
void mgl_GLUnlockArrays(GLcontext context) {}
