/*
 * e02_ogles3_queries: the rest of OpenGL ES 3.0 in ogles3.library (docs/
 * phase10-ogles3.md, stage 2b): transform feedback captured into a buffer
 * and read back on the 68k with glMapBufferRange, an occlusion query, the
 * uniform block queries (glGetActiveUniformBlockiv/Name, glGetUniform-
 * Indices, glGetActiveUniformsiv) and glGetIntegeri_v of a uniform buffer
 * binding.
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
	printf("e02: %s %s\n", ok ? "PASS" : "FAIL", what);
}

static void checkInt(const char *what, GLint value, GLint expected) {
	if (value != expected) ++failures;
	printf("e02: %s %s: %ld, expected %ld\n", value == expected ? "PASS" : "FAIL", what, (long) value, (long) expected);
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
		printf("e02: shader log: %s\n", log);
	}
	return s;
}

/* Transform feedback: each vertex's position doubled, and an int. */
static const char *feedbackVertex =
	"#version 300 es\n"
	"layout(location = 0) in vec2 pos;\n"
	"out vec2 doubled;\n"
	"flat out int index;\n"
	"void main() {\n"
	"  doubled = pos * 2.0;\n"
	"  index = gl_VertexID + 100;\n"
	"  gl_Position = vec4(pos, 0.0, 1.0);\n"
	"}\n";
static const char *plainFragment =
	"#version 300 es\n"
	"precision mediump float;\n"
	"layout(std140) uniform Block { vec4 a; float b; vec4 c; };\n"
	"out vec4 colour;\n"
	"void main() { colour = a + vec4(b) + c; }\n";

int main(void) {
	struct Window *win;
	void *ctx;
	ULONG err = 0;
	GLuint p, vs, fs, buf[3], query;
	static const GLfloat points[6] = {0.25f, -0.5f, 1.0f, 0.75f, -1.0f, 0.125f};
	static const GLchar *varyings[2] = {"doubled", "index"};
	static const GLfloat block[12] = {0, 0, 0, 1, 0.5f, 0, 0, 0, 0, 1, 0, 0};

	write_label("e02_ogles3_queries");
	if (!OGLES3Open()) {
		printf("e02: FAIL no ogles3.library\n");
		return 20;
	}
	if (!(win = open_window("e02: ogles3 queries"))) {
		printf("e02: FAIL no window\n");
		OGLES3Close();
		return 20;
	}
	ctx = aglCreateContextTags2(&err, OGLES3_CCT_WINDOW, (ULONG) win, OGLES3_CCT_PISTORM_ES_VERSION, 30, TAG_DONE);
	if (!ctx) {
		printf("e02: FAIL aglCreateContextTags2, error %lu\n", (unsigned long) err);
		CloseWindow(win);
		OGLES3Close();
		return 20;
	}
	aglMakeCurrent(ctx);

	vs = shader(GL_VERTEX_SHADER, feedbackVertex);
	fs = shader(GL_FRAGMENT_SHADER, plainFragment);
	p = glCreateProgram();
	glAttachShader(p, vs);
	glAttachShader(p, fs);
	glTransformFeedbackVaryings(p, 2, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(p);
	{
		GLint ok = 0;
		glGetProgramiv(p, GL_LINK_STATUS, &ok);
		check("program with transform feedback varyings links", ok);
	}
	glUseProgram(p);
	{
		GLsizei length = 0, size = 0;
		GLenum type = 0;
		char name[16];
		glGetTransformFeedbackVarying(p, 1, sizeof(name), &length, &size, &type, name);
		check("glGetTransformFeedbackVarying", !strcmp(name, "index") && type == GL_INT && size == 1);
	}

	/* Uniform block queries. */
	{
		GLuint index = glGetUniformBlockIndex(p, "Block");
		GLint size = 0, count = 0, indices[3] = {-1, -1, -1}, offsets[3] = {-1, -1, -1};
		char name[16];
		GLsizei length = 0;
		static const GLchar *names[3] = {"a", "b", "c"};
		GLuint got[3];
		glGetActiveUniformBlockiv(p, index, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
		checkInt("GL_UNIFORM_BLOCK_DATA_SIZE (std140)", size, 48);
		glGetActiveUniformBlockiv(p, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &count);
		checkInt("GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS", count, 3);
		glGetActiveUniformBlockiv(p, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices);
		check("GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES", indices[0] >= 0 && indices[1] >= 0 && indices[2] >= 0);
		glGetActiveUniformBlockName(p, index, sizeof(name), &length, name);
		check("glGetActiveUniformBlockName", !strcmp(name, "Block") && length == 5);
		glGetUniformIndices(p, 3, names, got);
		check("glGetUniformIndices", got[0] != GL_INVALID_INDEX && got[1] != GL_INVALID_INDEX && got[2] != GL_INVALID_INDEX);
		glGetActiveUniformsiv(p, 3, got, GL_UNIFORM_OFFSET, offsets);
		check("glGetActiveUniformsiv(GL_UNIFORM_OFFSET): 0, 16, 32", offsets[0] == 0 && offsets[1] == 16 && offsets[2] == 32);
		glUniformBlockBinding(p, index, 1);
	}

	glGenBuffers(3, buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_UNIFORM_BUFFER, buf[2]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(block), block, GL_STATIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, buf[2], 0, sizeof(block));
	{
		GLint bound = 0, size = 0;
		glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, 1, &bound);
		checkInt("glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, 1)", bound, (GLint) buf[2]);
		glGetIntegeri_v(GL_UNIFORM_BUFFER_SIZE, 1, &size);
		checkInt("glGetIntegeri_v(GL_UNIFORM_BUFFER_SIZE, 1)", size, (GLint) sizeof(block));
	}

	/* Three points captured: 3 x (vec2 + int) = 36 bytes. */
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buf[1]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 36, NULL, GL_DYNAMIC_READ);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buf[1]);
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 3);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);
	{
		const GLfloat *f = (const GLfloat *) glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 36, GL_MAP_READ_BIT);
		check("glMapBufferRange of the feedback buffer", f != NULL);
		if (f) {
			const GLint *i = (const GLint *) f;
			check("captured: point 1 doubled", f[0] == 0.5f && f[1] == -1.0f);
			check("captured: point 3 doubled", f[6] == -2.0f && f[7] == 0.25f);
			check("captured: gl_VertexID + 100", i[2] == 100 && i[5] == 101 && i[8] == 102);
		}
		glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
	}

	/* An occlusion query: a full-screen quad passes, a culled one does not. */
	glGenQueries(1, &query);
	{
		static const GLfloat quad[8] = {-1, -1, 1, -1, 1, 1, -1, 1};
		GLuint passed = 77, available = 0;
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, quad);
		glViewport(0, 0, WIDTH, HEIGHT);
		glBeginQuery(GL_ANY_SAMPLES_PASSED, query);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glEndQuery(GL_ANY_SAMPLES_PASSED);
		glFinish();
		glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
		glGetQueryObjectuiv(query, GL_QUERY_RESULT, &passed);
		check("GL_ANY_SAMPLES_PASSED of a drawn quad", available && passed == GL_TRUE);
		/* A name is a query object only once glBeginQuery used it. */
		check("glIsQuery", glIsQuery(query));
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT_AND_BACK);
		glBeginQuery(GL_ANY_SAMPLES_PASSED, query);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glEndQuery(GL_ANY_SAMPLES_PASSED);
		glGetQueryObjectuiv(query, GL_QUERY_RESULT, &passed);
		check("GL_ANY_SAMPLES_PASSED of a culled quad", passed == GL_FALSE);
		glDisable(GL_CULL_FACE);
	}
	{
		GLint formats = -1;
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
		checkInt("GL_NUM_PROGRAM_BINARY_FORMATS", formats, 0);
	}

	checkInt("glGetError", (GLint) glGetError(), GL_NO_ERROR);
	glDeleteQueries(1, &query);
	glDeleteBuffers(3, buf);
	aglDestroyContext(ctx);
	CloseWindow(win);
	OGLES3Close();
	printf("e02: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 10 : 0;
}
