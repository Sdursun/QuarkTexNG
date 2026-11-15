#include "ogles2.h"
#include <proto/exec.h>

/*
 * OpenGL ES 3.1 (docs/phase10-ogles3.md, stage 3): compute shaders, images,
 * barriers and indirect draws. They need the host's core contexts to be
 * OpenGL 4.3 (an ES 3.1 context is refused otherwise, context.c).
 *
 * Shader storage and atomic counter buffers are fake buffers as all others;
 * glBindBufferBase/Range names them to the host (es3.c), which binds their
 * words swapped before each dispatch and draw and reads back what the GPU
 * wrote when the 68k or an attribute reads them (host/ogles2.cpp).
 *
 * The indirect commands are read on the 68k from the fake buffer bound to
 * GL_DISPATCH_INDIRECT_BUFFER or GL_DRAW_INDIRECT_BUFFER (a compute shader
 * may have written them: the host reads those back first), and run as
 * direct ones.
 */

#define GL_DISPATCH_INDIRECT_BUFFER_ 0x90EE
#define GL_DRAW_INDIRECT_BUFFER_ 0x8F3F

/* size bytes of the indirect command at offset: from the buffer bound to
 * target, or (not in OpenGL ES 3.1, but harmless) from client memory. */
static int readIndirect(struct OGLES2Context *ctx, GLenum target, const void *indirect, ULONG *words, ULONG size) {
	GLuint buffer = ogles2_boundBuffer(ctx, target);
	if (buffer) {
		ogles2_bufferCopy(QT_OGLES2_READ_BUFFER, buffer, (ULONG) indirect, size, words);
		return 1;
	}
	if (!indirect) return 0;
	CopyMem((APTR) indirect, words, size);
	return 1;
}

void ogles2_GLDispatchCompute(struct OGLES2Context *ctx, GLuint x, GLuint y, GLuint z) {
	if (!ctx || !x || !y || !z) return;
	ogles2_call(QT_OGLES2_C_DispatchCompute, x, y, z, 0, 0);
}

void ogles2_GLDispatchComputeIndirect(struct OGLES2Context *ctx, GLintptr indirect) {
	ULONG groups[3] = {0, 0, 0};
	if (!ctx || !ogles2_boundBuffer(ctx, GL_DISPATCH_INDIRECT_BUFFER_)) return;
	if (readIndirect(ctx, GL_DISPATCH_INDIRECT_BUFFER_, (const void *) indirect, groups, sizeof(groups)))
		ogles2_GLDispatchCompute(ctx, groups[0], groups[1], groups[2]);
}

void ogles2_GLMemoryBarrier(struct OGLES2Context *ctx, GLbitfield barriers) {
	ogles2_call(QT_OGLES2_C_MemoryBarrier, barriers, 0, 0, 0, 0);
}

/* OpenGL 4.3 has no glMemoryBarrierByRegion (4.5): the whole barrier, of
 * the bits it allows, does no less. */
void ogles2_GLMemoryBarrierByRegion(struct OGLES2Context *ctx, GLbitfield barriers) {
	ogles2_GLMemoryBarrier(ctx, barriers);
}

void ogles2_GLBindImageTexture(struct OGLES2Context *ctx, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer,
		GLenum access, GLenum format) {
	ULONG words[10] = {0};
	words[0] = unit;
	words[1] = texture;
	words[2] = (ULONG) level;
	words[3] = layered;
	words[4] = (ULONG) layer;
	words[5] = access;
	words[6] = format;
	ogles2_call10(QT_OGLES2_C_BindImageTexture, words);
}

/* { count, instanceCount, first, reserved (baseInstance, must be 0) } */
void ogles2_GLDrawArraysIndirect(struct OGLES2Context *ctx, GLenum mode, const void *indirect) {
	ULONG c[4] = {0, 0, 0, 0};
	if (!ctx || !readIndirect(ctx, GL_DRAW_INDIRECT_BUFFER_, indirect, c, sizeof(c)) || !c[0] || !c[1]) return;
	ogles2_draw(ctx, mode, (GLint) c[2], (GLsizei) c[0], 0, 0, (GLsizei) c[1]);
}

/* { count, instanceCount, firstIndex, baseVertex, reserved }: baseVertex
 * (not in the ES 3.1 draws without indirect either) is not offered; a
 * command with one is drawn without it. */
void ogles2_GLDrawElementsIndirect(struct OGLES2Context *ctx, GLenum mode, GLenum type, const void *indirect) {
	ULONG c[5] = {0, 0, 0, 0, 0};
	ULONG unit = type == GL_UNSIGNED_INT ? 4 : type == GL_UNSIGNED_SHORT ? 2 : 1;
	if (!ctx || !readIndirect(ctx, GL_DRAW_INDIRECT_BUFFER_, indirect, c, sizeof(c)) || !c[0] || !c[1]) return;
	ogles2_draw(ctx, mode, 0, (GLsizei) c[0], type, (const void *) (c[2] * unit), (GLsizei) c[1]);
}
