// Framebuffer objects of offscreen contexts and the copy into Amiga memory
// (present.h). The functions past OpenGL 1.1 are looked up in the current
// context, compatibility or core.
//
// A picture is read back asynchronously: at the swap glReadPixels goes into a
// pixel buffer with a fence after it, and the host carries on. The picture is
// written into the Amiga bitmap as soon as the fence has passed, checked
// between the next frame's command buffers (the GPU finishes within a
// millisecond or two), or at the next swap at the latest; the 68k side asks
// for it to be written before it waits (minigl's main loop). Waiting for the
// GPU at the swap cost RTCW about 2.3 ms a frame.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <cstddef>
#include "present.h"

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#endif
#ifndef GL_PIXEL_PACK_BUFFER
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#define GL_STREAM_READ 0x88E1
#define GL_READ_ONLY 0x88B8
#endif
#ifndef GL_SYNC_GPU_COMMANDS_COMPLETE
#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define GL_ALREADY_SIGNALED 0x911A
#define GL_CONDITION_SATISFIED 0x911C
#endif
#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif
#ifndef GL_PACK_SWAP_BYTES
#define GL_PACK_SWAP_BYTES 0x0D00
#endif

namespace present {
	namespace {
		typedef void (APIENTRY* GenFunction)(GLsizei n, GLuint* names);
		typedef void (APIENTRY* BindFunction)(GLenum target, GLuint name);
		typedef void (APIENTRY* AttachFunction)(GLenum target, GLenum attachment, GLenum renderbufferTarget, GLuint renderbuffer);
		typedef void (APIENTRY* StorageFunction)(GLenum target, GLenum format, GLsizei width, GLsizei height);
		typedef GLenum (APIENTRY* StatusFunction)(GLenum target);
		typedef void (APIENTRY* BufferDataFunction)(GLenum target, ptrdiff_t size, const void* data, GLenum usage);
		typedef void* (APIENTRY* MapFunction)(GLenum target, GLenum access);
		typedef GLboolean (APIENTRY* UnmapFunction)(GLenum target);
		typedef void* (APIENTRY* FenceFunction)(GLenum condition, GLbitfield flags);
		typedef GLenum (APIENTRY* WaitFunction)(void* sync, GLbitfield flags, unsigned long long timeout);
		typedef void (APIENTRY* DeleteSyncFunction)(void* sync);
		GenFunction genFramebuffers, genRenderbuffers, deleteFramebuffers, deleteRenderbuffers, genBuffers, deleteBuffers;
		BindFunction bindFramebuffer, bindRenderbuffer, bindBuffer;
		AttachFunction framebufferRenderbuffer;
		StorageFunction renderbufferStorage;
		StatusFunction checkFramebufferStatus;
		BufferDataFunction bufferData;
		MapFunction mapBuffer;
		UnmapFunction unmapBuffer;
		FenceFunction fenceSync;
		WaitFunction clientWaitSync;
		DeleteSyncFunction deleteSync;

		template <typename T> T lookUp(const char* name) {
			return reinterpret_cast<T>(reinterpret_cast<void*>(wglGetProcAddress(name)));
		}

		bool load() {
			if (!genFramebuffers) {
				genFramebuffers = lookUp<GenFunction>("glGenFramebuffers");
				genRenderbuffers = lookUp<GenFunction>("glGenRenderbuffers");
				// glDelete* take const names; the call is the same.
				deleteFramebuffers = lookUp<GenFunction>("glDeleteFramebuffers");
				deleteRenderbuffers = lookUp<GenFunction>("glDeleteRenderbuffers");
				genBuffers = lookUp<GenFunction>("glGenBuffers");
				deleteBuffers = lookUp<GenFunction>("glDeleteBuffers");
				bindFramebuffer = lookUp<BindFunction>("glBindFramebuffer");
				bindRenderbuffer = lookUp<BindFunction>("glBindRenderbuffer");
				bindBuffer = lookUp<BindFunction>("glBindBuffer");
				framebufferRenderbuffer = lookUp<AttachFunction>("glFramebufferRenderbuffer");
				renderbufferStorage = lookUp<StorageFunction>("glRenderbufferStorage");
				checkFramebufferStatus = lookUp<StatusFunction>("glCheckFramebufferStatus");
				bufferData = lookUp<BufferDataFunction>("glBufferData");
				mapBuffer = lookUp<MapFunction>("glMapBuffer");
				unmapBuffer = lookUp<UnmapFunction>("glUnmapBuffer");
				fenceSync = lookUp<FenceFunction>("glFenceSync");
				clientWaitSync = lookUp<WaitFunction>("glClientWaitSync");
				deleteSync = lookUp<DeleteSyncFunction>("glDeleteSync");
			}
			return genFramebuffers && genRenderbuffers && deleteFramebuffers && deleteRenderbuffers && genBuffers
				&& deleteBuffers && bindFramebuffer && bindRenderbuffer && bindBuffer && framebufferRenderbuffer
				&& renderbufferStorage && checkFramebufferStatus && bufferData && mapBuffer && unmapBuffer && fenceSync
				&& clientWaitSync && deleteSync;
		}

		long long now() {
			LARGE_INTEGER t;
			QueryPerformanceCounter(&t);
			return t.QuadPart;
		}

		// The pixel buffer bound for packing before we bind ours.
		GLint boundPackBuffer() {
			GLint buffer = 0;
			glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &buffer);
			return buffer;
		}

		// Writes the pending picture into its target and releases its fence.
		void writePending(Framebuffer& f, void* (*resolve)(uint32_t)) {
			long long begin = now();
			GLint saved = boundPackBuffer();
			bindBuffer(GL_PIXEL_PACK_BUFFER, f.pixelBuffers[f.pending]);
			const uint8_t* bgra = static_cast<const uint8_t*>(mapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
			if (bgra) {
				write(bgra, f.width, f.height, f.target, resolve);
				unmapBuffer(GL_PIXEL_PACK_BUFFER);
			}
			bindBuffer(GL_PIXEL_PACK_BUFFER, static_cast<GLuint>(saved));
			deleteSync(f.fence);
			f.fence = 0;
			writeTicks += now() - begin;
		}
	}

	long long readTicks, writeTicks;

	bool resize(Framebuffer& f, int width, int height, void* (*resolve)(uint32_t)) {
		if (!load() || width <= 0 || height <= 0) return false;
		poll(f, resolve, true);
		if (!f.fbo) {
			genFramebuffers(1, &f.fbo);
			genRenderbuffers(1, &f.color);
			genRenderbuffers(1, &f.depthStencil);
			genBuffers(2, f.pixelBuffers);
		}
		bindRenderbuffer(GL_RENDERBUFFER, f.color);
		renderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
		bindRenderbuffer(GL_RENDERBUFFER, f.depthStencil);
		renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		bindRenderbuffer(GL_RENDERBUFFER, 0);
		GLint saved = boundPackBuffer();
		for (int i = 0; i < 2; ++i) {
			bindBuffer(GL_PIXEL_PACK_BUFFER, f.pixelBuffers[i]);
			bufferData(GL_PIXEL_PACK_BUFFER, static_cast<ptrdiff_t>(width) * height * 4, 0, GL_STREAM_READ);
		}
		bindBuffer(GL_PIXEL_PACK_BUFFER, static_cast<GLuint>(saved));
		bindFramebuffer(GL_FRAMEBUFFER, f.fbo);
		framebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, f.color);
		framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, f.depthStencil);
		f.width = width;
		f.height = height;
		return checkFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}

	void destroy(Framebuffer& f) {
		if (!f.fbo || !load()) return;
		if (f.fence) deleteSync(f.fence);
		bindFramebuffer(GL_FRAMEBUFFER, 0);
		deleteFramebuffers(1, &f.fbo);
		deleteRenderbuffers(1, &f.color);
		deleteRenderbuffers(1, &f.depthStencil);
		deleteBuffers(2, f.pixelBuffers);
		f.fbo = f.color = f.depthStencil = f.pixelBuffers[0] = f.pixelBuffers[1] = 0;
		f.fence = 0;
	}

	void start(Framebuffer& f, const Target& t, void* (*resolve)(uint32_t)) {
		if (!f.fbo || !bytesPerPixel(t.format)) return;
		poll(f, resolve, true);
		long long begin = now();
		// The application's pack state stays as it was (as in the frame
		// capture); the core profile has no attribute stacks and no
		// GL_PACK_SWAP_BYTES.
		const GLenum packs[] = {GL_PACK_ALIGNMENT, GL_PACK_ROW_LENGTH, GL_PACK_SKIP_ROWS, GL_PACK_SKIP_PIXELS, GL_PACK_SWAP_BYTES};
		const GLint values[] = {4, 0, 0, 0, GL_FALSE};
		const int count = f.core ? 4 : 5;
		GLint saved[5];
		for (int i = 0; i < count; ++i) glGetIntegerv(packs[i], &saved[i]);
		GLint savedBuffer = boundPackBuffer();
		for (int i = 0; i < count; ++i) glPixelStorei(packs[i], values[i]);
		bindBuffer(GL_PIXEL_PACK_BUFFER, f.pixelBuffers[f.next]);
		glReadPixels(0, 0, f.width, f.height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		bindBuffer(GL_PIXEL_PACK_BUFFER, static_cast<GLuint>(savedBuffer));
		for (int i = 0; i < count; ++i) glPixelStorei(packs[i], saved[i]);
		f.fence = fenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush();
		f.pending = f.next;
		f.next ^= 1;
		f.target = t;
		readTicks += now() - begin;
	}

	void poll(Framebuffer& f, void* (*resolve)(uint32_t), bool wait) {
		if (!f.fence) return;
		if (!wait) {
			GLenum state = clientWaitSync(f.fence, 0, 0);
			if (state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED) return;
		}
		writePending(f, resolve);
	}
}
