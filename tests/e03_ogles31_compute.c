/*
 * e03_ogles31_compute: OpenGL ES 3.1 in ogles3.library (docs/phase10-
 * ogles3.md, stage 3): an ES 3.1 context, a compute shader writing a shader
 * storage buffer and an atomic counter (read back on the 68k with
 * glMapBufferRange), glDispatchComputeIndirect, an image written with
 * imageStore, and a buffer a compute shader filled drawn as vertices with
 * glDrawArraysIndirect.
 *
 * Built only when the ogles3 SDK is there (tests/Makefile).
 */
#include <proto/ogles3.h>
#include <GLES3/gl31.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <stdio.h>
#include <string.h>
#include "window.h"

static int failures;

static void check(const char *what, int ok) {
	if (!ok) ++failures;
	printf("e03: %s %s\n", ok ? "PASS" : "FAIL", what);
}

static void checkInt(const char *what, GLint value, GLint expected) {
	if (value != expected) ++failures;
	printf("e03: %s %s: %ld, expected %ld\n", value == expected ? "PASS" : "FAIL", what, (long) value, (long) expected);
}

static void expectAt(const char *what, int x, int y, int r, int g, int b) {
	GLubyte p[4] = {0, 0, 0, 0};
	int ok;
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, p);
	ok = p[0] >= r - 4 && p[0] <= r + 4 && p[1] >= g - 4 && p[1] <= g + 4 && p[2] >= b - 4 && p[2] <= b + 4;
	if (!ok) ++failures;
	printf("e03: %s %s: (%d, %d, %d), expected (%d, %d, %d)\n", ok ? "PASS" : "FAIL", what, p[0], p[1], p[2], r, g, b);
}

static GLuint shader(GLenum type, const char *source) {
	GLuint s = glCreateShader(type);
	GLint ok = 0;
	glShaderSource(s, 1, &source, NULL);
	glCompileShader(s);
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetShaderInfoLog(s, sizeof(log), NULL, log);
		printf("e03: shader log: %s\n", log);
	}
	return s;
}

static GLuint program(GLuint a, GLuint b) {
	GLuint p = glCreateProgram();
	GLint ok = 0;
	glAttachShader(p, a);
	if (b) glAttachShader(p, b);
	glLinkProgram(p);
	glGetProgramiv(p, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetProgramInfoLog(p, sizeof(log), NULL, log);
		printf("e03: program log: %s\n", log);
	}
	return p;
}

/* Each invocation writes its value and counts itself. */
static const char *fill =
	"#version 310 es\n"
	"layout(local_size_x = 4) in;\n"
	"layout(std430, binding = 0) buffer Values { uint v[]; };\n"
	"layout(binding = 0, offset = 0) uniform atomic_uint counter;\n"
	"void main() {\n"
	"  uint i = gl_GlobalInvocationID.x;\n"
	"  v[i] = i * 3u + 7u;\n"
	"  atomicCounterIncrement(counter);\n"
	"}\n";
static const char *add =
	"#version 310 es\n"
	"layout(local_size_x = 4) in;\n"
	"layout(std430, binding = 0) buffer Values { uint v[]; };\n"
	"void main() { v[gl_GlobalInvocationID.x] += 100u; }\n";
static const char *paint =
	"#version 310 es\n"
	"layout(local_size_x = 8, local_size_y = 8) in;\n"
	"layout(rgba8, binding = 0) writeonly uniform highp image2D img;\n"
	"void main() { imageStore(img, ivec2(gl_GlobalInvocationID.xy), vec4(0.0, 1.0, 0.0, 1.0)); }\n";
/* A quad over the right half of the window, as a fan, written by the GPU. */
static const char *quad =
	"#version 310 es\n"
	"layout(local_size_x = 4) in;\n"
	"layout(std430, binding = 1) buffer Positions { vec2 p[]; };\n"
	"void main() {\n"
	"  const vec2 corners[4] = vec2[4](vec2(0.0, -1.0), vec2(1.0, -1.0), vec2(1.0, 1.0), vec2(0.0, 1.0));\n"
	"  p[gl_LocalInvocationID.x] = corners[gl_LocalInvocationID.x];\n"
	"}\n";
static const char *vertex =
	"#version 310 es\n"
	"layout(location = 0) in vec2 pos;\n"
	"void main() { gl_Position = vec4(pos, 0.0, 1.0); }\n";
static const char *fragment =
	"#version 310 es\n"
	"precision mediump float;\n"
	"out vec4 colour;\n"
	"void main() { colour = vec4(0.0, 0.0, 1.0, 1.0); }\n";

int main(void) {
	struct Window *win;
	void *ctx;
	ULONG err = 0;
	GLuint buf[5], tex, fbo, vao;
	static const GLuint zero[16] = {0};
	static const GLuint groups[3] = {2, 1, 1};
	static const GLuint draw[4] = {4, 1, 0, 0};

	write_label("e03_ogles31_compute");
	if (!OGLES3Open()) {
		printf("e03: FAIL no ogles3.library\n");
		return 20;
	}
	if (!(win = open_window("e03: ogles3 compute"))) {
		printf("e03: FAIL no window\n");
		OGLES3Close();
		return 20;
	}
	ctx = aglCreateContextTags2(&err, OGLES3_CCT_WINDOW, (ULONG) win, OGLES3_CCT_PISTORM_ES_VERSION, 31, TAG_DONE);
	if (!ctx) {
		printf("e03: FAIL aglCreateContextTags2, error %lu\n", (unsigned long) err);
		CloseWindow(win);
		OGLES3Close();
		return 20;
	}
	aglMakeCurrent(ctx);
	check("GL_VERSION is OpenGL ES 3.1", !strcmp((const char *) glGetString(GL_VERSION), "OpenGL ES 3.1 ogles3.library 1.0"));
	{
		GLint major = 0, minor = -1;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		check("GL_MAJOR_VERSION 3, GL_MINOR_VERSION 1", major == 3 && minor == 1);
	}

	glGenBuffers(5, buf);
	/* buf[0]: 16 uints, buf[1]: the counter, buf[2]: dispatch command,
	 * buf[3]: 4 vec2 positions, buf[4]: draw command. */
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(zero), zero, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buf[0]);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, buf[1]);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, zero, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buf[1]);
	{
		GLint bound = 0;
		glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 0, &bound);
		checkInt("glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 0)", bound, (GLint) buf[0]);
	}

	glUseProgram(program(shader(GL_COMPUTE_SHADER, fill), 0));
	glDispatchCompute(4, 1, 1);
	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
	{
		const GLuint *v = (const GLuint *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(zero), GL_MAP_READ_BIT);
		check("storage buffer written by a compute shader", v && v[0] == 7 && v[1] == 10 && v[15] == 52);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
	{
		const GLuint *n = (const GLuint *) glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, 4, GL_MAP_READ_BIT);
		checkInt("atomic counter after 16 invocations", n ? (GLint) n[0] : -1, 16);
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	}

	/* Two groups of 4 by glDispatchComputeIndirect: the first 8 values. */
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buf[2]);
	glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(groups), groups, GL_STATIC_DRAW);
	glUseProgram(program(shader(GL_COMPUTE_SHADER, add), 0));
	glDispatchComputeIndirect(0);
	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
	{
		GLuint v[16];
		memset(v, 0, sizeof(v));
		/* Read as glBufferSubData's reverse would: through a mapping. */
		{
			const GLuint *m = (const GLuint *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(v), GL_MAP_READ_BIT);
			if (m) memcpy(v, m, sizeof(v));
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		check("glDispatchComputeIndirect: 8 values added to", v[0] == 107 && v[7] == 128 && v[8] == 31 && v[15] == 52);
	}

	/* imageStore into a texture, read back through a framebuffer. */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 16, 16);
	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glUseProgram(program(shader(GL_COMPUTE_SHADER, paint), 0));
	glDispatchCompute(2, 2, 1);
	glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	expectAt("image written with imageStore", 9, 9, 0, 255, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* Vertices a compute shader wrote, drawn by glDrawArraysIndirect. */
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf[3]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 32, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buf[3]);
	glUseProgram(program(shader(GL_COMPUTE_SHADER, quad), 0));
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buf[3]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf[4]);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(draw), draw, GL_STATIC_DRAW);
	glUseProgram(program(shader(GL_VERTEX_SHADER, vertex), shader(GL_FRAGMENT_SHADER, fragment)));
	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArraysIndirect(GL_TRIANGLE_FAN, 0);
	expectAt("glDrawArraysIndirect of GPU-written vertices: right", 3 * WIDTH / 4, HEIGHT / 2, 0, 0, 255);
	expectAt("glDrawArraysIndirect of GPU-written vertices: left", WIDTH / 4, HEIGHT / 2, 0, 0, 0);

	checkInt("glGetError", (GLint) glGetError(), GL_NO_ERROR);
	glDeleteVertexArrays(1, &vao);
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &tex);
	glDeleteBuffers(5, buf);
	aglDestroyContext(ctx);
	CloseWindow(win);
	OGLES3Close();
	printf("e03: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 10 : 0;
}
