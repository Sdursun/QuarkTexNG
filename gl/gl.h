#ifndef UAE_GL
#define UAE_GL

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;

#include <exec/libraries.h>

void glInit(void);
void glExit(void);
/* Host contexts. createContext returns the new context's id (0 on failure)
 * and selects it. flags: QT_CONTEXT_CORE for an OpenGL 3.3 core profile
 * context (Warp3D, drawn by the host's emulation), 0 for a compatibility
 * one (agl, which passes OpenGL 1.1 calls on). Commands go to the selected
 * context, as do moveWindow, swapBuffers and freeContext (which leaves none
 * selected); selectContext flushes the buffer when it switches. */
ULONG createContext(int left, int top, int width, int height, int flags);
#define QT_CONTEXT_CORE 1
/* An OpenGL compatibility context as OpenGL makes it: no QuarkTex 0.53 model
 * view matrix, and a 24-bit depth buffer (minigl.library). */
#define QT_CONTEXT_PLAIN 2
/* The context draws into a picture of its own (width x height) that
 * swapBuffersTo writes into Amiga display memory, where the emulator shows it
 * like any other graphics, in a window or fullscreen; the host window stays
 * hidden. moveWindow gives the picture a new size. */
#define QT_CONTEXT_OFFSCREEN 4
void selectContext(ULONG id);
void moveWindow(int left, int top, int width, int height);
void freeContext(void);
void swapBuffers(void);
/* Where swapBuffersTo writes the picture: a bitmap of a Picasso96 RGBFormat
 * (RGBFB_R8G8B8 to RGBFB_B5G5R5PC) at address, and the rectangle in it. */
typedef struct {
	ULONG address, bytesPerRow, format, bitmapWidth, bitmapHeight;
	LONG left, top, width, height;
} QtTarget;
void swapBuffersTo(const QtTarget *target);
/* The host writes an offscreen picture as soon as the GPU has finished it,
 * between the next frame's commands; finishFrame has it written now (before
 * waiting for input, when no commands would follow). */
void finishFrame(void);
/* Whether the host can write frames into the bitmap: Picasso96's, of a format
 * RGBFB_R8G8B8 to RGBFB_B5G5R5PC (host/present.h). */
struct BitMap;
struct Layer;
int presentable(struct BitMap *bitmap);
/* Has the host write the frame into the rectangle of the bitmap, which it
 * locks, and the layer (if any) drawing there. wait: it is written before
 * presentInto returns (else as soon as the GPU has it, see finishFrame), for
 * applications that may stop drawing after any frame (agl, Warp3D). */
void presentInto(struct BitMap *bitmap, struct Layer *layer, LONG left, LONG top, LONG width, LONG height, int wait);
void logString(char* c);
/* 0 if the FPU does not compute with 80-bit extended precision (an emulator
 * set to 64 bits): glInit logs a warning. */
extern int qt_fpu_extended;

/* Command buffer (gl/gl.c). qt_reserve returns room for words 32-bit words;
 * qt_flush executes the buffer on the host and returns the last result. */
ULONG *qt_reserve(ULONG words);
ULONG qt_flush(void);

#include "gldefines.h"
#include "gldeclarations.auto.h"

extern long memoffset;
extern char *bp, b;
extern int i;

#define LOG /*												\
	logString((char*) (memoffset + (int)(__FILE__)));	\
	logString((char*) (memoffset + (int)(": ")));		\
	logString((char*) (memoffset + (int)(__FUNC__)));	\
	logString((char*) (memoffset + (int)("\n")))*/

/* Byte order of count 16-, 32- or 64-bit elements, in place. The element size
 * comes from the macro, not from the pointer type, so void pointers work too
 * (0.53 stepped through those one byte at a time). */
#define SWAP16(array, count)		\
	for (i = 0; i < (count); ++i) {	\
		bp = (char*) (array) + 2 * i;	\
		b = bp[0];					\
		bp[0] = bp[1];				\
		bp[1] = b;					\
	}

#define SWAP32(array, count)		\
	for (i = 0; i < (count); ++i) {	\
		bp = (char*) (array) + 4 * i;	\
		b = bp[0];					\
		bp[0] = bp[3];				\
		bp[3] = b;					\
		b = bp[1];					\
		bp[1] = bp[2];				\
		bp[2] = b;					\
	}

#define SWAP64(array, count)		\
	for (i = 0; i < (count); ++i) {	\
		bp = (char*) (array) + 8 * i;	\
		b = bp[0];					\
		bp[0] = bp[7];				\
		bp[7] = b;					\
		b = bp[1];					\
		bp[1] = bp[6];				\
		bp[6] = b;					\
		b = bp[2];					\
		bp[2] = bp[5];				\
		bp[5] = b;					\
		b = bp[3];					\
		bp[3] = bp[4];				\
		bp[4] = b;					\
	}

//#define PUSHREGS
//#define POPREGS
//#define PUSHREGS __asm("movem.l a2-a6/d2-d7,-(a7)"); __asm("fmovem.x fp0-fp7,-(a7)")
//#define POPREGS __asm("fmovem.x (a7)+,fp0-fp7"); __asm("movem.l (a7)+,a2-a6/d2-d7")
//#define PUSHREGS __asm("movem.l a2-a6/d2-d7,-(a7)")
//#define POPREGS __asm("movem.l (a7)+,a2-a6/d2-d7")

#endif