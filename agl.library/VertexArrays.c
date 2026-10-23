#include "../gl/gl.h"
#include <exec/memory.h>
#include <proto/exec.h>

/*
 * The arrays' pointers and formats, read on the 68k side for glArrayElement
 * and the like: per task (gl/gl.c's qt_taskData()[1]), as several programs
 * may use the library at once (docs/multitasking.md). Whether each array is
 * enabled lives in the host's context.
 */
typedef struct {
	GLint vertexSize;
	GLenum vertexType;
	GLsizei vertexStride;
	GLvoid *vertexPtr;
	GLenum normalType;
	GLsizei normalStride;
	GLvoid *normalPtr;
	GLint colorSize;
	GLenum colorType;
	GLsizei colorStride;
	GLvoid *colorPtr;
	GLenum indexType;
	GLsizei indexStride;
	GLvoid *indexPtr;
	GLint texCoordSize;
	GLenum texCoordType;
	GLsizei texCoordStride;
	GLvoid *texCoordPtr;
	GLsizei edgeFlagStride;
	GLboolean *edgeFlagPtr;
} Arrays;

/* The calling task's arrays, made on first use (freed by gl/gl.c with the
 * task's last context); a scratch set if there is no memory. Not static:
 * the inline element functions below use it. */
Arrays *qt_aglArrays(void) {
	static Arrays scratch;
	APTR *data = qt_taskData();
	if (!data[1]) data[1] = AllocVec(sizeof(Arrays), MEMF_ANY | MEMF_CLEAR);
	return data[1] ? (Arrays *) data[1] : &scratch;
}

#define A (qt_aglArrays())

/* Stride 0 means tightly packed elements of size components. 0.53 stepped by
 * 0 then and repeated the first element. */
static GLsizei packedStride(GLsizei stride, GLint size, GLenum type) {
	int bytes;
	if (stride) return stride;
	switch (type) {
	case GL_BYTE: case GL_UNSIGNED_BYTE: bytes = 1; break;
	case GL_SHORT: case GL_UNSIGNED_SHORT: bytes = 2; break;
	case GL_DOUBLE: bytes = 8; break;
	default: bytes = 4; break;
	}
	return size * bytes;
}


void glVertexPointer(GLint size __asm("d0"), GLenum type __asm("d1"), GLsizei stride __asm("d2"), GLvoid *ptr __asm("a0")) {
	LOG;
	A->vertexSize = size;
	A->vertexType = type;
	A->vertexStride = packedStride(stride, size, type);
	A->vertexPtr = ptr;
	_glVertexPointer(size, type, stride, memoffset + (long) ptr);
}

inline void vertex(int i) {
	GLshort *sp = NULL;
	GLint *ip = NULL;
	GLfloat *fp = NULL;
	GLdouble *dp = NULL;
	switch (A->vertexType) {
	case GL_SHORT:
		sp = (GLshort*) (A->vertexPtr + i * A->vertexStride);
		switch (A->vertexSize) {
		case 2: _glVertex2s(sp[0], sp[1]); break;
		case 3: _glVertex3s(sp[0], sp[1], sp[2]); break;
		case 4: _glVertex4s(sp[0], sp[1], sp[2], sp[3]); break;
		}
		break;
	case GL_INT:
		ip = (GLint*) (A->vertexPtr + i * A->vertexStride);
		switch (A->vertexSize) {
		case 2: _glVertex2i(ip[0], ip[1]); break;
		case 3: _glVertex3i(ip[0], ip[1], ip[2]); break;
		case 4: _glVertex4i(ip[0], ip[1], ip[2], ip[3]); break;
		}
		break;
	case GL_FLOAT:
		fp = (GLfloat*) (A->vertexPtr + i * A->vertexStride);
		switch (A->vertexSize) {
		case 2: _glVertex2f(fp[0], fp[1]); break;
		case 3: _glVertex3f(fp[0], fp[1], fp[2]); break;
		case 4: _glVertex4f(fp[0], fp[1], fp[2], fp[3]); break;
		}
		break;
	case GL_DOUBLE:
		dp = (GLdouble*) (A->vertexPtr + i * A->vertexStride);
		switch (A->vertexSize) {
		case 2: _glVertex2d(dp[0], dp[1]); break;
		case 3: _glVertex3d(dp[0], dp[1], dp[2]); break;
		case 4: _glVertex4d(dp[0], dp[1], dp[2], dp[3]); break;
		}
		break;
	}
}


void glNormalPointer(GLenum type __asm("d0"), GLsizei stride __asm("d1"), GLvoid *ptr __asm("a0")) {
	LOG;
	A->normalType = type;
	A->normalStride = packedStride(stride, 3, type);
	A->normalPtr = ptr;
	_glNormalPointer(type, stride, memoffset + (long) ptr);
}

inline void normal(int i) {
	GLbyte *bp = NULL;
	GLshort *sp = NULL;
	GLint *ip = NULL;
	GLfloat *fp = NULL;
	GLdouble *dp = NULL;
	switch (A->normalType) {
	case GL_BYTE:
		bp = (GLbyte*) (A->normalPtr + i * A->normalStride);
		_glNormal3b(bp[0], bp[1], bp[2]);
		break;
	case GL_SHORT:
		sp = (GLshort*) (A->normalPtr + i * A->normalStride);
		_glNormal3s(sp[0], sp[1], sp[2]);
		break;
	case GL_INT:
		ip = (GLint*) (A->normalPtr + i * A->normalStride);
		_glNormal3i(ip[0], ip[1], ip[2]);
		break;
	case GL_FLOAT:
		fp = (GLfloat*) (A->normalPtr + i * A->normalStride);
		_glNormal3f(fp[0], fp[1], fp[2]);
		break;
	case GL_DOUBLE:
		dp = (GLdouble*) (A->normalPtr + i * A->normalStride);
		_glNormal3d(dp[0], dp[1], dp[2]);
		break;
	}
}


void glColorPointer(GLint size __asm("d0"), GLenum type __asm("d1"), GLsizei stride __asm("d2"), GLvoid *ptr __asm("a0")) {
	LOG;
	A->colorSize = size;
	A->colorType = type;
	A->colorStride = packedStride(stride, size, type);
	A->colorPtr = ptr;
	_glColorPointer(size, type, stride, memoffset + (long) ptr);
}


inline void color(int i) {
	GLbyte *bp = NULL;
	GLubyte *ubp = NULL;
	GLshort *sp = NULL;
	GLushort *usp = NULL;
	GLint *ip = NULL;
	GLuint *uip = NULL;
	GLfloat *fp = NULL;
	GLdouble *dp = NULL;
	switch (A->colorType) {
	case GL_BYTE:
		bp = (GLbyte*) (A->colorPtr + i * A->colorStride);
		switch (A->colorSize) {
		case 3: _glColor3b(bp[0], bp[1], bp[2]); break;
		case 4: _glColor4b(bp[0], bp[1], bp[2], bp[3]); break;
		}
		break;
	case GL_UNSIGNED_BYTE:
		ubp = (GLubyte*) (A->colorPtr + i * A->colorStride);
		switch (A->colorSize) {
		case 3: _glColor3ub(ubp[0], ubp[1], ubp[2]); break;
		case 4: _glColor4ub(ubp[0], ubp[1], ubp[2], ubp[3]); break;
		}
		break;
	case GL_SHORT:
		sp = (GLshort*) (A->colorPtr + i * A->colorStride);
		switch (A->colorSize) {
		case 3: _glColor3s(sp[0], sp[1], sp[2]); break;
		case 4: _glColor4s(sp[0], sp[1], sp[2], sp[3]); break;
		}
		break;
	case GL_UNSIGNED_SHORT:
		usp = (GLushort*) (A->colorPtr + i * A->colorStride);
		switch (A->colorSize) {
		case 3: _glColor3us(usp[0], usp[1], usp[2]); break;
		case 4: _glColor4us(usp[0], usp[1], usp[2], usp[3]); break;
		}
		break;
	case GL_INT:
		ip = (GLint*) (A->colorPtr + i * A->colorStride);
		switch (A->colorSize) {
		case 3: _glColor3i(ip[0], ip[1], ip[2]); break;
		case 4: _glColor4i(ip[0], ip[1], ip[2], ip[3]); break;
		}
		break;
	case GL_UNSIGNED_INT:
		uip = (GLuint*) (A->colorPtr + i * A->colorStride);
		switch (A->colorSize) {
		case 3: _glColor3ui(uip[0], uip[1], uip[2]); break;
		case 4: _glColor4ui(uip[0], uip[1], uip[2], uip[3]); break;
		}
		break;
	case GL_FLOAT:
		fp = (GLfloat*) (A->colorPtr + i * A->colorStride);
		switch (A->colorSize) {
		case 3: _glColor3f(fp[0], fp[1], fp[2]); break;
		case 4: _glColor4f(fp[0], fp[1], fp[2], fp[3]); break;
		}
		break;
	case GL_DOUBLE:
		dp = (GLdouble*) (A->colorPtr + i * A->colorStride);
		switch (A->colorSize) {
		case 3: _glColor3d(dp[0], dp[1], dp[2]); break;
		case 4: _glColor4d(dp[0], dp[1], dp[2], dp[3]); break;
		}
		break;
	}
}


void glIndexPointer(GLenum type __asm("d0"), GLsizei stride __asm("d1"), GLvoid *ptr __asm("a0")) {
	LOG;
	A->indexType = type;
	A->indexStride = packedStride(stride, 1, type);
	A->indexPtr = ptr;
	_glIndexPointer(type, stride, memoffset + (long) ptr);
}

inline void index(int i) {
	GLubyte *ubp = NULL;
	GLshort *sp = NULL;
	GLint *ip = NULL;
	GLfloat *fp = NULL;
	GLdouble *dp = NULL;
	switch (A->indexType) {
	case GL_UNSIGNED_BYTE:
		ubp = (GLubyte*) (A->indexPtr + i * A->indexStride);
		_glIndexub(bp[0]);
		break;
	case GL_SHORT:
		sp = (GLshort*) (A->indexPtr + i * A->indexStride);
		_glIndexs(sp[0]);
		break;
	case GL_INT:
		ip = (GLint*) (A->indexPtr + i * A->indexStride);
		_glIndexi(ip[0]);
		break;
	case GL_FLOAT:
		fp = (GLfloat*) (A->indexPtr + i * A->indexStride);
		_glIndexf(fp[0]);
		break;
	case GL_DOUBLE:
		dp = (GLdouble*) (A->indexPtr + i * A->indexStride);
		_glIndexd(dp[0]);
		break;
	}
}


void glTexCoordPointer(GLint size __asm("d0"), GLenum type __asm("d1"), GLsizei stride __asm("d2"), GLvoid *ptr __asm("a0")) {
	LOG;
	A->texCoordSize = size;
	A->texCoordType = type;
	A->texCoordStride = packedStride(stride, size, type);
	A->texCoordPtr = ptr;
	_glTexCoordPointer(size, type, stride, memoffset + (long) ptr);
}

inline void texCoord(int i) {
	GLshort *sp = NULL;
	GLint *ip = NULL;
	GLfloat *fp = NULL;
	GLdouble *dp = NULL;
	switch (A->texCoordType) {
	case GL_SHORT:
		sp = (GLshort*) (A->texCoordPtr + i * A->texCoordStride);
		switch (A->texCoordSize) {
		case 1: _glTexCoord1s(sp[0]); break;
		case 2: _glTexCoord2s(sp[0], sp[1]); break;
		case 3: _glTexCoord3s(sp[0], sp[1], sp[2]); break;
		case 4: _glTexCoord4s(sp[0], sp[1], sp[2], sp[3]); break;
		}
		break;
	case GL_INT:
		ip = (GLint*) (A->texCoordPtr + i * A->texCoordStride);
		switch (A->texCoordSize) {
		case 1: _glTexCoord1i(ip[0]); break;
		case 2: _glTexCoord2i(ip[0], ip[1]); break;
		case 3: _glTexCoord3i(ip[0], ip[1], ip[2]); break;
		case 4: _glTexCoord4i(ip[0], ip[1], ip[2], ip[3]); break;
		}
		break;
	case GL_FLOAT:
		fp = (GLfloat*) (A->texCoordPtr + i * A->texCoordStride);
		switch (A->texCoordSize) {
		case 1: _glTexCoord1f(fp[0]); break;
		case 2: _glTexCoord2f(fp[0], fp[1]); break;
		case 3: _glTexCoord3f(fp[0], fp[1], fp[2]); break;
		case 4: _glTexCoord4f(fp[0], fp[1], fp[2], fp[3]); break;
		}
		break;
	case GL_DOUBLE:
		dp = (GLdouble*) (A->texCoordPtr + i * A->texCoordStride);
		switch (A->texCoordSize) {
		case 1: _glTexCoord1d(dp[0]); break;
		case 2: _glTexCoord2d(dp[0], dp[1]); break;
		case 3: _glTexCoord3d(dp[0], dp[1], dp[2]); break;
		case 4: _glTexCoord4d(dp[0], dp[1], dp[2], dp[3]); break;
		}
		break;
	}
}

void glEdgeFlagPointer(GLsizei stride __asm("d0"), GLboolean *ptr __asm("a0")) {
	LOG;
	A->edgeFlagStride = stride ? stride : (GLsizei) sizeof(GLboolean);
	A->edgeFlagPtr = ptr;
	_glEdgeFlagPointer(stride, memoffset + (long) ptr);
}

inline void edgeFlag(int i) {
	GLboolean *bp = (GLboolean*) (A->edgeFlagPtr + i * A->edgeFlagStride);
	_glEdgeFlag(bp[0]);
}

void glGetPointerv(GLenum pname __asm("d0"), GLvoid *params __asm("a0")) {
	LOG;
	_glGetPointerv(pname, memoffset + (long) params);
	SWAP32(params, 1);
}
void glArrayElement(GLint i __asm("d0")) {
	LOG;
	if (_glIsEnabled(GL_EDGE_FLAG_ARRAY)) edgeFlag(i);
	if (_glIsEnabled(GL_TEXTURE_COORD_ARRAY)) texCoord(i);
	if (_glIsEnabled(GL_INDEX_ARRAY)) index(i);
	if (_glIsEnabled(GL_COLOR_ARRAY)) color(i);
	if (_glIsEnabled(GL_NORMAL_ARRAY)) normal(i);
	if (_glIsEnabled(GL_VERTEX_ARRAY)) vertex(i);
}

void glDrawArrays(GLenum mode __asm("d0"), GLint first __asm("d1"), GLsizei count __asm("d2")) {
	int i;
	GLboolean edgeFlagOn = _glIsEnabled(GL_EDGE_FLAG_ARRAY);
	GLboolean texCoordOn = _glIsEnabled(GL_TEXTURE_COORD_ARRAY);
	GLboolean indexOn = _glIsEnabled(GL_INDEX_ARRAY);
	GLboolean colorOn = _glIsEnabled(GL_COLOR_ARRAY);
	GLboolean normalOn = _glIsEnabled(GL_NORMAL_ARRAY);
	GLboolean vertexOn = _glIsEnabled(GL_VERTEX_ARRAY);
	LOG;
	_glBegin(mode);
	for (i = first; i < first + count; ++i) {
		if (edgeFlagOn) edgeFlag(i);
		if (texCoordOn) texCoord(i);
		if (indexOn) index(i);
		if (colorOn) color(i);
		if (normalOn) normal(i);
		if (vertexOn) vertex(i);
	}
	_glEnd();
}

void glDrawElements(GLenum mode __asm("d0"), GLsizei count __asm("d1"), GLenum type __asm("d2"), GLvoid *indices __asm("a0")) {
	GLubyte *ubp = NULL;
	GLushort *usp = NULL;
	GLuint *uip = NULL;
	int i;
	GLboolean edgeFlagOn = _glIsEnabled(GL_EDGE_FLAG_ARRAY);
	GLboolean texCoordOn = _glIsEnabled(GL_TEXTURE_COORD_ARRAY);
	GLboolean indexOn = _glIsEnabled(GL_INDEX_ARRAY);
	GLboolean colorOn = _glIsEnabled(GL_COLOR_ARRAY);
	GLboolean normalOn = _glIsEnabled(GL_NORMAL_ARRAY);
	GLboolean vertexOn = _glIsEnabled(GL_VERTEX_ARRAY);
	LOG;
	_glBegin(mode);
	switch (type) {
	case GL_UNSIGNED_BYTE:
		ubp = (GLubyte*) indices;
		for (i = 0; i < count; ++i) {
			if (edgeFlagOn) edgeFlag(ubp[i]);
			if (texCoordOn) texCoord(ubp[i]);
			if (indexOn) index(ubp[i]);
			if (colorOn) color(ubp[i]);
			if (normalOn) normal(ubp[i]);
			if (vertexOn) vertex(ubp[i]);
		}
		break;
	case GL_UNSIGNED_SHORT:
		usp = (GLushort*) indices;
		for (i = 0; i < count; ++i) {
			if (edgeFlagOn) edgeFlag(usp[i]);
			if (texCoordOn) texCoord(usp[i]);
			if (indexOn) index(usp[i]);
			if (colorOn) color(usp[i]);
			if (normalOn) normal(usp[i]);
			if (vertexOn) vertex(usp[i]);
		}
		break;
	case GL_UNSIGNED_INT:
		uip = (GLuint*) indices;
		for (i = 0; i < count; ++i) {
			if (edgeFlagOn) edgeFlag(uip[i]);
			if (texCoordOn) texCoord(uip[i]);
			if (indexOn) index(uip[i]);
			if (colorOn) color(uip[i]);
			if (normalOn) normal(uip[i]);
			if (vertexOn) vertex(uip[i]);
		}
		break;
	}
	_glEnd();
}
void glInterleavedArrays(GLenum format __asm("d0"), GLsizei stride __asm("d1"), GLvoid *pointer __asm("a0")) {
	LOG;
	//_glInterleavedArrays(format, stride, pointer);
}