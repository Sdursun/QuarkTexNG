/*
 * o04_ogles2_surfaces: the context surfaces and tags o01-o03 do not use.
 *
 * - OGLES2_CCT_BITMAP: frames go into the program's bitmap, there when
 *   aglSwapBuffers returns; aglSetBitmap renders the open frame into the old
 *   bitmap and gives the default framebuffer the new one's size.
 * - aglSetParams2 with OGLES2_CCT_WINDOW after the window was resized: the
 *   default framebuffer takes the new inner size (GET_WIDTH/GET_HEIGHT).
 * - OGLES2_CCT_SHARE_WITH: a second context sees the first one's texture,
 *   shaders, program and buffer, draws with them, and still does after the
 *   first context is gone.
 * - The dispatch table's calls go to the context they name, from this task
 *   and from a second one, whatever context is current.
 */
#include <proto/ogles2.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <proto/Picasso96.h>
#include <graphics/rastport.h>
#include <dos/dostags.h>
#include <stdio.h>
#include <string.h>
#include "window.h"

struct Library *P96Base;
static int failures;

static void check(const char *what, int ok) {
	if (!ok) ++failures;
	printf("o04: %s %s\n", ok ? "PASS" : "FAIL", what);
}

static ULONG pixel(struct BitMap *bm, int x, int y) {
	struct RastPort rp;
	InitRastPort(&rp);
	rp.BitMap = bm;
	return p96ReadPixel(&rp, (UWORD) x, (UWORD) y) & 0xFFFFFF;
}

static void expectPixel(const char *what, struct BitMap *bm, int x, int y, ULONG rgb) {
	ULONG got = pixel(bm, x, y);
	int ok = got == rgb;
	if (!ok) ++failures;
	printf("o04: %s %s: %06lx, expected %06lx\n", ok ? "PASS" : "FAIL", what, (unsigned long) got, (unsigned long) rgb);
}

static struct BitMap *bitmap(int width, int height) {
	return p96AllocBitMap((ULONG) width, (ULONG) height, 32, BMF_CLEAR, NULL, RGBFB_A8R8G8B8);
}

static void clear(float r, float g, float b) {
	glClearColor(r, g, b, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

/* OGLES2_CCT_BITMAP and aglSetBitmap. */
static void bitmaps(void) {
	struct BitMap *a = bitmap(64, 48), *b = bitmap(96, 80);
	ULONG err = 0, width = 0, height = 0;
	GLint viewport[4];
	void *ctx;
	if (!a || !b) {
		check("bitmaps allocated", 0);
		goto out;
	}
	ctx = aglCreateContextTags2(&err, OGLES2_CCT_BITMAP, (ULONG) a, OGLES2_CCT_GET_WIDTH, (ULONG) &width,
		OGLES2_CCT_GET_HEIGHT, (ULONG) &height, TAG_DONE);
	check("a context on a bitmap", ctx != NULL);
	if (!ctx) {
		printf("o04: error %lu\n", (unsigned long) err);
		goto out;
	}
	aglMakeCurrent(ctx);
	check("its size is the bitmap's", width == 64 && height == 48);
	glGetIntegerv(GL_VIEWPORT, viewport);
	check("the viewport starts at the bitmap's size", viewport[2] == 64 && viewport[3] == 48);
	clear(1, 0, 0);
	aglSwapBuffers();
	expectPixel("the frame is in the bitmap when aglSwapBuffers returns", a, 0, 0, 0xFF0000);
	expectPixel("... all of it", a, 63, 47, 0xFF0000);

	/* The open frame goes into the old bitmap. */
	clear(0, 1, 0);
	aglSetBitmap(b);
	expectPixel("aglSetBitmap renders the open frame into the old bitmap", a, 32, 24, 0x00FF00);
	width = height = 0;
	aglSetParamsTags2(OGLES2_CCT_GET_WIDTH, (ULONG) &width, OGLES2_CCT_GET_HEIGHT, (ULONG) &height, TAG_DONE);
	check("the context takes the new bitmap's size", width == 96 && height == 80);
	glGetIntegerv(GL_VIEWPORT, viewport);
	check("the viewport stays as it was", viewport[2] == 64 && viewport[3] == 48);
	glViewport(0, 0, 96, 80);
	clear(0, 0, 1);
	aglSwapBuffers();
	expectPixel("the next frame is in the new bitmap", b, 0, 0, 0x0000FF);
	expectPixel("... all of it, at the new size", b, 95, 79, 0x0000FF);
	expectPixel("the old bitmap keeps its frame", a, 32, 24, 0x00FF00);
	aglDestroyContext(ctx);
out:
	if (a) p96FreeBitMap(a);
	if (b) p96FreeBitMap(b);
}

/* aglSetParams2 with the window after IDCMP_NEWSIZE. */
static void resized(void) {
	struct Window *win;
	struct IntuiMessage *msg;
	ULONG err = 0, width = 0, height = 0;
	int innerW, innerH, i, sized = 0;
	void *ctx;
	win = OpenWindowTags(NULL, WA_Left, 20, WA_Top, 20, WA_InnerWidth, 200, WA_InnerHeight, 150, WA_Title,
		(ULONG) "o04: resized", WA_SizeGadget, TRUE, WA_DragBar, TRUE, WA_MaxWidth, ~0, WA_MaxHeight, ~0,
		WA_IDCMP, IDCMP_NEWSIZE, WA_Activate, TRUE, TAG_DONE);
	if (!win) {
		check("a window", 0);
		return;
	}
	ctx = aglCreateContextTags2(&err, OGLES2_CCT_WINDOW, (ULONG) win, TAG_DONE);
	check("a context on the window", ctx != NULL);
	if (!ctx) {
		CloseWindow(win);
		return;
	}
	aglMakeCurrent(ctx);
	clear(1, 0, 0);
	aglSwapBuffers();
	ChangeWindowBox(win, win->LeftEdge, win->TopEdge, win->Width + 100, win->Height + 60);
	for (i = 0; i < 100 && !sized; ++i) {
		while ((msg = (struct IntuiMessage *) GetMsg(win->UserPort))) {
			if (msg->Class == IDCMP_NEWSIZE) sized = 1;
			ReplyMsg((struct Message *) msg);
		}
		if (!sized) Delay(2);
	}
	check("the window was resized", sized);
	innerW = win->Width - win->BorderLeft - win->BorderRight;
	innerH = win->Height - win->BorderTop - win->BorderBottom;
	aglSetParamsTags2(OGLES2_CCT_WINDOW, (ULONG) win, OGLES2_CCT_GET_WIDTH, (ULONG) &width, OGLES2_CCT_GET_HEIGHT,
		(ULONG) &height, TAG_DONE);
	printf("o04: inner size %dx%d, context %lux%lu\n", innerW, innerH, (unsigned long) width, (unsigned long) height);
	check("the context takes the window's new inner size", (int) width == innerW && (int) height == innerH);
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	for (i = 0; i < 3; ++i) {
		clear(1, 1, 0);
		aglSwapBuffers();
	}
	glFinish();
	Delay(25);
	expectPixel("the frame fills the larger window", win->RPort->BitMap, win->LeftEdge + win->BorderLeft + innerW - 3,
		win->TopEdge + win->BorderTop + innerH - 3, 0xFFFF00);
	expectPixel("... from its top left corner", win->RPort->BitMap, win->LeftEdge + win->BorderLeft + 2,
		win->TopEdge + win->BorderTop + 2, 0xFFFF00);
	aglDestroyContext(ctx);
	CloseWindow(win);
}

static const char *vertexSource = "attribute vec2 pos;\nvoid main() { gl_Position = vec4(pos, 0.0, 1.0); }\n";
static const char *fragmentSource =
	"precision mediump float;\nuniform sampler2D tex;\nvoid main() { gl_FragColor = texture2D(tex, vec2(0.5)); }\n";

static GLuint shader(GLenum type, const char *source) {
	GLuint s = glCreateShader(type);
	GLint ok = 0;
	glShaderSource(s, 1, &source, NULL);
	glCompileShader(s);
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	return ok ? s : 0;
}

/* Draws a triangle over the whole surface with the shared objects. */
static void draw(GLuint program, GLuint texture, GLuint buffer) {
	GLint pos = glGetAttribLocation(program, "pos");
	clear(0, 0, 0);
	glUseProgram(program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glVertexAttribPointer((GLuint) pos, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray((GLuint) pos);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	aglSwapBuffers();
}

/* OGLES2_CCT_SHARE_WITH. */
static void shared(void) {
	static const GLfloat triangle[] = {-1, -1, 3, -1, -1, 3};
	static const GLubyte magenta[] = {255, 0, 255, 255};
	static ULONG unattached[64]; /* what a context without a host context looks like: zeros */
	struct BitMap *a = bitmap(32, 32), *b = bitmap(32, 32);
	void *first = NULL, *second = NULL;
	GLuint vs, fs, program, texture, buffer;
	GLint linked = 0;
	ULONG err = 0;
	char text[256];
	GLsizei length = 0;
	if (!a || !b) {
		check("bitmaps allocated", 0);
		goto out;
	}
	first = aglCreateContextTags2(&err, OGLES2_CCT_BITMAP, (ULONG) a, TAG_DONE);
	check("the first context", first != NULL);
	if (!first) goto out;
	aglMakeCurrent(first);
	vs = shader(GL_VERTEX_SHADER, vertexSource);
	fs = shader(GL_FRAGMENT_SHADER, fragmentSource);
	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	check("the first context's program links", vs && fs && linked);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, magenta);
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
	draw(program, texture, buffer);
	expectPixel("the first context draws with its objects", a, 16, 16, 0xFF00FF);

	second = aglCreateContextTags2(&err, OGLES2_CCT_BITMAP, (ULONG) b, OGLES2_CCT_SHARE_WITH, (ULONG) first, TAG_DONE);
	check("a second context sharing the first one's objects", second != NULL);
	if (!second) goto out;
	aglMakeCurrent(second);
	check("it has the texture", glIsTexture(texture));
	check("... the program", glIsProgram(program));
	check("... the shaders", glIsShader(vs) && glIsShader(fs));
	check("... the buffer", glIsBuffer(buffer));
	glGetShaderSource(vs, sizeof(text), &length, text);
	check("... and the vertex shader's source", length == (GLsizei) strlen(vertexSource) && !strcmp(text, vertexSource));
	draw(program, texture, buffer);
	expectPixel("the second context draws with the shared objects", b, 16, 16, 0xFF00FF);

	aglDestroyContext(first);
	first = NULL;
	check("the objects stay after the first context is gone", glIsTexture(texture) && glIsProgram(program));
	glGetShaderSource(vs, sizeof(text), &length, text);
	check("... the source too", length == (GLsizei) strlen(vertexSource));
	draw(program, texture, buffer);
	expectPixel("the second context still draws with them", b, 16, 16, 0xFF00FF);
	glUseProgram(0); /* a program in use is only marked for deletion */
	glDeleteProgram(program);
	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteTextures(1, &texture);
	glDeleteBuffers(1, &buffer);
	check("they can be deleted there", !glIsTexture(texture) && !glIsProgram(program));

	err = 0;
	check("SHARE_WITH a context without a host context is refused",
		!aglCreateContextTags2(&err, OGLES2_CCT_BITMAP, (ULONG) a, OGLES2_CCT_SHARE_WITH, (ULONG) unattached, TAG_DONE) &&
		err == OGLES2_CCE_BAD_TAGS);
out:
	if (second) aglDestroyContext(second);
	if (first) aglDestroyContext(first);
	if (a) p96FreeBitMap(a);
	if (b) p96FreeBitMap(b);
}

/* The dispatch table's entries draw into the context they are given, not
 * the task's current one: from this task without aglMakeCurrent, and from
 * a second task (the SDK's way of drawing from one). */
static struct OGLES2Context *workerContext;
static struct Task *mainTask;

static void worker(void) {
	OGLES2Dispatch->GLClearColor(workerContext, 1, 1, 0, 1);
	OGLES2Dispatch->GLClear(workerContext, GL_COLOR_BUFFER_BIT);
	OGLES2Dispatch->aglSwapBuffers(workerContext);
	Signal(mainTask, SIGBREAKF_CTRL_F);
}

static void dispatched(void) {
	struct BitMap *a = bitmap(32, 32), *b = bitmap(32, 32);
	struct OGLES2Context *first = NULL, *second = NULL;
	ULONG err = 0;
	if (!a || !b) {
		check("bitmaps allocated", 0);
		goto out;
	}
	first = aglCreateContextTags2(&err, OGLES2_CCT_BITMAP, (ULONG) a, TAG_DONE);
	second = aglCreateContextTags2(&err, OGLES2_CCT_BITMAP, (ULONG) b, TAG_DONE);
	check("two contexts", first && second);
	if (!first || !second) goto out;
	aglMakeCurrent(first);
	clear(1, 0, 0);
	aglSwapBuffers();
	OGLES2Dispatch->GLClearColor(second, 0, 1, 1, 1);
	OGLES2Dispatch->GLClear(second, GL_COLOR_BUFFER_BIT);
	OGLES2Dispatch->aglSwapBuffers(second);
	expectPixel("the table's calls draw into the context given", b, 16, 16, 0x00FFFF);
	expectPixel("... not into the current one", a, 16, 16, 0xFF0000);
	clear(0, 0, 1);
	aglSwapBuffers();
	expectPixel("the current context draws again after them", a, 16, 16, 0x0000FF);

	workerContext = second;
	mainTask = FindTask(NULL);
	SetSignal(0, SIGBREAKF_CTRL_F);
	if (CreateNewProcTags(NP_Entry, (ULONG) worker, NP_Name, (ULONG) "o04 worker", NP_StackSize, 65536, TAG_DONE)) {
		Wait(SIGBREAKF_CTRL_F);
		Delay(5); /* until the worker has returned */
		expectPixel("a second task draws into a context this one made", b, 16, 16, 0xFFFF00);
		expectPixel("... and not into this task's current one", a, 16, 16, 0x0000FF);
	}
	else check("a second task", 0);
	clear(0, 1, 0);
	aglSwapBuffers();
	expectPixel("this task's current context still draws", a, 16, 16, 0x00FF00);
out:
	if (second) aglDestroyContext(second);
	if (first) aglDestroyContext(first);
	if (a) p96FreeBitMap(a);
	if (b) p96FreeBitMap(b);
}

int main(void) {
	write_label("o04_ogles2_surfaces");
	P96Base = OpenLibrary((STRPTR) "Picasso96API.library", 2);
	if (!P96Base || !OGLES2Open()) {
		printf("o04: FAIL no Picasso96API.library or ogles2.library\n");
		return 20;
	}
	bitmaps();
	resized();
	shared();
	dispatched();
	OGLES2Close();
	CloseLibrary(P96Base);
	printf("o04: %s, %d failures\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 10 : 0;
}
