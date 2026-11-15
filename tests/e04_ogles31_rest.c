/*
 * e04_ogles31_rest: the rest of OpenGL ES 3.1 in ogles3.library (docs/
 * phase10-ogles3.md, stage 3b): separable programs made by glCreateShader-
 * Programv in a program pipeline, glProgramUniform* on a program that is not
 * current, the program interface queries, separate vertex attribute formats
 * and bindings (interleaved data at an offset, a per-instance binding, a
 * binding switched to another buffer, kept by vertex array objects),
 * multisample textures read with texelFetch, the sample mask, and the small
 * queries (glGetTexLevelParameter*, glGetMultisamplefv, glFramebuffer-
 * Parameteri).
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
	printf("e04: %s %s\n", ok ? "PASS" : "FAIL", what);
}

static void checkInt(const char *what, GLint value, GLint expected) {
	if (value != expected) ++failures;
	printf("e04: %s %s: %ld, expected %ld\n", value == expected ? "PASS" : "FAIL", what, (long) value, (long) expected);
}

static void expectAt(const char *what, int x, int y, int r, int g, int b) {
	GLubyte p[4] = {0, 0, 0, 0};
	int ok;
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, p);
	ok = p[0] >= r - 4 && p[0] <= r + 4 && p[1] >= g - 4 && p[1] <= g + 4 && p[2] >= b - 4 && p[2] <= b + 4;
	if (!ok) ++failures;
	printf("e04: %s %s: (%d, %d, %d), expected (%d, %d, %d)\n", ok ? "PASS" : "FAIL", what, p[0], p[1], p[2], r, g, b);
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
		printf("e04: shader log: %s\n", log);
	}
	return s;
}

static void programLog(GLuint p) {
	GLint ok = 0;
	glGetProgramiv(p, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetProgramInfoLog(p, sizeof(log), NULL, log);
		printf("e04: program %lu log: %s\n", (unsigned long) p, log);
	}
}

static GLuint program(GLuint a, GLuint b) {
	GLuint p = glCreateProgram();
	glAttachShader(p, a);
	if (b) glAttachShader(p, b);
	glLinkProgram(p);
	programLog(p);
	return p;
}

/* The separable stages of the pipeline. */
static const char *sepVertex =
	"#version 310 es\n"
	"layout(location = 0) in vec2 pos;\n"
	"void main() { gl_Position = vec4(pos, 0.0, 1.0); }\n";
static const char *sepFragment =
	"#version 310 es\n"
	"precision mediump float;\n"
	"uniform vec4 colour;\n"
	"layout(location = 0) out vec4 outColour;\n"
	"void main() { outColour = colour; }\n";

/* Interleaved position and colour, a per-instance shift, a matrix, a tint
 * and a flag set with glProgramUniform*. */
static const char *tintVertex =
	"#version 310 es\n"
	"layout(location = 0) in vec2 pos;\n"
	"layout(location = 1) in vec4 col;\n"
	"layout(location = 2) in vec2 shift;\n"
	"uniform mat2 m;\n"
	"out vec4 vCol;\n"
	"void main() { vCol = col; gl_Position = vec4(m * pos + shift, 0.0, 1.0); }\n";
static const char *tintFragment =
	"#version 310 es\n"
	"precision mediump float;\n"
	"uniform vec4 tint;\n"
	"uniform uint flags;\n"
	"in vec4 vCol;\n"
	"layout(location = 0) out vec4 outColour;\n"
	"void main() { outColour = vCol * tint * (flags == 7u ? 1.0 : 0.0); }\n";

/* Sample 0 of a multisample texture, tiled. */
static const char *msFragment =
	"#version 310 es\n"
	"precision mediump float;\n"
	"uniform highp sampler2DMS ms;\n"
	"layout(location = 0) out vec4 outColour;\n"
	"void main() { outColour = texelFetch(ms, ivec2(gl_FragCoord.xy) % 16, 0); }\n";

static const char *storage =
	"#version 310 es\n"
	"layout(local_size_x = 1) in;\n"
	"layout(std430, binding = 3) buffer Values { uint v[]; };\n"
	"void main() { v[0] = 1u; }\n";

/* A vertex as the interleaved buffers hold it: 12 bytes. */
struct Vertex {
	GLfloat x, y;
	GLubyte r, g, b, a;
};

int main(void) {
	struct Window *win;
	void *ctx;
	ULONG err = 0;
	GLuint buf[5], vao[2], pipe, vp, fp, tint, msProg, tex[2], fbo[2];
	/* A quad over the left half, and one over the whole window. */
	static const GLfloat left[8] = {-1, -1, 0, -1, 0, 1, -1, 1};
	static const GLfloat whole[8] = {-1, -1, 1, -1, 1, 1, -1, 1};
	/* 12 bytes not read, then a quad from x 0 to 0.5, magenta. */
	static const struct Vertex magenta[5] = {
		{0, 0, 0, 0, 0, 0},
		{0, -1, 255, 0, 255, 255}, {0.5f, -1, 255, 0, 255, 255}, {0.5f, 1, 255, 0, 255, 255}, {0, 1, 255, 0, 255, 255}
	};
	static const struct Vertex green[4] = {
		{0, -1, 0, 255, 0, 255}, {0.5f, -1, 0, 255, 0, 255}, {0.5f, 1, 0, 255, 0, 255}, {0, 1, 0, 255, 0, 255}
	};
	static const GLfloat shifts[4] = {0, 0, 0.5f, 0};
	static const GLfloat identity[4] = {1, 0, 0, 1};

	write_label("e04_ogles31_rest");
	if (!OGLES3Open()) {
		printf("e04: FAIL no ogles3.library\n");
		return 20;
	}
	if (!(win = open_window("e04: ogles3 3.1 rest"))) {
		printf("e04: FAIL no window\n");
		OGLES3Close();
		return 20;
	}
	ctx = aglCreateContextTags2(&err, OGLES3_CCT_WINDOW, (ULONG) win, OGLES3_CCT_PISTORM_ES_VERSION, 31, TAG_DONE);
	if (!ctx) {
		printf("e04: FAIL aglCreateContextTags2, error %lu\n", (unsigned long) err);
		CloseWindow(win);
		OGLES3Close();
		return 20;
	}
	aglMakeCurrent(ctx);
	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0, 0, 0, 1);

	glGenBuffers(5, buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(left), left, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(whole), whole, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(magenta), magenta, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(green), green, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shifts), shifts, GL_STATIC_DRAW);
	glGenVertexArrays(2, vao);

	/* --- A pipeline of two separable programs --------------------------- */
	vp = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &sepVertex);
	fp = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &sepFragment);
	check("glCreateShaderProgramv made two programs", vp && fp);
	programLog(vp);
	programLog(fp);
	{
		GLint separable = 0;
		glGetProgramiv(vp, GL_PROGRAM_SEPARABLE, &separable);
		checkInt("GL_PROGRAM_SEPARABLE of glCreateShaderProgramv's program", separable, GL_TRUE);
	}
	glGenProgramPipelines(1, &pipe);
	glUseProgramStages(pipe, GL_VERTEX_SHADER_BIT, vp);
	glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, fp);
	glBindProgramPipeline(pipe);
	check("glIsProgramPipeline", glIsProgramPipeline(pipe) == GL_TRUE);
	{
		GLint v = 0, f = 0, valid = 0;
		char log[128];
		GLsizei length = -1;
		glGetProgramPipelineiv(pipe, GL_VERTEX_SHADER, &v);
		glGetProgramPipelineiv(pipe, GL_FRAGMENT_SHADER, &f);
		check("glGetProgramPipelineiv names the stages' programs", v == (GLint) vp && f == (GLint) fp);
		glValidateProgramPipeline(pipe);
		glGetProgramPipelineiv(pipe, GL_VALIDATE_STATUS, &valid);
		checkInt("GL_VALIDATE_STATUS of the pipeline", valid, GL_TRUE);
		glGetProgramPipelineInfoLog(pipe, sizeof(log), &length, log);
		check("glGetProgramPipelineInfoLog gives a length", length >= 0 && length < (GLsizei) sizeof(log));
	}
	glProgramUniform4f(fp, glGetUniformLocation(fp, "colour"), 1, 0, 0, 1);
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	expectAt("pipeline draws with glProgramUniform4f's colour", WIDTH / 4, HEIGHT / 2, 255, 0, 0);
	expectAt("pipeline: right half untouched", 3 * WIDTH / 4, HEIGHT / 2, 0, 0, 0);
	/* glUniform* with no current program: the pipeline's active program. */
	glActiveShaderProgram(pipe, fp);
	{
		GLint active = 0;
		glGetProgramPipelineiv(pipe, GL_ACTIVE_PROGRAM, &active);
		checkInt("GL_ACTIVE_PROGRAM of the pipeline", active, (GLint) fp);
	}
	glUniform4f(glGetUniformLocation(fp, "colour"), 0, 1, 0, 1);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	expectAt("glUniform4f into the pipeline's active program", WIDTH / 4, HEIGHT / 2, 0, 255, 0);
	glBindProgramPipeline(0);

	/* --- glProgramUniform* on a program that is not current ------------- */
	tint = program(shader(GL_VERTEX_SHADER, tintVertex), shader(GL_FRAGMENT_SHADER, tintFragment));
	msProg = program(shader(GL_VERTEX_SHADER, sepVertex), shader(GL_FRAGMENT_SHADER, msFragment));
	glUseProgram(msProg);
	glProgramUniform4f(tint, glGetUniformLocation(tint, "tint"), 0.5f, 1, 1, 1);
	glProgramUniform1ui(tint, glGetUniformLocation(tint, "flags"), 7);
	glProgramUniformMatrix2fv(tint, glGetUniformLocation(tint, "m"), 1, GL_FALSE, identity);
	{
		GLint current = 0;
		GLfloat t[4] = {0, 0, 0, 0};
		GLuint flags = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &current);
		checkInt("glProgramUniform* leaves the current program", current, (GLint) msProg);
		glGetUniformfv(tint, glGetUniformLocation(tint, "tint"), t);
		check("glGetUniformfv reads glProgramUniform4f's value", t[0] == 0.5f && t[1] == 1 && t[3] == 1);
		glGetUniformuiv(tint, glGetUniformLocation(tint, "flags"), &flags);
		checkInt("glGetUniformuiv reads glProgramUniform1ui's value", (GLint) flags, 7);
	}

	/* --- The program interface queries ---------------------------------- */
	{
		GLint n = 0, maxLength = 0, values[3] = {0, 0, 0};
		GLuint index;
		char name[32];
		GLsizei length = 0;
		static const GLenum props[3] = {GL_TYPE, GL_LOCATION, GL_ARRAY_SIZE};
		glGetProgramInterfaceiv(tint, GL_UNIFORM, GL_ACTIVE_RESOURCES, &n);
		checkInt("GL_ACTIVE_RESOURCES of GL_UNIFORM", n, 3);
		glGetProgramInterfaceiv(tint, GL_PROGRAM_INPUT, GL_MAX_NAME_LENGTH, &maxLength);
		checkInt("GL_MAX_NAME_LENGTH of GL_PROGRAM_INPUT", maxLength, 6);
		index = glGetProgramResourceIndex(tint, GL_UNIFORM, "tint");
		check("glGetProgramResourceIndex finds a uniform", index != GL_INVALID_INDEX);
		memset(name, 0, sizeof(name));
		glGetProgramResourceName(tint, GL_UNIFORM, index, sizeof(name), &length, name);
		check("glGetProgramResourceName gives its name back", !strcmp(name, "tint") && length == 4);
		length = 0;
		glGetProgramResourceiv(tint, GL_UNIFORM, index, 3, props, 3, &length, values);
		check("glGetProgramResourceiv: type, location, array size",
			length == 3 && values[0] == GL_FLOAT_VEC4 && values[1] == glGetUniformLocation(tint, "tint") && values[2] == 1);
		checkInt("glGetProgramResourceLocation of an input", glGetProgramResourceLocation(tint, GL_PROGRAM_INPUT, "col"), 1);
		checkInt("glGetProgramResourceLocation of an output", glGetProgramResourceLocation(tint, GL_PROGRAM_OUTPUT, "outColour"), 0);
		check("glGetProgramResourceIndex of a name not there",
			glGetProgramResourceIndex(tint, GL_UNIFORM, "nothing") == GL_INVALID_INDEX);
	}
	{
		GLuint compute = program(shader(GL_COMPUTE_SHADER, storage), 0), index;
		GLint binding = -1;
		static const GLenum prop = GL_BUFFER_BINDING;
		index = glGetProgramResourceIndex(compute, GL_SHADER_STORAGE_BLOCK, "Values");
		check("glGetProgramResourceIndex of a storage block", index != GL_INVALID_INDEX);
		glGetProgramResourceiv(compute, GL_SHADER_STORAGE_BLOCK, index, 1, &prop, 1, NULL, &binding);
		checkInt("GL_BUFFER_BINDING of the storage block", binding, 3);
		glDeleteProgram(compute);
	}

	/* --- Separate attribute formats and bindings ------------------------ */
	glUseProgram(tint);
	glBindVertexArray(vao[1]);
	glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribFormat(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 8);
	glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(0, 0);
	glVertexAttribBinding(1, 0);
	glVertexAttribBinding(2, 1);
	glBindVertexBuffer(0, buf[2], sizeof(struct Vertex), sizeof(struct Vertex));
	glBindVertexBuffer(1, buf[4], 0, 8);
	glVertexBindingDivisor(1, 1);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 2);
	expectAt("interleaved binding at an offset, instance 0", 5 * WIDTH / 8, HEIGHT / 2, 128, 0, 255);
	expectAt("per-instance binding, instance 1", 7 * WIDTH / 8, HEIGHT / 2, 128, 0, 255);
	expectAt("bindings: left half untouched", WIDTH / 4, HEIGHT / 2, 0, 0, 0);
	{
		GLint v = -1;
		glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_BINDING, &v);
		checkInt("GL_VERTEX_ATTRIB_BINDING of attribute 1", v, 0);
		glGetVertexAttribiv(2, GL_VERTEX_ATTRIB_BINDING, &v);
		checkInt("GL_VERTEX_ATTRIB_BINDING of attribute 2", v, 1);
		glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_RELATIVE_OFFSET, &v);
		checkInt("GL_VERTEX_ATTRIB_RELATIVE_OFFSET of attribute 1", v, 8);
		glGetIntegeri_v(GL_VERTEX_BINDING_STRIDE, 0, &v);
		checkInt("GL_VERTEX_BINDING_STRIDE of binding 0", v, 12);
		glGetIntegeri_v(GL_VERTEX_BINDING_OFFSET, 0, &v);
		checkInt("GL_VERTEX_BINDING_OFFSET of binding 0", v, 12);
		glGetIntegeri_v(GL_VERTEX_BINDING_DIVISOR, 1, &v);
		checkInt("GL_VERTEX_BINDING_DIVISOR of binding 1", v, 1);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &v);
		checkInt("GL_MAX_VERTEX_ATTRIB_BINDINGS", v, 16);
	}
	/* The binding switched to another buffer: both attributes follow. */
	glBindVertexBuffer(0, buf[3], 0, sizeof(struct Vertex));
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 2);
	expectAt("binding switched to another buffer", 5 * WIDTH / 8, HEIGHT / 2, 0, 255, 0);
	/* Vertex array objects keep their bindings. */
	{
		GLint b = 0;
		glBindVertexArray(vao[0]);
		glGetIntegeri_v(GL_VERTEX_BINDING_BUFFER, 0, &b);
		checkInt("vertex array 1: glVertexAttribPointer's binding 0", b, (GLint) buf[0]);
		glBindVertexArray(vao[1]);
		glGetIntegeri_v(GL_VERTEX_BINDING_BUFFER, 0, &b);
		checkInt("vertex array 2: its binding 0 again", b, (GLint) buf[3]);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 2);
		expectAt("vertex array 2 draws as before", 7 * WIDTH / 8, HEIGHT / 2, 0, 255, 0);
	}

	/* --- Multisample textures and the sample mask ----------------------- */
	glGenTextures(2, tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex[0]);
	glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, 16, 16, GL_TRUE);
	{
		GLint v = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_TEXTURE_WIDTH, &v);
		checkInt("GL_TEXTURE_WIDTH of the multisample texture", v, 16);
		glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_TEXTURE_SAMPLES, &v);
		check("GL_TEXTURE_SAMPLES at least 4", v >= 4);
		printf("e04: (GL_TEXTURE_SAMPLES %ld)\n", (long) v);
		glGetTexLevelParameteriv(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_TEXTURE_FIXED_SAMPLE_LOCATIONS, &v);
		checkInt("GL_TEXTURE_FIXED_SAMPLE_LOCATIONS", v, GL_TRUE);
	}
	glGenFramebuffers(2, fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex[0], 0);
	checkInt("multisample framebuffer complete", (GLint) glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
	{
		GLfloat position[2] = {-1, -1};
		glGetMultisamplefv(GL_SAMPLE_POSITION, 0, position);
		check("glGetMultisamplefv: a position in the pixel",
			position[0] >= 0 && position[0] <= 1 && position[1] >= 0 && position[1] <= 1);
		printf("e04: (sample 0 at %ld/1000, %ld/1000)\n", (long) (position[0] * 1000), (long) (position[1] * 1000));
	}
	glViewport(0, 0, 16, 16);
	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	/* With every sample masked out, the red quad leaves the blue. */
	glUseProgram(0);
	glBindProgramPipeline(pipe);
	glProgramUniform4f(fp, glGetUniformLocation(fp, "colour"), 1, 0, 0, 1);
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, buf[1]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnable(GL_SAMPLE_MASK);
	glSampleMaski(0, 0);
	{
		GLint mask = -1;
		glGetIntegeri_v(GL_SAMPLE_MASK_VALUE, 0, &mask);
		checkInt("GL_SAMPLE_MASK_VALUE after glSampleMaski(0, 0)", mask, 0);
	}
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glSampleMaski(0, ~0u);
	glDisable(GL_SAMPLE_MASK);
	glBindProgramPipeline(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);
	/* Sample 0 of it, through a sampler2DMS. */
	glUseProgram(msProg);
	glUniform1i(glGetUniformLocation(msProg, "ms"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex[0]);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	expectAt("texelFetch of the multisample texture, all samples masked", WIDTH / 2, HEIGHT / 2, 0, 0, 255);
	/* The same, unmasked: red. */
	glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
	glViewport(0, 0, 16, 16);
	glUseProgram(0);
	glBindProgramPipeline(pipe);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindProgramPipeline(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);
	glUseProgram(msProg);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	expectAt("texelFetch of the multisample texture, drawn into", WIDTH / 2, HEIGHT / 2, 255, 0, 0);

	/* --- The small queries ---------------------------------------------- */
	{
		GLfloat h = 0;
		glBindTexture(GL_TEXTURE_2D, tex[1]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 8, 4);
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
		check("glGetTexLevelParameterfv: GL_TEXTURE_HEIGHT 4.0", h == 4.0f);
	}
	{
		GLint w = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
		glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 32);
		glGetFramebufferParameteriv(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, &w);
		checkInt("glFramebufferParameteri's GL_FRAMEBUFFER_DEFAULT_WIDTH", w, 32);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	/* --- Framebuffer 0 ---------------------------------------------------
	   is the window's picture, also on the host side: queries answer 0 for
	   it, GL_BACK names its colour buffer, and the swap shows it whatever
	   framebuffer is bound then (yellow, not fbo[0]'s red). */
	{
		GLint b = -1, type = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &b);
		checkInt("GL_FRAMEBUFFER_BINDING after binding 0", b, 0);
		b = -1;
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &b);
		checkInt("GL_READ_FRAMEBUFFER_BINDING after binding 0", b, 0);
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
		checkInt("framebuffer 0's GL_BACK is GL_FRAMEBUFFER_DEFAULT", type, GL_FRAMEBUFFER_DEFAULT);
		{
			GLenum back = GL_BACK;
			glDrawBuffers(1, &back);
		}
		glUseProgram(0);
		glClearColor(1, 1, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		expectAt("framebuffer 0 cleared yellow", 3, 3, 255, 255, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
		aglSwapBuffers();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		expectAt("framebuffer 0 still yellow after a swap", 3, 3, 255, 255, 0);
		/* Deleting the bound framebuffer binds 0 again. */
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
		glDeleteFramebuffers(1, &fbo[1]);
		fbo[1] = 0;
		b = -1;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &b);
		checkInt("GL_FRAMEBUFFER_BINDING after deleting the bound framebuffer", b, 0);
		expectAt("framebuffer 0 yellow after that", 3, 3, 255, 255, 0);
	}

	checkInt("glGetError", (GLint) glGetError(), GL_NO_ERROR);
	glDeleteFramebuffers(2, fbo);
	glDeleteTextures(2, tex);
	glDeleteProgramPipelines(1, &pipe);
	glDeleteProgram(vp);
	glDeleteProgram(fp);
	glDeleteProgram(tint);
	glDeleteProgram(msProg);
	glDeleteVertexArrays(2, vao);
	glDeleteBuffers(5, buf);
	aglDestroyContext(ctx);
	CloseWindow(win);
	OGLES3Close();
	printf("e04: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 10 : 0;
}
