/*
 * e01_ogles3_api: OpenGL ES 3.0's entries in ogles3.library (docs/phase10-
 * ogles3.md, stage 2), each checked by drawing with it and reading the
 * colour back: GLSL ES 3.00, vertex array objects switching attributes and
 * index buffers, integer attributes and uint uniforms, instancing with an
 * attribute divisor, a uniform buffer changed with glBufferSubData and
 * glMapBufferRange, glCopyBufferSubData, glTexStorage2D, a 3D texture,
 * sampler objects, a float render target (RGBA16F) read by glBlitFramebuffer,
 * glClearBufferfv, and the queries ES 3.0 adds.
 *
 * Built only when the ogles3 SDK is there (tests/Makefile).
 */
#include <proto/ogles3.h>
#include <GLES3/gl3.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <stdio.h>
#include <string.h>
#include "window.h"

static int failures;

static void check(const char *what, int ok) {
	if (!ok) ++failures;
	printf("e01: %s %s\n", ok ? "PASS" : "FAIL", what);
}

static void checkInt(const char *what, GLint value, GLint expected) {
	if (value != expected) ++failures;
	printf("e01: %s %s: %ld, expected %ld\n", value == expected ? "PASS" : "FAIL", what, (long) value, (long) expected);
}

/* The colour at (x, y) of the framebuffer bound for reading. */
static void expectAt(const char *what, int x, int y, int r, int g, int b) {
	GLubyte p[4] = {0, 0, 0, 0};
	int ok;
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, p);
	ok = p[0] >= r - 4 && p[0] <= r + 4 && p[1] >= g - 4 && p[1] <= g + 4 && p[2] >= b - 4 && p[2] <= b + 4;
	if (!ok) ++failures;
	printf("e01: %s %s: (%d, %d, %d), expected (%d, %d, %d)\n", ok ? "PASS" : "FAIL", what, p[0], p[1], p[2], r, g, b);
}

static void expectPixel(const char *what, int r, int g, int b) {
	expectAt(what, WIDTH / 2, HEIGHT / 2, r, g, b);
}

static GLuint shader(GLenum type, const char *source) {
	GLuint s = glCreateShader(type);
	GLint ok = 0;
	glShaderSource(s, 1, &source, NULL);
	glCompileShader(s);
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[256];
		glGetShaderInfoLog(s, sizeof(log), NULL, log);
		printf("e01: shader log: %s\n", log);
	}
	check("GLSL ES 3.00 shader compiles", ok);
	return s;
}

static GLuint program(const char *vertex, const char *fragment) {
	GLuint p = glCreateProgram();
	GLint ok = 0;
	glAttachShader(p, shader(GL_VERTEX_SHADER, vertex));
	glAttachShader(p, shader(GL_FRAGMENT_SHADER, fragment));
	glLinkProgram(p);
	glGetProgramiv(p, GL_LINK_STATUS, &ok);
	check("program links", ok);
	glUseProgram(p);
	return p;
}

/* Position (attribute 0) from a buffer; colour from the uniform block, an
 * integer attribute (1) and a per-instance offset (2) moving each instance
 * right by half the window. */
static const char *vertexSource =
	"#version 300 es\n"
	"layout(location = 0) in vec2 pos;\n"
	"layout(location = 1) in ivec2 code;\n"
	"layout(location = 2) in float shift;\n"
	"flat out ivec2 v_code;\n"
	"void main() {\n"
	"  v_code = code;\n"
	"  gl_Position = vec4(pos.x + shift, pos.y, 0.0, 1.0);\n"
	"}\n";

static const char *fragmentSource =
	"#version 300 es\n"
	"precision mediump float;\n"
	"layout(std140) uniform Colours { vec4 tint; vec4 extra[2]; };\n"
	"uniform uint u_scale;\n"
	"flat in ivec2 v_code;\n"
	"out vec4 colour;\n"
	"void main() {\n"
	"  colour = tint * float(u_scale) * float(v_code.x) + extra[v_code.y];\n"
	"}\n";

/* A textured quad: sampler2D and sampler3D. */
static const char *texVertex =
	"#version 300 es\n"
	"layout(location = 0) in vec2 pos;\n"
	"out vec2 uv;\n"
	"void main() { uv = pos * 0.5 + 0.5; gl_Position = vec4(pos, 0.0, 1.0); }\n";
static const char *texFragment =
	"#version 300 es\n"
	"precision mediump float;\n"
	"precision mediump sampler3D;\n"
	"uniform sampler2D tex2;\n"
	"uniform sampler3D tex3;\n"
	"uniform int which;\n"
	"in vec2 uv;\n"
	"out vec4 colour;\n"
	"void main() { colour = which == 0 ? texture(tex2, uv) : texture(tex3, vec3(uv, 0.75)); }\n";

static void queries(void) {
	GLint value = 0;
	GLint64 value64 = 0;
	const GLubyte *s = glGetString(GL_VERSION);
	/* 30 gives the highest 3.x, as the PiStorm library 1.0 does: 3.1 on the
	 * test machine's host (OpenGL 4.3). */
	check("GL_VERSION is OpenGL ES 3.1 ogles3.library 1.0", s && !strcmp((const char *) s, "OpenGL ES 3.1 ogles3.library 1.0"));
	s = glGetString(GL_SHADING_LANGUAGE_VERSION);
	check("GL_SHADING_LANGUAGE_VERSION is GLSL ES 3.10", s && !strcmp((const char *) s, "OpenGL ES GLSL ES 3.10"));
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value);
	checkInt("GL_MAX_VERTEX_ATTRIBS", value, 16);
	glGetInteger64v(GL_NUM_EXTENSIONS, &value64);
	check("GL_NUM_EXTENSIONS > 0", value64 > 0);
	s = value64 > 0 ? glGetStringi(GL_EXTENSIONS, 0) : NULL;
	check("glGetStringi(GL_EXTENSIONS, 0)", s && !strncmp((const char *) s, "GL_", 3));
	{
		/* glGetIntegerv's count is glGetStringi's (iortcw's rend2 lists them
		 * that way: once 404 from the host, NULL past the 10th), and every
		 * word is in glGetString's list. */
		const char *all = (const char *) glGetString(GL_EXTENSIONS);
		GLint i, words = 0, found = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &value);
		checkInt("glGetIntegerv(GL_NUM_EXTENSIONS) = glGetInteger64v's", value, (int) value64);
		for (i = 0; i < value; ++i) {
			s = glGetStringi(GL_EXTENSIONS, (GLuint) i);
			if (s) ++words;
			if (s && all && strstr(all, (const char *) s)) ++found;
		}
		checkInt("glGetStringi gives a name for every index", words, value);
		checkInt("... each one in glGetString's list", found, value);
		check("glGetStringi past the end is NULL", glGetStringi(GL_EXTENSIONS, (GLuint) value) == NULL);
	}
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &value);
	check("GL_MAX_UNIFORM_BUFFER_BINDINGS >= 24", value >= 24);
}

int main(void) {
	struct Window *win;
	void *ctx;
	ULONG err = 0;
	GLuint p, tp, buf[5], vao[2], tex[3], sampler, fbo, rb;
	/* A quad over the left half (x -1..0), drawn twice, the second
	 * instance shifted right by 1 (the divisor attribute). */
	static const GLfloat quad[8] = {-1, -1, 0, -1, 0, 1, -1, 1};
	static const GLint codes[8] = {1, 0, 1, 0, 1, 0, 1, 0};
	static const GLint codes2[8] = {0, 1, 0, 1, 0, 1, 0, 1};
	static const GLfloat shifts[2] = {0, 1};
	static const GLushort indices[6] = {0, 1, 2, 0, 2, 3};
	static const GLfloat full[8] = {-1, -1, 1, -1, 1, 1, -1, 1};
	/* std140: vec4 tint, vec4 extra[2]. */
	GLfloat block[12] = {0.25f, 0, 0, 1, 0, 0, 0, 0, 0, 0.5f, 0, 0};
	GLuint loc;
	GLubyte texel[4] = {0, 0, 255, 255};
	GLubyte volume[2 * 2 * 2 * 4];
	int i;

	write_label("e01_ogles3_api");
	if (!OGLES3Open()) {
		printf("e01: FAIL no ogles3.library\n");
		return 20;
	}
	if (!(win = open_window("e01: ogles3 API"))) {
		printf("e01: FAIL no window\n");
		OGLES3Close();
		return 20;
	}
	ctx = aglCreateContextTags2(&err, OGLES3_CCT_WINDOW, (ULONG) win, OGLES3_CCT_PISTORM_ES_VERSION, 30, TAG_DONE);
	if (!ctx) {
		printf("e01: FAIL aglCreateContextTags2, error %lu\n", (unsigned long) err);
		CloseWindow(win);
		OGLES3Close();
		return 20;
	}
	aglMakeCurrent(ctx);
	queries();
	{
		ULONG err31 = 0;
		void *ctx31 = aglCreateContextTags2(&err31, OGLES3_CCT_WINDOW, (ULONG) win, OGLES3_CCT_PISTORM_ES_VERSION, 31, TAG_DONE);
		/* The test machine's host has OpenGL 4.3: ES 3.1 contexts open (stage 3). */
		void *other;
		const GLubyte *s;
		checkInt("OpenGL ES 3.1 context opens", (GLint) err31, OGLES3_CCE_OK);
		/* No version tag: the highest, as the PiStorm library 1.0. */
		if ((other = aglCreateContextTags2(&err31, OGLES3_CCT_WINDOW, (ULONG) win, TAG_DONE))) {
			aglMakeCurrent(other);
			s = glGetString(GL_VERSION);
			check("a context without the version tag is OpenGL ES 3.1", s && !strncmp((const char *) s, "OpenGL ES 3.1", 13));
			aglDestroyContext(other);
		}
		else check("a context without the version tag opens", 0);
		/* 20: OpenGL ES 2.0, which shares objects only with 2.0 contexts. */
		if ((other = aglCreateContextTags2(&err31, OGLES3_CCT_WINDOW, (ULONG) win, OGLES3_CCT_PISTORM_ES_VERSION, 20, TAG_DONE))) {
			aglMakeCurrent(other);
			s = glGetString(GL_VERSION);
			check("version 20 gives OpenGL ES 2.0 ogles3.library 1.0", s && !strcmp((const char *) s, "OpenGL ES 2.0 ogles3.library 1.0"));
			check("... which does not share with a 3.x context",
				!aglCreateContextTags2(&err31, OGLES3_CCT_WINDOW, (ULONG) win, OGLES3_CCT_PISTORM_ES_VERSION, 20,
					OGLES3_CCT_SHARE_WITH, (ULONG) ctx, TAG_DONE) && err31 == OGLES3_CCE_BAD_TAGS);
			aglDestroyContext(other);
		}
		else check("a 2.0 context opens", 0);
		if (ctx31) aglDestroyContext(ctx31);
		aglMakeCurrent(ctx);
	}

	p = program(vertexSource, fragmentSource);
	glGenBuffers(5, buf);
	/* buf[0] positions, buf[1] codes, buf[2] codes2, buf[3] shifts, buf[4] the uniform block. */
	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(codes), codes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(codes2), codes2, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shifts), shifts, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, buf[4]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(block), block, GL_DYNAMIC_DRAW);
	loc = glGetUniformBlockIndex(p, "Colours");
	check("glGetUniformBlockIndex", loc != GL_INVALID_INDEX);
	glUniformBlockBinding(p, loc, 2);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, buf[4]);
	glUniform1ui(glGetUniformLocation(p, "u_scale"), 4);
	{
		GLuint scale = 0;
		glGetUniformuiv(p, glGetUniformLocation(p, "u_scale"), &scale);
		checkInt("glUniform1ui/glGetUniformuiv", (GLint) scale, 4);
	}

	/* Two vertex array objects: the first with codes (tint * 4 = red), the
	 * second with codes2 (extra[1] = green). */
	glGenVertexArrays(2, vao);
	for (i = 0; i < 2; ++i) {
		glBindVertexArray(vao[i]);
		glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, buf[i ? 2 : 1]);
		glVertexAttribIPointer(1, 2, GL_INT, 0, 0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, buf[3]);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);
		glEnableVertexAttribArray(2);
	}
	glBindVertexArray(0);
	check("glIsVertexArray", glIsVertexArray(vao[1]) && !glIsVertexArray(77));

	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(vao[0]);
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 2);
	expectAt("VAO 1, integer attribute, uniform block, uint uniform: left", WIDTH / 4, HEIGHT / 2, 255, 0, 0);
	expectAt("instance 2 shifted by its divisor attribute: right", 3 * WIDTH / 4, HEIGHT / 2, 255, 0, 0);

	/* Index buffer kept in the VAO; the second VAO's codes pick extra[1]. */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf[0]); /* in vao[0]: replaced below */
	glBindVertexArray(vao[1]);
	{
		GLuint ib;
		glGenBuffers(1, &ib);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, 2);
		expectPixel("VAO 2 with its own index buffer: extra[1]", 0, 128, 0);
	}

	/* The uniform block changed: glBufferSubData, then glMapBufferRange on a
	 * copy and glCopyBufferSubData back. */
	block[5] = 0;
	block[9] = 0;
	block[10] = 1.0f; /* extra[1] = blue */
	glBindBuffer(GL_UNIFORM_BUFFER, buf[4]);
	glBufferSubData(GL_UNIFORM_BUFFER, 32, 16, &block[8]);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, 1);
	expectAt("uniform block after glBufferSubData", WIDTH / 4, HEIGHT / 2, 0, 0, 255);
	{
		GLuint copy;
		GLfloat *mapped;
		glGenBuffers(1, &copy);
		glBindBuffer(GL_ARRAY_BUFFER, copy);
		glBufferData(GL_ARRAY_BUFFER, sizeof(block), block, GL_STATIC_DRAW);
		mapped = (GLfloat *) glMapBufferRange(GL_ARRAY_BUFFER, 32, 16, GL_MAP_WRITE_BIT);
		check("glMapBufferRange", mapped != NULL);
		if (mapped) {
			mapped[0] = 1.0f; /* extra[1] = magenta */
			mapped[1] = 0;
			mapped[2] = 1.0f;
		}
		check("glUnmapBuffer", glUnmapBuffer(GL_ARRAY_BUFFER) == GL_TRUE);
		glBindBuffer(GL_COPY_READ_BUFFER, copy);
		glBindBuffer(GL_COPY_WRITE_BUFFER, buf[4]);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 32, 32, 16);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, 1);
		expectAt("glMapBufferRange, glCopyBufferSubData into the uniform block", WIDTH / 4, HEIGHT / 2, 255, 0, 255);
	}
	glBindVertexArray(0);

	/* Textures: glTexStorage2D + glTexSubImage2D, a 3D texture, a sampler
	 * object overriding the texture's own filter. */
	tp = program(texVertex, texFragment);
	glUniform1i(glGetUniformLocation(tp, "tex2"), 0);
	glUniform1i(glGetUniformLocation(tp, "tex3"), 1);
	glGenTextures(3, tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, texel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); /* incomplete without a sampler */
	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindSampler(0, sampler);
	check("glIsSampler", glIsSampler(sampler));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, full);
	glEnableVertexAttribArray(0);
	glUniform1i(glGetUniformLocation(tp, "which"), 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	expectPixel("glTexStorage2D texture through a sampler object", 0, 0, 255);

	for (i = 0; i < 8; ++i) {
		/* Slice 0 black, slice 1 yellow. */
		volume[4 * i] = volume[4 * i + 1] = i >= 4 ? 255 : 0;
		volume[4 * i + 2] = 0;
		volume[4 * i + 3] = 255;
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, tex[1]);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, volume);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glUniform1i(glGetUniformLocation(tp, "which"), 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	expectPixel("glTexImage3D, slice 1", 255, 255, 0);

	/* A float render target cleared with glClearBufferfv, blitted to the
	 * window. */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, WIDTH, HEIGHT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex[2], 0);
	checkInt("RGBA16F framebuffer complete", (GLint) glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
	{
		static const GLfloat cyan[4] = {0, 1, 1, 1};
		glClearBufferfv(GL_COLOR, 0, cyan);
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	expectPixel("glClearBufferfv on RGBA16F, glBlitFramebuffer", 0, 255, 255);
	(void) rb;

	checkInt("glGetError", (GLint) glGetError(), GL_NO_ERROR);
	aglSwapBuffers();
	glFinish();
	report_shown("e01_ogles3_api", win);

	glDeleteVertexArrays(2, vao);
	glDeleteSamplers(1, &sampler);
	glDeleteTextures(3, tex);
	glDeleteFramebuffers(1, &fbo);
	glDeleteBuffers(5, buf);
	aglDestroyContext(ctx);
	CloseWindow(win);
	OGLES3Close();
	printf("e01: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 10 : 0;
}
