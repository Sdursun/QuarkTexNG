/*
 * QuarkTex's ogles2.library (docs/phase9-ogles2.md): what its hand-written
 * parts share. GLES2's tokens are already OpenGL's own values (no MiniGL-
 * style translation needed); the host runs a real OpenGL 3.3 core context
 * (QT_CONTEXT_CORE, as Warp3D does), so GLES2's shader-only model needs none
 * of minigl/agl's fixed-function emulation either.
 */
#ifndef QUARKTEX_OGLES2_H
#define QUARKTEX_OGLES2_H

#define GL_GLES_PROTOTYPES 0
#ifdef QT_OGLES3
/* ogles3.library (docs/phase10-ogles3.md): the same sources, built against
 * the ogles3 SDK. Its <libraries/ogles3.h> keeps ogles2's OGLES2_CCT_ and
 * OGLES2_CCE_ names; the context and dispatch table get theirs here. */
#include <GLES3/gl31.h>
#include <libraries/ogles3.h>
#include <libraries/ogles3_dispatch.h>
#define OGLES2Context OGLES3Context
#define OGLES2Dispatch OGLES3Dispatch
#define OGLES2_DISPATCH_ABI_VERSION OGLES3_DISPATCH_ABI_VERSION
#else
#include <GLES2/gl2.h>
#include <libraries/ogles2.h>
#include <libraries/ogles2_dispatch.h>
#endif
#include <intuition/intuition.h>
#include "oglescmd.h"

/* GLES2 has no double-precision entries, so <GLES2/gl2.h> defines neither:
 * gl/gldeclarations.auto.h (desktop GL 1.1's full set, Warp3D/agl's own)
 * needs them to declare the handful of _gl<Name> functions this library
 * calls by hand (glfuncs.c); the ones GLES2 actually has never use them. */
typedef double GLdouble;
typedef double GLclampd;

/* gl/gl.c (host contexts and the command buffer). */
void glInit(void);
void glExit(void);
ULONG createContext(int left, int top, int width, int height, int flags);
ULONG createSharedContext(int left, int top, int width, int height, int flags, ULONG share);
void selectContext(ULONG id);
void useContext(ULONG id);
void moveWindow(int left, int top, int width, int height);
void freeContext(void);
void swapBuffers(void);
struct BitMap;
struct Layer;
int presentable(struct BitMap *bitmap);
void presentInto(struct BitMap *bitmap, struct Layer *layer, LONG left, LONG top, LONG width, LONG height, int wait);
void finishFrame(void);
void logString(char *c);
/* Command buffer (gl/gl.c): qt_reserve returns room for words 32-bit words
 * (the header word, (opcode << 16) | words, and the arguments, big-endian);
 * qt_flush executes the buffer on the host and returns the last result. */
ULONG *qt_reserve(ULONG words);
ULONG qt_flush(void);
/* Two pointers kept per task (gl/gl.c): [0] is the task's current context. */
APTR *qt_taskData(void);
/* Whether the task still runs (is ready or waiting). */
struct Task;
int qt_alive(struct Task *task);
#define QT_CONTEXT_CORE 1
#define QT_CONTEXT_OFFSCREEN 4

/* A context. <libraries/ogles2_dispatch.h> forward-declares "struct
 * OGLES2Context; /@ opaque @/" -- applications only ever pass the pointer
 * back, so this gives that tag its real body, as MiniGL's own headers do
 * for GLcontext/GLcontext_t. */
/* glVertexAttribPointer's state (glfuncs.c): on the 68k side only, sent
 * with each draw (gl/oglescmd.h). OpenGL ES 3.1 splits an attribute into
 * its format and a vertex buffer binding it reads (glVertexAttribFormat,
 * glBindVertexBuffer, ogles3.library/es31.c); stride, bufferName, pointer
 * and divisor are what the draw needs, the binding's values with the
 * attribute's relative offset added, made again whenever either changes
 * (ogles2_bindingChanged). glVertexAttribPointer sets binding index's. */
typedef struct {
	GLboolean enabled;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	GLuint bufferName;  /* the one bound to GL_ARRAY_BUFFER at the call, or 0 */
	ULONG pointer;       /* an offset into it, or an Amiga address */
	GLboolean integer;   /* glVertexAttribIPointer's (OpenGL ES 3.0) */
	GLuint divisor;      /* glVertexAttribDivisor's (OpenGL ES 3.0) */
	GLuint relativeOffset; /* glVertexAttribFormat's (OpenGL ES 3.1) */
	GLuint binding;        /* the binding it reads: its own index unless glVertexAttribBinding changed it */
} QtOgles2Attrib;

/* A vertex buffer binding (OpenGL ES 3.1): glBindVertexBuffer's buffer,
 * offset and stride, glVertexBindingDivisor's divisor. */
typedef struct {
	GLuint buffer;
	ULONG offset;
	GLsizei stride;
	GLuint divisor;
} QtOgles2Binding;

/* A vertex array object (OpenGL ES 3.0, ogles3.library/es3.c): what
 * glBindVertexArray switches, kept on the 68k side as the attributes are. */
/* glMapBufferOES/glMapBufferRange copies, one per buffer target (ogles3.library/
 * es3.c, bufferSlot3; glfuncs.c's bufferSlot has the first two). */
#define QT_OGLES2_MAP_SLOTS 12

typedef struct QtOgles2Vao {
	struct QtOgles2Vao *next;
	GLuint name;
	QtOgles2Attrib attribs[QT_OGLES2_MAX_ATTRIBS];
	QtOgles2Binding bindings[QT_OGLES2_MAX_ATTRIBS];
	GLuint elementArrayBuffer;
} QtOgles2Vao;

struct OGLES2Context {
	ULONG host;
	struct Window *window;   /* NULL for a bitmap context */
	struct Screen *screen;   /* ours, in fullscreen (OGLES2_CCT_MODEID) */
	struct BitMap *bitmap;   /* an application bitmap context's */
	BOOL offscreen;          /* presented into Amiga display memory */
	BOOL ownWindow;
	BOOL ownScreen;
	int left, top, width, height;
	ULONG esVersion;         /* 20, or 30/31 for an OpenGL ES 3.0/3.1 context (ogles3.library) */
	/* GLES2 state the 68k side keeps (glfuncs.c): per context, as several
	 * programs may use the library at once (docs/multitasking.md). */
	QtOgles2Attrib attribs[QT_OGLES2_MAX_ATTRIBS];
	QtOgles2Binding bindings[QT_OGLES2_MAX_ATTRIBS]; /* the vertex buffer bindings (OpenGL ES 3.1) */
	GLfloat currentAttrib[QT_OGLES2_MAX_ATTRIBS][4]; /* glVertexAttrib4f's, for glGetVertexAttribfv */
	GLuint arrayBuffer, elementArrayBuffer;          /* the bound buffers */
	void *mapped[QT_OGLES2_MAP_SLOTS];               /* glMapBufferOES's/glMapBufferRange's copies, by target */
	ULONG mappedSize[QT_OGLES2_MAP_SLOTS];
	/* OpenGL ES 3.0 (ogles3.library/es3.c). The live attribute state is
	 * attribs/elementArrayBuffer above; glBindVertexArray copies it out to
	 * the object bound (vao, NULL: the default one, kept in defaultVao) and
	 * the new one's in. */
	GLuint uniformBuffer, copyReadBuffer, copyWriteBuffer, pixelPackBuffer, pixelUnpackBuffer, transformFeedbackBuffer;
	GLuint shaderStorageBuffer, atomicCounterBuffer, dispatchIndirectBuffer, drawIndirectBuffer; /* ES 3.1 */
	QtOgles2Vao *vaos, *vao, defaultVao;
	GLuint nextVao;
	ULONG mappedOffset[QT_OGLES2_MAP_SLOTS];         /* glMapBufferRange's offset, by target */
	GLbitfield mappedAccess[QT_OGLES2_MAP_SLOTS];
	/* An error the 68k side raised, given by glGetError before the host's
	 * (ogles2_error). */
	GLenum error;
	/* GL_KHR_debug (ogles3.library/es3more.c): the application's callback,
	 * which gets its own messages (glDebugMessageInsertKHR, debug groups);
	 * the host's are not passed on. */
	void *debugCallback;
	const void *debugUserParam;
	int debugDepth;
};

/* glfuncs.c: records an error for glGetError (the first one is kept). */
void ogles2_error(struct OGLES2Context *ctx, GLenum error);
/* glCompressedTex*Image* (QT_OGLES2_COMPRESSED_TEX). */
void ogles2_compressedTex(ULONG subop, GLenum target, GLint level, ULONG xOrFormat, GLint y, GLint z, GLsizei width,
	GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);

/* The calling task's current context (aglMakeCurrent). */
#define ogles2_current (*(struct OGLES2Context **) &qt_taskData()[0])

/* lib.c: an entry the library does not implement yet. */
void ogles2_missing(const char *name);

/* glfuncs.c's command helpers, shared with ogles3.library's files. */
ULONG ogles2_floatWord(GLfloat f);
LONG ogles2_simple(ULONG subop, ULONG w0, ULONG w1);
LONG ogles2_nameCall(ULONG subop, ULONG program, ULONG index, const char *text);
void ogles2_uniform(ULONG subop, GLint location, ULONG w0, ULONG w1, ULONG w2, ULONG w3);
void ogles2_uniformV(ULONG subop, GLint location, GLsizei count, const void *value, GLboolean transpose);
void ogles2_genObjects(ULONG kind, GLsizei n, GLuint *names);
void ogles2_deleteObjects(ULONG kind, GLsizei n, const GLuint *names);
LONG ogles2_call(ULONG subop, ULONG w0, ULONG w1, ULONG w2, ULONG w3, ULONG w4);
LONG ogles2_call10(ULONG subop, const ULONG *words);
void ogles2_bufferCopy(ULONG opcode, GLuint buffer, ULONG offset, ULONG size, const void *data);
void ogles2_getParam(ULONG subop, ULONG w0, ULONG w1, ULONG w2, void *out);
void ogles2_getActive(ULONG subop, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type,
	GLchar *name);
GLuint ogles2_boundBuffer(struct OGLES2Context *ctx, GLenum target);
/* Each attribute reading its own binding (a new context's, a new vertex
 * array object's); the draw values of the attributes reading binding b
 * made again after it or their format changed. */
void ogles2_resetAttribs(QtOgles2Attrib *attribs);
void ogles2_bindingChanged(struct OGLES2Context *ctx, GLuint b);
void ogles2_draw(struct OGLES2Context *ctx, GLenum mode, GLint first, GLsizei count, GLenum indexType, const void *indices,
	GLsizei instances);

#endif
