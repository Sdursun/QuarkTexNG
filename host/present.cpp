// Framebuffer objects of offscreen contexts and the copy into Amiga memory
// (present.h). The functions past OpenGL 1.1 are looked up in the current
// context, compatibility or core.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <vector>
#include "present.h"

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
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
		GenFunction genFramebuffers, genRenderbuffers, deleteFramebuffers, deleteRenderbuffers;
		BindFunction bindFramebuffer, bindRenderbuffer, bindBuffer;
		AttachFunction framebufferRenderbuffer;
		StorageFunction renderbufferStorage;
		StatusFunction checkFramebufferStatus;

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
				bindFramebuffer = lookUp<BindFunction>("glBindFramebuffer");
				bindRenderbuffer = lookUp<BindFunction>("glBindRenderbuffer");
				bindBuffer = lookUp<BindFunction>("glBindBuffer");
				framebufferRenderbuffer = lookUp<AttachFunction>("glFramebufferRenderbuffer");
				renderbufferStorage = lookUp<StorageFunction>("glRenderbufferStorage");
				checkFramebufferStatus = lookUp<StatusFunction>("glCheckFramebufferStatus");
			}
			return genFramebuffers && genRenderbuffers && deleteFramebuffers && deleteRenderbuffers && bindFramebuffer
				&& bindRenderbuffer && bindBuffer && framebufferRenderbuffer && renderbufferStorage && checkFramebufferStatus;
		}

		std::vector<uint8_t> pixels; // the last picture read back
	}

	bool resize(Framebuffer& f, int width, int height) {
		if (!load() || width <= 0 || height <= 0) return false;
		if (!f.fbo) {
			genFramebuffers(1, &f.fbo);
			genRenderbuffers(1, &f.color);
			genRenderbuffers(1, &f.depthStencil);
		}
		bindRenderbuffer(GL_RENDERBUFFER, f.color);
		renderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
		bindRenderbuffer(GL_RENDERBUFFER, f.depthStencil);
		renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		bindRenderbuffer(GL_RENDERBUFFER, 0);
		bindFramebuffer(GL_FRAMEBUFFER, f.fbo);
		framebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, f.color);
		framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, f.depthStencil);
		f.width = width;
		f.height = height;
		return checkFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}

	void destroy(Framebuffer& f) {
		if (!f.fbo || !load()) return;
		bindFramebuffer(GL_FRAMEBUFFER, 0);
		deleteFramebuffers(1, &f.fbo);
		deleteRenderbuffers(1, &f.color);
		deleteRenderbuffers(1, &f.depthStencil);
		f.fbo = f.color = f.depthStencil = 0;
	}

	bool copy(const Framebuffer& f, const Target& t, void* (*resolve)(uint32_t), bool core) {
		if (!f.fbo || !bytesPerPixel(t.format)) return false;
		pixels.resize(static_cast<size_t>(f.width) * f.height * 4);
		// The application's pack state stays as it was (as in the frame
		// capture); the core profile has no attribute stacks.
		const GLenum packs[] = {GL_PACK_ALIGNMENT, GL_PACK_ROW_LENGTH, GL_PACK_SKIP_ROWS, GL_PACK_SKIP_PIXELS, GL_PACK_SWAP_BYTES};
		const GLint values[] = {4, 0, 0, 0, GL_FALSE};
		// GL_PACK_SWAP_BYTES is not in the core profile.
		const int count = core ? 4 : 5;
		GLint saved[5], packBuffer = 0;
		for (int i = 0; i < count; ++i) glGetIntegerv(packs[i], &saved[i]);
		glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &packBuffer);
		if (packBuffer) bindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		for (int i = 0; i < count; ++i) glPixelStorei(packs[i], values[i]);
		glReadPixels(0, 0, f.width, f.height, GL_BGRA, GL_UNSIGNED_BYTE, &pixels[0]);
		for (int i = 0; i < count; ++i) glPixelStorei(packs[i], saved[i]);
		if (packBuffer) bindBuffer(GL_PIXEL_PACK_BUFFER, static_cast<GLuint>(packBuffer));
		return write(&pixels[0], f.width, f.height, t, resolve);
	}
}
