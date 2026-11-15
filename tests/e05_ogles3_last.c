/*
 * e05_ogles3_last: the last entries of ogles3.library's table
 * (docs/phase10-ogles3.md, "The last entries"): ETC1/ETC2/EAC compressed
 * textures in 2D and 3D, program binaries refused, GL_KHR_debug's messages
 * to the application's callback and its debug groups,
 * glFramebufferTexture3DOES, glMultiDraw*EXT and the base vertex draws. And
 * framebuffer 0 bound for drawing at the end, for the swap.
 *
 * Built only when the ogles3 SDK is there (tests/Makefile).
 */
#include <proto/ogles3.h>
#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <stdio.h>
#include <string.h>
#include "window.h"

static int failures;

static void check(const char *what, int ok) {
	if (!ok) ++failures;
	printf("e05: %s %s\n", ok ? "PASS" : "FAIL", what);
}

static void checkInt(const char *what, GLint value, GLint expected) {
	if (value != expected) ++failures;
	printf("e05: %s %s: %ld, expected %ld\n", value == expected ? "PASS" : "FAIL", what, (long) value, (long) expected);
}

static void expectAt(const char *what, int x, int y, int r, int g, int b) {
	GLubyte p[4] = {0, 0, 0, 0};
	int ok;
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, p);
	ok = p[0] >= r - 6 && p[0] <= r + 6 && p[1] >= g - 6 && p[1] <= g + 6 && p[2] >= b - 6 && p[2] <= b + 6;
	if (!ok) ++failures;
	printf("e05: %s %s: (%d, %d, %d), expected (%d, %d, %d)\n", ok ? "PASS" : "FAIL", what, p[0], p[1], p[2], r, g, b);
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
		printf("e05: shader log: %s\n", log);
	}
	return s;
}

static GLuint program(const char *vertex, const char *fragment) {
	GLuint p = glCreateProgram();
	GLint ok = 0;
	glAttachShader(p, shader(GL_VERTEX_SHADER, vertex));
	glAttachShader(p, shader(GL_FRAGMENT_SHADER, fragment));
	glLinkProgram(p);
	glGetProgramiv(p, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetProgramInfoLog(p, sizeof(log), NULL, log);
		printf("e05: program log: %s\n", log);
	}
	return p;
}

static const char *quadVertex =
	"#version 300 es\n"
	"layout(location = 0) in vec2 pos;\n"
	"out vec2 uv;\n"
	"void main() { uv = pos * 0.5 + 0.5; gl_Position = vec4(pos, 0.0, 1.0); }\n";
static const char *tex2DFragment =
	"#version 300 es\n"
	"precision mediump float;\n"
	"uniform sampler2D t;\n"
	"in vec2 uv;\n"
	"layout(location = 0) out vec4 o;\n"
	"void main() { o = texture(t, uv); }\n";
static const char *arrayFragment =
	"#version 300 es\n"
	"precision mediump float;\n"
	"uniform mediump sampler2DArray t;\n"
	"in vec2 uv;\n"
	"layout(location = 0) out vec4 o;\n"
	"void main() { o = texture(t, vec3(uv, 0.0)); }\n";
/* Position and a per-vertex colour, for the multi and base vertex draws. */
static const char *colourVertex =
	"#version 300 es\n"
	"layout(location = 0) in vec2 pos;\n"
	"layout(location = 1) in vec4 col;\n"
	"out vec4 c;\n"
	"void main() { c = col; gl_Position = vec4(pos, 0.0, 1.0); }\n";
static const char *colourFragment =
	"#version 300 es\n"
	"precision mediump float;\n"
	"in vec4 c;\n"
	"layout(location = 0) out vec4 o;\n"
	"void main() { o = c; }\n";

/* One 4x4 ETC1/ETC2 block of one colour: individual mode, 4-bit colours
 * (r, g, b of 0 or 15), codeword table 0, every pixel index 0 (+2). */
static void etcBlock(GLubyte *b, int r, int g, int b_) {
	b[0] = (GLubyte) (r ? 0xFF : 0);
	b[1] = (GLubyte) (g ? 0xFF : 0);
	b[2] = (GLubyte) (b_ ? 0xFF : 0);
	b[3] = 0;
	b[4] = b[5] = b[6] = b[7] = 0;
}

static int messages, pushes, pops;
static char lastMessage[64];
static const void *lastUser;

static void GL_APIENTRY onMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
		const void *userParam) {
	++messages;
	if (type == GL_DEBUG_TYPE_PUSH_GROUP_KHR) ++pushes;
	if (type == GL_DEBUG_TYPE_POP_GROUP_KHR) ++pops;
	if (type == GL_DEBUG_TYPE_MARKER_KHR && length < (GLsizei) sizeof(lastMessage)) {
		memcpy(lastMessage, message, length);
		lastMessage[length] = 0;
	}
	lastUser = userParam;
}

/* A quad's two triangles at x0..x1, the whole height, in one colour. */
static void quad(GLfloat *v, GLubyte *c, GLfloat x0, GLfloat x1, int r, int g, int b) {
	static const int corner[6][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}};
	int i;
	for (i = 0; i < 6; ++i) {
		v[2 * i] = corner[i][0] ? x1 : x0;
		v[2 * i + 1] = corner[i][1] ? 1.0f : -1.0f;
		c[4 * i] = (GLubyte) r;
		c[4 * i + 1] = (GLubyte) g;
		c[4 * i + 2] = (GLubyte) b;
		c[4 * i + 3] = 255;
	}
}

int main(void) {
	struct Window *win;
	void *ctx;
	ULONG err = 0;
	GLuint buf[4], vao, tex[3], fbo, p2d, pArray, pColour;
	static const GLfloat whole[8] = {-1, -1, 1, -1, -1, 1, 1, 1};
	GLubyte block[16];
	GLfloat v[4 * 12];
	GLubyte c[4 * 24];
	static const GLushort quadIndices[6] = {0, 1, 2, 3, 4, 5};

	write_label("e05_ogles3_last");
	if (!OGLES3Open()) {
		printf("e05: FAIL no ogles3.library\n");
		return 20;
	}
	if (!(win = open_window("e05: ogles3 last entries"))) {
		printf("e05: FAIL no window\n");
		OGLES3Close();
		return 20;
	}
	ctx = aglCreateContextTags2(&err, OGLES3_CCT_WINDOW, (ULONG) win, OGLES3_CCT_PISTORM_ES_VERSION, 31, TAG_DONE);
	if (!ctx) {
		printf("e05: FAIL aglCreateContextTags2, error %lu\n", (unsigned long) err);
		CloseWindow(win);
		OGLES3Close();
		return 20;
	}
	aglMakeCurrent(ctx);
	glViewport(0, 0, WIDTH, HEIGHT);

	p2d = program(quadVertex, tex2DFragment);
	pArray = program(quadVertex, arrayFragment);
	pColour = program(colourVertex, colourFragment);
	glGenBuffers(4, buf);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(whole), whole, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glGenTextures(3, tex);

	/* --- Compressed textures -------------------------------------------- */
	{
		GLint n = 0, formats[32], found = 0, i;
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &n);
		check("GL_NUM_COMPRESSED_TEXTURE_FORMATS has ES 3.0's ten and ETC1", n == 11);
		if (n > 0 && n <= 32) {
			glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats);
			for (i = 0; i < n; ++i) if (formats[i] == GL_COMPRESSED_RGBA8_ETC2_EAC || formats[i] == GL_ETC1_RGB8_OES) ++found;
		}
		check("GL_COMPRESSED_TEXTURE_FORMATS lists RGBA8_ETC2_EAC and ETC1", found == 2);
		check("GL_EXTENSIONS has GL_OES_compressed_ETC1_RGB8_texture",
			strstr((const char *) glGetString(GL_EXTENSIONS), "GL_OES_compressed_ETC1_RGB8_texture") != NULL);
	}
	glUseProgram(p2d);
	glUniform1i(glGetUniformLocation(p2d, "t"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	etcBlock(block, 1, 0, 0);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, 4, 4, 0, 8, block);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	expectAt("an ETC2 RGB8 texture (red block)", WIDTH / 2, HEIGHT / 2, 255, 2, 2);
	/* ETC1 is the same block format. */
	etcBlock(block, 0, 0, 1);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, 4, 4, 0, 8, block);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	expectAt("an ETC1 texture (blue block)", WIDTH / 2, HEIGHT / 2, 2, 2, 255);
	/* glCompressedTexSubImage2D: an 8x4 texture, its right block green. */
	{
		GLubyte two[16];
		etcBlock(two, 1, 0, 0);
		etcBlock(two + 8, 1, 0, 0);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, 8, 4, 0, 16, two);
		etcBlock(block, 0, 1, 0);
		glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 4, 0, 4, 4, GL_COMPRESSED_RGB8_ETC2, 8, block);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		expectAt("glCompressedTexSubImage2D: left block still red", WIDTH / 4, HEIGHT / 2, 255, 2, 2);
		expectAt("glCompressedTexSubImage2D: right block green", 3 * WIDTH / 4, HEIGHT / 2, 2, 255, 2);
	}
	/* RGBA8_ETC2_EAC (an EAC alpha block of 255, then the colour) in a 2D
	 * array texture through glCompressedTexImage3D, and a sub-image. */
	glUseProgram(pArray);
	glUniform1i(glGetUniformLocation(pArray, "t"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex[1]);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	memset(block, 0, sizeof(block));
	block[0] = 0xFF;
	etcBlock(block + 8, 0, 1, 1);
	glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_COMPRESSED_RGBA8_ETC2_EAC, 4, 4, 1, 0, 16, block);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	expectAt("glCompressedTexImage3D: RGBA8_ETC2_EAC (cyan)", WIDTH / 2, HEIGHT / 2, 2, 255, 255);
	etcBlock(block + 8, 1, 1, 0);
	glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 4, 4, 1, GL_COMPRESSED_RGBA8_ETC2_EAC, 16, block);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	expectAt("glCompressedTexSubImage3D (yellow)", WIDTH / 2, HEIGHT / 2, 255, 255, 2);
	checkInt("glGetError after the compressed textures", (GLint) glGetError(), GL_NO_ERROR);

	/* --- Program binaries: none ----------------------------------------- */
	{
		GLint n = -1, length = -1;
		GLsizei got = -1;
		GLenum format = 0;
		char bin[16];
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &n);
		checkInt("GL_NUM_PROGRAM_BINARY_FORMATS", n, 0);
		glGetProgramiv(p2d, GL_PROGRAM_BINARY_LENGTH, &length);
		checkInt("GL_PROGRAM_BINARY_LENGTH", length, 0);
		glGetProgramBinary(p2d, sizeof(bin), &got, &format, bin);
		checkInt("glGetProgramBinary gives nothing", got, 0);
		checkInt("... and GL_INVALID_OPERATION", (GLint) glGetError(), GL_INVALID_OPERATION);
		glProgramBinary(p2d, 0x1234, bin, sizeof(bin));
		checkInt("glProgramBinary of any format: GL_INVALID_ENUM", (GLint) glGetError(), GL_INVALID_ENUM);
		checkInt("glGetError once more: none", (GLint) glGetError(), GL_NO_ERROR);
	}

	/* --- GL_KHR_debug ----------------------------------------------------- */
	{
		void *got = NULL;
		GLint depth = 0;
		char label[8] = "x";
		GLsizei length = -1;
		static int user;
		glEnable(GL_DEBUG_OUTPUT_KHR);
		glDebugMessageCallbackKHR(onMessage, &user);
		glGetPointervKHR(GL_DEBUG_CALLBACK_FUNCTION_KHR, &got);
		check("GL_DEBUG_CALLBACK_FUNCTION is the callback", got == (void *) onMessage);
		glDebugMessageInsertKHR(GL_DEBUG_SOURCE_APPLICATION_KHR, GL_DEBUG_TYPE_MARKER_KHR, 1, GL_DEBUG_SEVERITY_NOTIFICATION_KHR, -1,
			"hello");
		check("glDebugMessageInsertKHR reaches the callback", messages == 1 && !strcmp(lastMessage, "hello") && lastUser == &user);
		glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION_KHR, 2, -1, "group");
		glGetIntegerv(GL_DEBUG_GROUP_STACK_DEPTH_KHR, &depth);
		checkInt("GL_DEBUG_GROUP_STACK_DEPTH in a group", depth, 2);
		glPopDebugGroupKHR();
		check("push and pop gave messages", pushes == 1 && pops == 1);
		glPopDebugGroupKHR();
		checkInt("popping the default group: GL_STACK_UNDERFLOW", (GLint) glGetError(), GL_STACK_UNDERFLOW_KHR);
		glObjectLabelKHR(GL_TEXTURE, tex[0], -1, "label");
		glGetObjectLabelKHR(GL_TEXTURE, tex[0], sizeof(label), &length, label);
		check("glGetObjectLabelKHR answers (labels are not kept)", length == 0 && label[0] == 0);
		checkInt("glGetDebugMessageLogKHR: empty", (GLint) glGetDebugMessageLogKHR(1, 0, NULL, NULL, NULL, NULL, NULL, NULL), 0);
		glDebugMessageCallbackKHR(NULL, NULL);
	}

	/* --- glFramebufferTexture3DOES: layer 1 of a 3D texture ---------------- */
	{
		static const GLubyte layers[2][4] = {{255, 0, 0, 255}, {0, 0, 255, 255}};
		GLubyte p[4] = {0, 0, 0, 0};
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, tex[2]);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 1, 1, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, layers);
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture3DOES(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tex[2], 0, 1);
		checkInt("glFramebufferTexture3DOES: complete", (GLint) glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
		glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, p);
		check("glFramebufferTexture3DOES: reads layer 1 (blue)", p[0] == 0 && p[2] == 255);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
	}

	/* --- glMultiDraw*EXT and the base vertex draws -------------------------
	 * Four quads in a row: red, green, blue, white. */
	quad(v, c, -1.0f, -0.5f, 255, 0, 0);
	quad(v + 12, c + 24, -0.5f, 0.0f, 0, 255, 0);
	quad(v + 24, c + 48, 0.0f, 0.5f, 0, 0, 255);
	quad(v + 36, c + 72, 0.5f, 1.0f, 255, 255, 255);
	glUseProgram(pColour);
	glBindBuffer(GL_ARRAY_BUFFER, buf[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, buf[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(c), c, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
	{
		static const GLint firsts[2] = {0, 12};
		static const GLsizei counts[2] = {6, 6};
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glMultiDrawArraysEXT(GL_TRIANGLES, firsts, counts, 2);
		expectAt("glMultiDrawArraysEXT: first quad", WIDTH / 8, HEIGHT / 2, 255, 0, 0);
		expectAt("glMultiDrawArraysEXT: third quad", 5 * WIDTH / 8, HEIGHT / 2, 0, 0, 255);
		expectAt("glMultiDrawArraysEXT: second quad not drawn", 3 * WIDTH / 8, HEIGHT / 2, 0, 0, 0);
	}
	{
		/* The same six indices from the element buffer, twice. */
		static const void *const offsets[2] = {(const void *) 0, (const void *) 0};
		static const GLsizei counts[2] = {6, 6};
		static const GLint bases[2] = {6, 18};
		glClear(GL_COLOR_BUFFER_BIT);
		glMultiDrawElementsEXT(GL_TRIANGLES, counts, GL_UNSIGNED_SHORT, offsets, 1);
		expectAt("glMultiDrawElementsEXT: first quad", WIDTH / 8, HEIGHT / 2, 255, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glMultiDrawElementsBaseVertexEXT(GL_TRIANGLES, counts, GL_UNSIGNED_SHORT, offsets, 2, bases);
		expectAt("glMultiDrawElementsBaseVertexEXT: base 6 (green)", 3 * WIDTH / 8, HEIGHT / 2, 0, 255, 0);
		expectAt("glMultiDrawElementsBaseVertexEXT: base 18 (white)", 7 * WIDTH / 8, HEIGHT / 2, 255, 255, 255);
		expectAt("glMultiDrawElementsBaseVertexEXT: base 0 not drawn", WIDTH / 8, HEIGHT / 2, 0, 0, 0);
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawElementsBaseVertexEXT(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, 12);
	expectAt("glDrawElementsBaseVertexEXT (blue)", 5 * WIDTH / 8, HEIGHT / 2, 0, 0, 255);
	glDrawRangeElementsBaseVertexOES(GL_TRIANGLES, 0, 5, 6, GL_UNSIGNED_SHORT, 0, 6);
	expectAt("glDrawRangeElementsBaseVertexOES (green)", 3 * WIDTH / 8, HEIGHT / 2, 0, 255, 0);
	glDrawElementsInstancedBaseVertexOES(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, 2, 18);
	expectAt("glDrawElementsInstancedBaseVertexOES (white)", 7 * WIDTH / 8, HEIGHT / 2, 255, 255, 255);
	glPrimitiveBoundingBoxOES(-1, -1, -1, 1, 1, 1, 1, 1);
	checkInt("glGetError at the end", (GLint) glGetError(), GL_NO_ERROR);
	aglSwapBuffers();

	glDeleteTextures(3, tex);
	glDeleteBuffers(4, buf);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(p2d);
	glDeleteProgram(pArray);
	glDeleteProgram(pColour);
	aglDestroyContext(ctx);
	CloseWindow(win);
	OGLES3Close();
	printf("e05: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 10 : 0;
}
