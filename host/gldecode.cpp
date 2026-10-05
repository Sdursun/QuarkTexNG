// Executes the OpenGL command buffer written by the 68k side. The dispatch is
// generated (gldecode.auto.inc); the functions that need the Amiga addresses of
// their buffers later are implemented here.
//
// The unit test (tests/host) includes this file with QT_TEST defined and
// QT_GL pointing to recording stubs.
#ifndef QT_TEST
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#define QT_GL(name) gl##name
#endif
#include <cstdio>
#include "gldecode.h"
#include "w3dcmd.h"

namespace {
	int32_t qt_bad_command(const Command& c) {
		char message[96];
		snprintf(message, sizeof(message), "Bad command: opcode %u, %u words",
			static_cast<unsigned>(c.u(0) >> 16), static_cast<unsigned>(c.words));
		qt_report(message);
		return 0;
	}

	// Amiga addresses of the buffers OpenGL keeps pointers to.
	enum {
		VertexArray, NormalArray, ColorArray, IndexArray, TexCoordArray, EdgeFlagArray,
		FeedbackBuffer, SelectionBuffer, Buffers
	};
	uint32_t addresses[Buffers];

	int32_t qt_manual_VertexPointer(const Command& c) {
		addresses[VertexArray] = c.u(4);
		QT_GL(VertexPointer)((GLint) (int32_t) c.u(1), (GLenum) c.u(2), (GLsizei) (int32_t) c.u(3), c.p(4));
		return 0;
	}

	int32_t qt_manual_NormalPointer(const Command& c) {
		addresses[NormalArray] = c.u(3);
		QT_GL(NormalPointer)((GLenum) c.u(1), (GLsizei) (int32_t) c.u(2), c.p(3));
		return 0;
	}

	int32_t qt_manual_ColorPointer(const Command& c) {
		addresses[ColorArray] = c.u(4);
		QT_GL(ColorPointer)((GLint) (int32_t) c.u(1), (GLenum) c.u(2), (GLsizei) (int32_t) c.u(3), c.p(4));
		return 0;
	}

	int32_t qt_manual_IndexPointer(const Command& c) {
		addresses[IndexArray] = c.u(3);
		QT_GL(IndexPointer)((GLenum) c.u(1), (GLsizei) (int32_t) c.u(2), c.p(3));
		return 0;
	}

	int32_t qt_manual_TexCoordPointer(const Command& c) {
		addresses[TexCoordArray] = c.u(4);
		QT_GL(TexCoordPointer)((GLint) (int32_t) c.u(1), (GLenum) c.u(2), (GLsizei) (int32_t) c.u(3), c.p(4));
		return 0;
	}

	int32_t qt_manual_EdgeFlagPointer(const Command& c) {
		addresses[EdgeFlagArray] = c.u(2);
		QT_GL(EdgeFlagPointer)((GLsizei) (int32_t) c.u(1), c.p(2));
		return 0;
	}

	// Sets several arrays inside one block. GetPointerv answers with the start
	// of the block for each of them; the offsets inside it are not tracked.
	int32_t qt_manual_InterleavedArrays(const Command& c) {
		addresses[VertexArray] = addresses[NormalArray] = addresses[ColorArray] = addresses[TexCoordArray] = c.u(3);
		QT_GL(InterleavedArrays)((GLenum) c.u(1), (GLsizei) (int32_t) c.u(2), c.p(3));
		return 0;
	}

	int32_t qt_manual_FeedbackBuffer(const Command& c) {
		addresses[FeedbackBuffer] = c.u(3);
		QT_GL(FeedbackBuffer)((GLsizei) (int32_t) c.u(1), (GLenum) c.u(2), c.p(3));
		return 0;
	}

	int32_t qt_manual_SelectBuffer(const Command& c) {
		addresses[SelectionBuffer] = c.u(2);
		QT_GL(SelectBuffer)((GLsizei) (int32_t) c.u(1), c.p(2));
		return 0;
	}

	// OpenGL would return host pointers. Write the Amiga address instead, as
	// a little-endian word like every other result (agl.library swaps it).
	int32_t qt_manual_GetPointerv(const Command& c) {
		uint32_t address = 0;
		switch (c.u(1)) {
		case GL_VERTEX_ARRAY_POINTER: address = addresses[VertexArray]; break;
		case GL_NORMAL_ARRAY_POINTER: address = addresses[NormalArray]; break;
		case GL_COLOR_ARRAY_POINTER: address = addresses[ColorArray]; break;
		case GL_INDEX_ARRAY_POINTER: address = addresses[IndexArray]; break;
		case GL_TEXTURE_COORD_ARRAY_POINTER: address = addresses[TexCoordArray]; break;
		case GL_EDGE_FLAG_ARRAY_POINTER: address = addresses[EdgeFlagArray]; break;
		case GL_FEEDBACK_BUFFER_POINTER: address = addresses[FeedbackBuffer]; break;
		case GL_SELECTION_BUFFER_POINTER: address = addresses[SelectionBuffer]; break;
		}
		uint8_t* params = c.p(2);
		if (params) memcpy(params, &address, 4);
		return 0;
	}

	// A host pointer means nothing on the Amiga side; agl.library answers
	// glGetString itself.
	int32_t qt_manual_GetString(const Command&) {
		return 0;
	}
}

int32_t qt_decode(const uint8_t* buffer, uint32_t bytes, QtResolver resolve) {
	int32_t result = 0;
	uint32_t offset = 0;
	while (offset + 4 <= bytes) {
		Command c = {buffer + offset, 0, resolve};
		uint32_t header = c.u(0);
		c.words = header & 0xFFFF;
		if (c.words == 0 || offset + 4 * c.words > bytes) return qt_bad_command(c);
		result = 0;
		if ((header >> 16) < QT_W3D_FIRST) qt_w3d_sync();
		switch (header >> 16) {
#include "gldecode.auto.inc"
		default:
			if ((header >> 16) < QT_W3D_FIRST || !qt_w3d_decode(c, result)) return qt_bad_command(c);
			break;
		}
		// Tracing: which OpenGL command an error comes from (opcode = line in
		// gl/glFuncs.txt), with its first arguments. Not between glBegin (5)
		// and glEnd (75), where glGetError is an error itself.
		static bool insideBegin = false;
		if (qt_w3d_trace_all && (header >> 16) == 5) insideBegin = true;
		else if ((header >> 16) == 75) insideBegin = false;
		if (qt_w3d_trace_all && !insideBegin && (header >> 16) < QT_W3D_FIRST) {
			GLenum error = QT_GL(GetError)();
			if (error) {
				char line[120];
				snprintf(line, sizeof(line), "GL error 0x%X after opcode %u: %X %X %X", error, header >> 16,
					c.words > 1 ? c.u(1) : 0, c.words > 2 ? c.u(2) : 0, c.words > 3 ? c.u(3) : 0);
				qt_report(line);
			}
		}
		offset += 4 * c.words;
	}
	return result;
}
