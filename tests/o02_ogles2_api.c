/*
 * o02_ogles2_api: the rest of ogles2.library's GLES2 entries (docs/phase9-
 * ogles2.md, stage 3), each checked by reading its result back: glGet*
 * queries, uniform arrays and matrices, the program queries, glBufferSub-
 * Data and glMapBufferOES, a GL_LUMINANCE texture (stored as GL_RED and
 * swizzled on the host) and glTexSubImage2D, a GL_UNSIGNED_SHORT_5_6_5
 * texture (byte-swapped on the host), constant vertex attributes,
 * glBlendColor, glDrawElementsBaseVertexOES and glReadPixels.
 *
 * Built only when the ogles2 SDK headers are there (tests/Makefile).
 */
#include <proto/ogles2.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <string.h>
#include "window.h"

static int failures;

static void check(const char *what, int ok) {
	if (!ok) ++failures;
	printf("o02: %s %s\n", ok ? "PASS" : "FAIL", what);
}

static int near(float a, float b) { return a > b - 0.01f && a < b + 0.01f; }

static void checkInt(const char *what, GLint value, GLint expected) {
	if (value != expected) ++failures;
	printf("o02: %s %s: %ld, expected %ld\n", value == expected ? "PASS" : "FAIL", what, (long) value, (long) expected);
}

/* The colour at the window's centre, read back with glReadPixels. */
static void expectPixel(const char *what, int r, int g, int b) {
	GLubyte p[4] = {0, 0, 0, 0};
	int ok;
	glReadPixels(WIDTH / 2, HEIGHT / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, p);
	ok = p[0] >= r - 3 && p[0] <= r + 3 && p[1] >= g - 3 && p[1] <= g + 3 && p[2] >= b - 3 && p[2] <= b + 3;
	if (!ok) ++failures;
	printf("o02: %s %s: (%d, %d, %d), expected (%d, %d, %d)\n", ok ? "PASS" : "FAIL", what, p[0], p[1], p[2], r, g, b);
}

static const char *vertexSource =
	"attribute vec2 pos;\n"
	"attribute vec4 tint;\n"
	"varying vec4 v_tint;\n"
	"uniform vec4 u_v[2];\n"
	"uniform mat3 u_m;\n"
	"void main() {\n"
	"  v_tint = tint * (u_v[0] + u_v[1]) * u_m[0][0];\n"
	"  gl_Position = vec4(pos, 0.0, 1.0);\n"
	"}\n";

static const char *fragmentSource =
	"precision mediump float;\n"
	"varying vec4 v_tint;\n"
	"uniform sampler2D tex;\n"
	"void main() { gl_FragColor = texture2D(tex, vec2(0.5, 0.5)) * v_tint; }\n";

static GLuint shader(GLenum type, const char *source) {
	GLuint s = glCreateShader(type);
	GLint ok = 0;
	glShaderSource(s, 1, &source, NULL);
	glCompileShader(s);
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetShaderInfoLog(s, sizeof(log), NULL, log);
		printf("o02: shader log: %s\n", log);
	}
	check("shader compiles", ok);
	return s;
}

static void program(GLuint p) {
	GLint value = 0, size = 0, attached = 0;
	GLenum type = 0;
	GLsizei length = 0;
	GLuint shaders[4];
	char name[64];
	GLfloat v[8] = {0.25f, 0.5f, 0.0f, 1.0f, 0.25f, 0.0f, 0.5f, 0.0f}, back[16];
	GLfloat m[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
	int i, found = 0;

	glGetProgramiv(p, GL_ACTIVE_UNIFORMS, &value);
	check("GL_ACTIVE_UNIFORMS >= 3", value >= 3);
	for (i = 0; i < value; ++i) {
		glGetActiveUniform(p, (GLuint) i, sizeof(name), &length, &size, &type, name);
		if (!strncmp(name, "u_v", 3)) {
			found = 1;
			checkInt("u_v size", size, 2);
			checkInt("u_v type", (GLint) type, GL_FLOAT_VEC4);
			checkInt("u_v name length", length, (GLint) strlen(name));
		}
	}
	check("glGetActiveUniform finds u_v", found);
	found = 0;
	glGetProgramiv(p, GL_ACTIVE_ATTRIBUTES, &value);
	for (i = 0; i < value; ++i) {
		glGetActiveAttrib(p, (GLuint) i, sizeof(name), &length, &size, &type, name);
		if (!strcmp(name, "tint")) found = type == GL_FLOAT_VEC4;
	}
	check("glGetActiveAttrib finds vec4 tint", found);
	glGetAttachedShaders(p, 4, &attached, shaders);
	checkInt("glGetAttachedShaders", attached, 2);

	glUseProgram(p);
	glUniform4fv(glGetUniformLocation(p, "u_v"), 2, v);
	glUniformMatrix3fv(glGetUniformLocation(p, "u_m"), 1, GL_FALSE, m);
	glUniform1i(glGetUniformLocation(p, "tex"), 0);
	memset(back, 0, sizeof(back));
	glGetUniformfv(p, glGetUniformLocation(p, "u_v[1]"), back);
	check("glUniform4fv/glGetUniformfv u_v[1]", near(back[0], 0.25f) && near(back[2], 0.5f) && near(back[3], 0.0f));
	glGetUniformfv(p, glGetUniformLocation(p, "u_m"), back);
	check("glUniformMatrix3fv/glGetUniformfv u_m", near(back[0], 1) && near(back[4], 1) && near(back[8], 1) && near(back[1], 0));
	glGetUniformiv(p, glGetUniformLocation(p, "tex"), &value);
	checkInt("glGetUniformiv tex", value, 0);
}

static void queries(void) {
	GLint v[4] = {0, 0, 0, 0};
	GLfloat f[4] = {0, 0, 0, 0};
	GLboolean b = GL_FALSE;
	GLint range[2], precision;

	glGetIntegerv(GL_VIEWPORT, v);
	check("GL_VIEWPORT", v[0] == 0 && v[1] == 0 && v[2] == WIDTH && v[3] == HEIGHT);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, v);
	checkInt("GL_MAX_VERTEX_ATTRIBS", v[0], 8);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, v);
	check("GL_MAX_VERTEX_UNIFORM_VECTORS >= 128", v[0] >= 128);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, v);
	check("GL_MAX_TEXTURE_SIZE >= 64", v[0] >= 64);
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, v);
	checkInt("GL_NUM_COMPRESSED_TEXTURE_FORMATS", v[0], 0);
	glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
	glGetFloatv(GL_COLOR_CLEAR_VALUE, f);
	check("GL_COLOR_CLEAR_VALUE", near(f[0], 0.25f) && near(f[1], 0.5f) && near(f[2], 0.75f) && near(f[3], 1));
	glDepthMask(GL_TRUE);
	glGetBooleanv(GL_DEPTH_WRITEMASK, &b);
	check("GL_DEPTH_WRITEMASK", b == GL_TRUE);
	glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range, &precision);
	check("glGetShaderPrecisionFormat highp float", range[0] >= 62 && precision >= 16);
}

static void buffers(GLuint buffer) {
	static const GLfloat first[4] = {1, 2, 3, 4};
	static const GLfloat second[2] = {5, 6};
	GLfloat *mapped;
	GLint size = 0, usage = 0;

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	check("glIsBuffer", glIsBuffer(buffer));
	glBufferData(GL_ARRAY_BUFFER, sizeof(first), first, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 4, sizeof(second), second);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	checkInt("GL_BUFFER_SIZE", size, (GLint) sizeof(first));
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_USAGE, &usage);
	checkInt("GL_BUFFER_USAGE", usage, GL_DYNAMIC_DRAW);
	mapped = (GLfloat *) glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
	check("glMapBufferOES", mapped != NULL);
	if (!mapped) return;
	check("glBufferSubData then mapped contents", mapped[0] == 1 && mapped[1] == 5 && mapped[2] == 6 && mapped[3] == 4);
	glUnmapBufferOES(GL_ARRAY_BUFFER);
}

int main(void) {
	struct Window *win;
	void *ctx;
	ULONG err = 0;
	GLuint p, vs, fs, buf[2], tex[2];
	GLubyte lum = 128, lum2 = 200;
	GLushort red565 = 0xF800;
	/* Vertices 0-3 are never drawn: the base vertex (4) skips them. */
	static const GLfloat quad[16] = {0, 0, 0, 0, 0, 0, 0, 0, -1, -1, 1, -1, 1, 1, -1, 1};
	static const GLushort indices[6] = {0, 1, 2, 0, 2, 3};
	GLint shown;

	write_label("o02_ogles2_api");
	if (!OGLES2Open()) {
		printf("o02: FAIL no ogles2.library\n");
		return 20;
	}
	if (!(win = open_window("o02: ogles2 API"))) {
		printf("o02: FAIL no window\n");
		OGLES2Close();
		return 20;
	}
	ctx = aglCreateContextTags2(&err, OGLES2_CCT_WINDOW, (ULONG) win, OGLES2_CCT_DEPTH, 0, OGLES2_CCT_STENCIL, 0, TAG_DONE);
	if (!ctx) {
		printf("o02: FAIL aglCreateContextTags2, error %lu\n", (unsigned long) err);
		CloseWindow(win);
		OGLES2Close();
		return 20;
	}
	aglMakeCurrent(ctx);

	queries();

	vs = shader(GL_VERTEX_SHADER, vertexSource);
	fs = shader(GL_FRAGMENT_SHADER, fragmentSource);
	p = glCreateProgram();
	glAttachShader(p, vs);
	glAttachShader(p, fs);
	glBindAttribLocation(p, 0, "pos");
	glBindAttribLocation(p, 1, "tint");
	glLinkProgram(p);
	{
		GLint linked = 0, length = 0;
		glGetProgramiv(p, GL_LINK_STATUS, &linked);
		check("program links", linked);
		glGetShaderiv(vs, GL_SHADER_SOURCE_LENGTH, &length);
		checkInt("GL_SHADER_SOURCE_LENGTH", length, (GLint) strlen(vertexSource) + 1);
	}
	{
		char source[64];
		GLsizei length = 0;
		glGetShaderSource(fs, sizeof(source), &length, source);
		check("glGetShaderSource", length == sizeof(source) - 1 && !strncmp(source, fragmentSource, length));
	}
	program(p);

	glGenBuffers(2, buf);
	buffers(buf[0]);

	/* The quad: positions from buf[1] through glDrawElementsBaseVertexOES,
	 * the tint a constant attribute (u_v[0] + u_v[1] = (0.5, 0.5, 0.5, 1)
	 * times this (2, 2, 2, 1) = white). */
	glBindBuffer(GL_ARRAY_BUFFER, buf[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glVertexAttrib4f(1, 2, 2, 2, 1);
	{
		GLfloat current[4];
		GLint enabled = 1;
		glGetVertexAttribfv(1, GL_CURRENT_VERTEX_ATTRIB, current);
		check("glGetVertexAttribfv GL_CURRENT_VERTEX_ATTRIB", current[0] == 2 && current[3] == 1);
		glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
		checkInt("glGetVertexAttribiv enabled", enabled, 0);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &enabled);
		checkInt("GL_ARRAY_BUFFER_BINDING", enabled, (GLint) buf[1]);
	}

	glGenTextures(2, tex);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &lum);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	{
		GLint filter = 0;
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &filter);
		checkInt("glTexParameterf/glGetTexParameteriv", filter, GL_NEAREST);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawElementsBaseVertexOES(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices, 4);
	expectPixel("GL_LUMINANCE texture", 128, 128, 128);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_LUMINANCE, GL_UNSIGNED_BYTE, &lum2);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawElementsBaseVertexOES(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices, 4);
	expectPixel("glTexSubImage2D", 200, 200, 200);

	glEnable(GL_BLEND);
	glBlendColor(0.5f, 0.5f, 0.5f, 1.0f);
	glBlendFunc(GL_CONSTANT_COLOR, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawElementsBaseVertexOES(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices, 4);
	expectPixel("glBlendColor", 100, 100, 100);
	glDisable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, &red565);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawElementsBaseVertexOES(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices, 4);
	expectPixel("GL_UNSIGNED_SHORT_5_6_5 texture", 255, 0, 0);

	checkInt("glGetError", (GLint) glGetError(), GL_NO_ERROR);

	for (shown = 0; shown < FRAMES; ++shown) {
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawElementsBaseVertexOES(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices, 4);
		aglSwapBuffers();
	}
	wait_active(win);
	report_shown("o02_ogles2_api", win);

	glDeleteTextures(2, tex);
	glDeleteBuffers(2, buf);
	glDeleteProgram(p);
	glDeleteShader(vs);
	glDeleteShader(fs);
	printf("o02: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	aglDestroyContext(ctx);
	CloseWindow(win);
	OGLES2Close();
	return failures ? 5 : 0;
}
