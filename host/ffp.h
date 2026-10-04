// The OpenGL 1.1 calls host/w3d.cpp draws Warp3D with, on an OpenGL 3.3 core
// context (docs/phase6-core-renderer.md). The functions have the names and
// the meaning of the OpenGL 1.1 ones; w3d.cpp calls them as QT_GL(name).
// Immediate mode, the current colour and texture coordinate, flat shading,
// texture environments, fog and the alpha test are emulated with a shader;
// the rest is passed on.
#ifndef QUARKTEX_FFP_H
#define QUARKTEX_FFP_H

#include "gl3.h"

namespace ffp {
	// With the context current. width and height are those of the drawing
	// area, as the 0.53 model view matrix used them. False on failure,
	// logged.
	bool init(int width, int height);
	void shutdown();

	// Separate triangles, lines and points are batched until the state
	// changes; flush draws them. Every function below that is not immediate
	// mode flushes itself; OpenGL calls made around this emulation (the
	// command buffer's OpenGL commands, SwapBuffers, frame capture) need a
	// flush first.
	void flush();

	// Immediate mode
	void Begin(GLenum mode);
	void End();
	void Vertex2f(GLfloat x, GLfloat y);
	void Vertex3f(GLfloat x, GLfloat y, GLfloat z);
	void Color3f(GLfloat r, GLfloat g, GLfloat b);
	void Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	void Color3ub(GLubyte r, GLubyte g, GLubyte b);
	void Color4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a);
	void TexCoord2f(GLfloat s, GLfloat t);
	void TexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	void Recti(GLint x1, GLint y1, GLint x2, GLint y2);

	// Emulated state; the other capabilities go to OpenGL.
	void Enable(GLenum cap);
	void Disable(GLenum cap);
	void ShadeModel(GLenum mode);
	void AlphaFunc(GLenum function, GLclampf reference);
	void Fogf(GLenum pname, GLfloat param);
	void Fogi(GLenum pname, GLint param);
	void Fogfv(GLenum pname, const GLfloat* params);
	void TexEnvi(GLenum target, GLenum pname, GLint param);
	void TexEnvfv(GLenum target, GLenum pname, const GLfloat* params);

	// Textures: the OpenGL 1.1 pixel formats (GL_ALPHA, GL_LUMINANCE, ...)
	// become red/green textures with a swizzle; GL_CLAMP is emulated (see
	// TextureInfo in ffp.cpp).
	void GenTextures(GLsizei n, GLuint* textures);
	void DeleteTextures(GLsizei n, const GLuint* textures);
	void BindTexture(GLenum target, GLuint texture);
	void TexParameteri(GLenum target, GLenum pname, GLint param);
	void TexParameterfv(GLenum target, GLenum pname, const GLfloat* params);
	void TexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border,
		GLenum format, GLenum type, const GLvoid* pixels);
	void TexSubImage2D(GLenum target, GLint level, GLint x, GLint y, GLsizei width, GLsizei height,
		GLenum format, GLenum type, const GLvoid* pixels);

	// Writes count depth values without touching the colours: xyz holds
	// x, y (window coordinates as for Vertex*) and the depth (0..1) of each
	// pixel. The depth test, depth function and colour mask are restored.
	void DepthPoints(GLsizei count, const GLfloat* xyz);

	// Writes count stencil values without touching colours and depths: xy
	// holds x, y of each pixel as for DepthPoints. The stencil write mask
	// applies, as for glDrawPixels; the stencil state is restored.
	void StencilPoints(GLsizei count, const GLfloat* xy, const GLuint* values);

	// Passed on as they are.
	inline void PixelStorei(GLenum pname, GLint param) { flush(); glPixelStorei(pname, param); }
	inline void BlendFunc(GLenum source, GLenum destination) { flush(); glBlendFunc(source, destination); }
	inline void DepthMask(GLboolean flag) { flush(); glDepthMask(flag); }
	inline void DepthFunc(GLenum function) { flush(); glDepthFunc(function); }
	inline void LogicOp(GLenum operation) { flush(); glLogicOp(operation); }
	inline void ColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a) { flush(); glColorMask(r, g, b, a); }
	inline void Scissor(GLint x, GLint y, GLsizei width, GLsizei height) { flush(); glScissor(x, y, width, height); }
	inline void ClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a) { flush(); glClearColor(r, g, b, a); }
	inline void Clear(GLbitfield mask) { flush(); glClear(mask); }
	inline void PointSize(GLfloat size) { flush(); glPointSize(size); }
	inline void LineWidth(GLfloat width) { flush(); glLineWidth(width); }
	inline void StencilFunc(GLenum function, GLint reference, GLuint mask) { flush(); glStencilFunc(function, reference, mask); }
	inline void StencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) { flush(); glStencilOp(sfail, dpfail, dppass); }
	inline void StencilMask(GLuint mask) { flush(); glStencilMask(mask); }
	inline void ClearStencil(GLint value) { flush(); glClearStencil(value); }
	inline void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) {
		flush();
		glReadPixels(x, y, width, height, format, type, pixels);
	}
}

#endif
