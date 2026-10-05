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
	// the display; it is written as 0. One loop per format: this runs for
	// every pixel of every frame.
	inline uint32_t loadPixel(const uint8_t* p) {
		uint32_t v;
		memcpy(&v, p, 4); // little-endian host: b | g << 8 | r << 16 | a << 24
		return v;
	}

	inline uint32_t swapBytes(uint32_t v) {
		return (v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24);
	}

	// 16 bits from r, g, b (each 0-255) as hi:lo bits per channel.
	template <int R, int G, int B, bool RedHigh>
	inline uint16_t pack16(uint32_t p) {
		uint32_t r = (p >> 16) & 0xFF, g = (p >> 8) & 0xFF, b = p & 0xFF;
		uint32_t high = RedHigh ? r : b, low = RedHigh ? b : r;
		return static_cast<uint16_t>(((high >> (8 - R)) << (G + B)) | ((g >> (8 - G)) << B) | (low >> (8 - B)));
	}

	template <int G, bool RedHigh, bool BigEndian>
	inline void convert16(const uint8_t* bgra, uint8_t* dest, int count) {
		for (int i = 0; i < count; ++i, bgra += 4, dest += 2) {
			uint16_t v = pack16<5, G, 5, RedHigh>(loadPixel(bgra));
			dest[BigEndian ? 0 : 1] = static_cast<uint8_t>(v >> 8);
			dest[BigEndian ? 1 : 0] = static_cast<uint8_t>(v);
		}
	}

	inline void convertRow(const uint8_t* bgra, uint8_t* dest, int count, uint32_t format) {
		switch (format) {
		case B8G8R8A8: case A8B8G8R8: case R8G8B8A8: case A8R8G8B8:
			for (int i = 0; i < count; ++i, bgra += 4, dest += 4) {
				uint32_t p = loadPixel(bgra) & 0x00FFFFFF, v;
				if (format == B8G8R8A8) v = p;                       // b g r 0
				else if (format == A8B8G8R8) v = p << 8;             // 0 b g r
				else if (format == R8G8B8A8) v = swapBytes(p << 8);  // r g b 0
				else v = swapBytes(p);                                // 0 r g b
				memcpy(dest, &v, 4);
			}
			return;
		case R8G8B8:
			for (int i = 0; i < count; ++i, bgra += 4, dest += 3) { dest[0] = bgra[2]; dest[1] = bgra[1]; dest[2] = bgra[0]; }
			return;
		case B8G8R8:
			for (int i = 0; i < count; ++i, bgra += 4, dest += 3) { dest[0] = bgra[0]; dest[1] = bgra[1]; dest[2] = bgra[2]; }
			return;
		// The PC formats are little-endian, the others big-endian.
		case R5G6B5: convert16<6, true, true>(bgra, dest, count); return;
		case R5G6B5PC: convert16<6, true, false>(bgra, dest, count); return;
		case R5G5B5: convert16<5, true, true>(bgra, dest, count); return;
		case R5G5B5PC: convert16<5, true, false>(bgra, dest, count); return;
		case B5G6R5PC: convert16<6, false, false>(bgra, dest, count); return;
		case B5G5R5PC: convert16<5, false, false>(bgra, dest, count); return;
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
	// depth/stencil renderbuffers, and two pixel buffers its pictures are
	// read back into without waiting for the GPU. In the context that is
	// current.
	struct Framebuffer {
		unsigned int fbo, color, depthStencil;
		int width, height;
		unsigned int pixelBuffers[2];
		int next;          // the pixel buffer the next picture goes to
		void* fence;       // GLsync of the pending picture, 0 if none
		int pending;       // its pixel buffer
		Target target;     // where it goes
		bool core;
	};

	// Creates or resizes it and binds it for drawing and reading; a pending
	// picture is written first.
	bool resize(Framebuffer& f, int width, int height, void* (*resolve)(uint32_t));
	void destroy(Framebuffer& f);

	// Starts reading the picture back for the target (at a swap); a picture
	// still pending is written first. Leaves the application's pack state as
	// it was.
	void start(Framebuffer& f, const Target& t, void* (*resolve)(uint32_t));
	// Writes the pending picture if the GPU has finished it (between
	// commands, without waiting), or in any case (wait).
	void poll(Framebuffer& f, void* (*resolve)(uint32_t), bool wait);

	// Performance counter ticks spent starting the read back and writing
	// pictures, for the profile (QUARKTEX_PROFILE); the profile resets them.
	extern long long readTicks, writeTicks;
}

#endif
