// Presenting into Amiga display memory (phase 8): a context created with
// QT_CONTEXT_OFFSCREEN draws into a framebuffer object of its own; at each
// swap its picture is read back and written into the Amiga bitmap the 68k
// side names (a QtTarget, gl/gl.h), in the bitmap's pixel format. The
// emulator then shows it as any other Amiga graphics: in a window, full
// window or fullscreen, scaled and filtered as the rest of the display.
#ifndef QUARKTEX_PRESENT_H
#define QUARKTEX_PRESENT_H

#include <cstdint>
#include <cstring>

namespace present {
	// Picasso96's RGBFormat numbers (libraries/Picasso96.h), which the 68k
	// side passes as they are.
	enum Format {
		R8G8B8 = 2, B8G8R8 = 3, R5G6B5PC = 4, R5G5B5PC = 5, A8R8G8B8 = 6, A8B8G8R8 = 7,
		R8G8B8A8 = 8, B8G8R8A8 = 9, R5G6B5 = 10, R5G5B5 = 11, B5G6R5PC = 12, B5G5R5PC = 13
	};

	// Bytes per pixel of a format, 0 if it is not one we write.
	inline int bytesPerPixel(uint32_t format) {
		switch (format) {
		case R8G8B8: case B8G8R8: return 3;
		case R5G6B5PC: case R5G5B5PC: case R5G6B5: case R5G5B5: case B5G6R5PC: case B5G5R5PC: return 2;
		case A8R8G8B8: case A8B8G8R8: case R8G8B8A8: case B8G8R8A8: return 4;
		}
		return 0;
	}

	// count pixels of B, G, R, A bytes (what glReadPixels gives for GL_BGRA)
	// into dest in the format. The alpha of the 32-bit formats is unused by
	// the display; it is written as 0.
	inline void convertRow(const uint8_t* bgra, uint8_t* dest, int count, uint32_t format) {
		if (format == B8G8R8A8) {
			for (int i = 0; i < count; ++i, bgra += 4, dest += 4) {
				dest[0] = bgra[0]; dest[1] = bgra[1]; dest[2] = bgra[2]; dest[3] = 0;
			}
			return;
		}
		for (int i = 0; i < count; ++i, bgra += 4) {
			uint8_t b = bgra[0], g = bgra[1], r = bgra[2];
			uint16_t v = 0;
			switch (format) {
			case R8G8B8: *dest++ = r; *dest++ = g; *dest++ = b; continue;
			case B8G8R8: *dest++ = b; *dest++ = g; *dest++ = r; continue;
			case A8R8G8B8: *dest++ = 0; *dest++ = r; *dest++ = g; *dest++ = b; continue;
			case A8B8G8R8: *dest++ = 0; *dest++ = b; *dest++ = g; *dest++ = r; continue;
			case R8G8B8A8: *dest++ = r; *dest++ = g; *dest++ = b; *dest++ = 0; continue;
			case R5G6B5: case R5G6B5PC: v = static_cast<uint16_t>(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)); break;
			case R5G5B5: case R5G5B5PC: v = static_cast<uint16_t>(((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3)); break;
			case B5G6R5PC: v = static_cast<uint16_t>(((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3)); break;
			case B5G5R5PC: v = static_cast<uint16_t>(((b >> 3) << 10) | ((g >> 3) << 5) | (r >> 3)); break;
			}
			// The PC formats are little-endian, the others big-endian.
			if (format == R5G6B5 || format == R5G5B5) { *dest++ = static_cast<uint8_t>(v >> 8); *dest++ = static_cast<uint8_t>(v); }
			else { *dest++ = static_cast<uint8_t>(v); *dest++ = static_cast<uint8_t>(v >> 8); }
		}
	}

	// Where a frame goes: the words of a QtTarget, already in host order.
	struct Target {
		uint32_t address, bytesPerRow, format, bitmapWidth, bitmapHeight;
		int32_t left, top, width, height;
	};

	// Writes a bottom-up picture of width x height BGRA pixels into the
	// target's rectangle, clipped to the bitmap and to the picture; resolve
	// maps an Amiga address to a host pointer (0 if there is no memory).
	// Returns false if the target's format is not one we write.
	template <typename Resolve>
	bool write(const uint8_t* bgra, int width, int height, const Target& t, Resolve resolve) {
		int bpp = bytesPerPixel(t.format);
		if (!bpp) return false;
		int x0 = t.left < 0 ? -t.left : 0, y0 = t.top < 0 ? -t.top : 0;
		int w = t.width < width ? t.width : width, h = t.height < height ? t.height : height;
		if (t.left + w > static_cast<int>(t.bitmapWidth)) w = static_cast<int>(t.bitmapWidth) - t.left;
		if (t.top + h > static_cast<int>(t.bitmapHeight)) h = static_cast<int>(t.bitmapHeight) - t.top;
		for (int y = y0; y < h; ++y) {
			uint32_t row = t.address + static_cast<uint32_t>(t.top + y) * t.bytesPerRow + static_cast<uint32_t>(t.left + x0) * bpp;
			uint8_t* dest = static_cast<uint8_t*>(resolve(row));
			if (!dest || x0 >= w) continue;
			// The picture's rows run from the bottom up.
			convertRow(bgra + (static_cast<size_t>(height - 1 - y) * width + x0) * 4, dest, w - x0, t.format);
		}
		return true;
	}

	// The framebuffer object of an offscreen context: colour and
	// depth/stencil renderbuffers. In the context that is current.
	struct Framebuffer {
		unsigned int fbo, color, depthStencil;
		int width, height;
	};

	// Creates or resizes it and binds it for drawing and reading.
	bool resize(Framebuffer& f, int width, int height);
	void destroy(Framebuffer& f);
	// Reads its picture (bottom-up BGRA) into pixels and writes it into the
	// target; leaves the application's pack state as it was.
	bool copy(const Framebuffer& f, const Target& t, void* (*resolve)(uint32_t), bool core);
}

#endif
