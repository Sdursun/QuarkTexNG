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
#include <vector>
#include "gldecode.h"
#include "w3dcmd.h"
#include "mglcmd.h"
#include "oglescmd.h"

namespace {
	// minigl.library passes MiniGL's packed texel formats, MGL_UNSIGNED_SHORT_
	// 4_4_4_4 and MGL_UNSIGNED_SHORT_5_6_5 (OpenGL's packed type values), as
	// the format and the internal format, with GL_UNSIGNED_BYTE: GLQuake's
	// lightmaps. Each texel is a big-endian word, a r g b 4 bits each or
	// r g b 5 6 5. OpenGL has no such format and refused them (the lightmaps
	// were missing); they are uploaded as RGBA bytes.
	const GLenum mglPacked4444 = 0x8033, mglPacked565 = 0x8363;

	bool packedFormat(GLenum format) {
		return format == mglPacked4444 || format == mglPacked565;
	}

	// Rows as the unpack state lays them out; the upload's state is set to
	// tight RGBA rows and put back by uploadPacked.
	std::vector<uint8_t> packedToRgba(GLenum format, GLsizei width, GLsizei height, const uint8_t* pixels) {
		std::vector<uint8_t> rgba;
		if (!pixels || width <= 0 || height <= 0) return rgba;
		GLint rowLength = 0, alignment = 4;
		QT_GL(GetIntegerv)(0x0CF2, &rowLength); // GL_UNPACK_ROW_LENGTH
		QT_GL(GetIntegerv)(GL_UNPACK_ALIGNMENT, &alignment);
		size_t row = static_cast<size_t>(rowLength > 0 ? rowLength : width) * 2;
		if (alignment > 1) row = (row + alignment - 1) / alignment * alignment;
		rgba.resize(static_cast<size_t>(width) * height * 4);
		uint8_t* out = &rgba[0];
		for (GLsizei y = 0; y < height; ++y) {
			const uint8_t* p = pixels + row * y;
			for (GLsizei x = 0; x < width; ++x, p += 2, out += 4) {
				uint32_t v = static_cast<uint32_t>(p[0]) << 8 | p[1];
				if (format == mglPacked4444) {
					out[0] = static_cast<uint8_t>(((v >> 8) & 15) * 17);
					out[1] = static_cast<uint8_t>(((v >> 4) & 15) * 17);
					out[2] = static_cast<uint8_t>((v & 15) * 17);
					out[3] = static_cast<uint8_t>((v >> 12) * 17);
				}
				else {
					out[0] = static_cast<uint8_t>((v >> 11) * 255 / 31);
					out[1] = static_cast<uint8_t>(((v >> 5) & 63) * 255 / 63);
					out[2] = static_cast<uint8_t>((v & 31) * 255 / 31);
					out[3] = 255;
				}
			}
		}
		return rgba;
	}

	void uploadPacked(bool sub, const Command& c, GLenum format) {
		GLenum target = static_cast<GLenum>(c.u(1));
		GLint level = static_cast<GLint>(c.u(2));
		GLsizei width = static_cast<GLsizei>(c.u(sub ? 5 : 4)), height = static_cast<GLsizei>(c.u(sub ? 6 : 5));
		std::vector<uint8_t> rgba = packedToRgba(format, width, height, c.p(9));
		GLint rowLength = 0, alignment = 4;
		QT_GL(GetIntegerv)(0x0CF2, &rowLength);
		QT_GL(GetIntegerv)(GL_UNPACK_ALIGNMENT, &alignment);
		QT_GL(PixelStorei)(0x0CF2, 0);
		QT_GL(PixelStorei)(GL_UNPACK_ALIGNMENT, 4);
		void* data = rgba.empty() ? 0 : &rgba[0];
		if (sub) QT_GL(TexSubImage2D)(target, level, static_cast<GLint>(c.u(3)), static_cast<GLint>(c.u(4)), width, height,
			GL_RGBA, GL_UNSIGNED_BYTE, data);
		else QT_GL(TexImage2D)(target, level, GL_RGBA, width, height, static_cast<GLint>(c.u(6)), GL_RGBA, GL_UNSIGNED_BYTE, data);
		QT_GL(PixelStorei)(0x0CF2, rowLength);
		QT_GL(PixelStorei)(GL_UNPACK_ALIGNMENT, alignment);
	}

	// MiniGL (and Warp3D) make the mipmaps a program does not supply; OpenGL
	// takes a texture with a mipmap filter but only its level 0 as incomplete
	// and draws without it. GLQuake's font is such a texture, and came out as
	// white squares. OpenGL 1.4 makes them on every level 0 upload, where
	// a program's own levels still replace them.
	void generateMipmaps(const Command& c) {
		if (static_cast<GLenum>(c.u(1)) == GL_TEXTURE_2D && c.u(2) == 0)
			QT_GL(TexParameteri)(GL_TEXTURE_2D, 0x8191, GL_TRUE); // GL_GENERATE_MIPMAP
	}

	int32_t qt_manual_TexImage2D(const Command& c) {
		GLenum format = static_cast<GLenum>(c.u(7));
		GLint internalformat = static_cast<GLint>(c.u(3));
		generateMipmaps(c);
		if (packedFormat(format)) uploadPacked(false, c, format);
		else QT_GL(TexImage2D)(static_cast<GLenum>(c.u(1)), static_cast<GLint>(c.u(2)),
			packedFormat(static_cast<GLenum>(internalformat)) ? GL_RGBA : internalformat,
			static_cast<GLsizei>(c.u(4)), static_cast<GLsizei>(c.u(5)), static_cast<GLint>(c.u(6)), format,
			static_cast<GLenum>(c.u(8)), c.p(9));
		return 0;
	}

	int32_t qt_manual_TexSubImage2D(const Command& c) {
		GLenum format = static_cast<GLenum>(c.u(7));
		if (packedFormat(format)) uploadPacked(true, c, format);
		else QT_GL(TexSubImage2D)(static_cast<GLenum>(c.u(1)), static_cast<GLint>(c.u(2)), static_cast<GLint>(c.u(3)),
			static_cast<GLint>(c.u(4)), static_cast<GLsizei>(c.u(5)), static_cast<GLsizei>(c.u(6)), format,
			static_cast<GLenum>(c.u(8)), c.p(9));
		return 0;
	}

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

	// An offscreen context (QT_CONTEXT_OFFSCREEN) draws into a framebuffer
	// object, which has no front or back buffer: those names become its
	// colour buffer, as the application means them.
	GLenum colourBuffer(GLenum mode, GLenum binding) {
		GLint framebuffer = 0;
		switch (mode) {
		case GL_FRONT: case GL_BACK: case GL_FRONT_AND_BACK: case GL_LEFT: case GL_FRONT_LEFT: case GL_BACK_LEFT:
			QT_GL(GetIntegerv)(binding, &framebuffer);
			if (framebuffer) return 0x8CE0; // GL_COLOR_ATTACHMENT0
		}
		return mode;
	}

	int32_t qt_manual_DrawBuffer(const Command& c) {
		QT_GL(DrawBuffer)(colourBuffer(c.u(1), 0x8CA6 /* GL_DRAW_FRAMEBUFFER_BINDING */));
		return 0;
	}

	int32_t qt_manual_ReadBuffer(const Command& c) {
		QT_GL(ReadBuffer)(colourBuffer(c.u(1), 0x8CAA /* GL_READ_FRAMEBUFFER_BINDING */));
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
		uint32_t opcode = header >> 16;
		if (opcode < QT_W3D_FIRST || opcode >= QT_MGL_FIRST) qt_w3d_sync();
		switch (opcode) {
#include "gldecode.auto.inc"
		default:
			if (opcode >= QT_OGLES2_FIRST) {
				if (!qt_ogles2_decode(c, result)) return qt_bad_command(c);
			} else if (opcode >= QT_MGL_FIRST) {
				if (!qt_mgl_decode(c, result)) return qt_bad_command(c);
			} else if (opcode < QT_W3D_FIRST || !qt_w3d_decode(c, result)) return qt_bad_command(c);
			break;
		}
		// Tracing (QUARKTEX_TRACE_FRAME): every OpenGL and minigl command of
		// the frame (opcode = line in gl/glFuncs.txt, or gl/mglcmd.h) with its
		// first arguments in hex; Warp3D's are traced in host/w3d.cpp.
		if (qt_w3d_trace_all && (opcode < QT_W3D_FIRST || opcode >= QT_MGL_FIRST)) {
			char line[160];
			int n = snprintf(line, sizeof(line), "GL %X:", opcode);
			for (uint32_t i = 1; i < c.words && i <= 8 && n > 0 && n < static_cast<int>(sizeof(line)) - 12; ++i)
				n += snprintf(line + n, sizeof(line) - n, " %X", c.u(i));
			qt_report(line);
		}
		// Which OpenGL command an error comes from, with its first arguments.
		// Not between glBegin (5) and glEnd (75), where glGetError is an error
		// itself.
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
