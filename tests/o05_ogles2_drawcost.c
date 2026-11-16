/*
 * o05_ogles2_drawcost: what one small draw costs through ogles2.library, the
 * way GZDoom's 2D drawer and sprites draw (a texture bound, a quad drawn
 * from a vertex buffer with three attributes, per draw). Times FRAMES_TIMED
 * frames of DRAWS draws each, then the same frames with only the binds, and
 * prints the cost per draw. A measuring tool (QuarkTex NG's command buffer
 * and draw packet); it checks only that a quad was drawn.
 *
 *   o05_ogles2_drawcost [draws per frame] [frames] [client]
 *
 * "client": the vertices in Amiga memory instead of a buffer.
 */
#include <proto/ogles2.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <devices/timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "window.h"

struct Device *TimerBase;
static struct timerequest timerReq;

static const char *vs =
	"attribute vec4 aPos; attribute vec2 aUV; attribute vec4 aColor;\n"
	"varying vec2 vUV; varying vec4 vColor;\n"
	"void main() { gl_Position = aPos; vUV = aUV; vColor = aColor; }\n";
static const char *fs =
	"precision mediump float; varying vec2 vUV; varying vec4 vColor; uniform sampler2D tex;\n"
	"void main() { gl_FragColor = texture2D(tex, vUV) * vColor; }\n";

static double nowMs(void) {
	struct EClockVal e;
	ULONG freq = ReadEClock(&e);
	return ((double) e.ev_hi * 4294967296.0 + e.ev_lo) * 1000.0 / freq;
}

static GLuint shader(GLenum type, const char *src) {
	GLuint s = glCreateShader(type);
	glShaderSource(s, 1, &src, 0);
	glCompileShader(s);
	return s;
}

/* x y z w, u v, r g b a */
static float quad[4 * 10] = {
	-0.5f, -0.5f, 0, 1, 0, 0, 1, 1, 1, 1,
	 0.5f, -0.5f, 0, 1, 1, 0, 1, 1, 1, 1,
	 0.5f,  0.5f, 0, 1, 1, 1, 1, 1, 1, 1,
	-0.5f,  0.5f, 0, 1, 0, 1, 1, 1, 1, 1,
};

int main(int argc, char **argv) {
	struct Window *win;
	void *ctx;
	ULONG err = 0;
	int draws = argc > 1 ? atoi(argv[1]) : 2000;
	int frames = argc > 2 ? atoi(argv[2]) : 50;
	int client = argc > 3 && !strcmp(argv[3], "client");
	GLuint prog, vbo, tex[8];
	int i, f, pass;
	double ms[2];
	unsigned char pixel[4];

	write_label("o05_ogles2_drawcost");
	if (OpenDevice((STRPTR) TIMERNAME, UNIT_ECLOCK, (struct IORequest *) &timerReq, 0)) return 20;
	TimerBase = timerReq.tr_node.io_Device;
	if (!OGLES2Open()) {
		printf("o05: FAIL no ogles2.library\n");
		return 20;
	}
	if (!(win = open_window("o05: draw cost"))) {
		OGLES2Close();
		return 20;
	}
	ctx = aglCreateContextTags2(&err, OGLES2_CCT_WINDOW, (ULONG) win, OGLES2_CCT_DEPTH, 0, OGLES2_CCT_STENCIL, 0, TAG_DONE);
	if (!ctx) {
		printf("o05: FAIL aglCreateContextTags2, error %lu\n", (unsigned long) err);
		CloseWindow(win);
		OGLES2Close();
		return 20;
	}
	aglMakeCurrent(ctx);

	prog = glCreateProgram();
	glAttachShader(prog, shader(GL_VERTEX_SHADER, vs));
	glAttachShader(prog, shader(GL_FRAGMENT_SHADER, fs));
	glBindAttribLocation(prog, 0, "aPos");
	glBindAttribLocation(prog, 1, "aUV");
	glBindAttribLocation(prog, 2, "aColor");
	glLinkProgram(prog);
	glUseProgram(prog);
	glUniform1i(glGetUniformLocation(prog, "tex"), 0);

	glGenTextures(8, tex);
	for (i = 0; i < 8; ++i) {
		unsigned char texel[4 * 4 * 4];
		memset(texel, 40 + 25 * i, sizeof(texel));
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, texel);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	if (client) glBindBuffer(GL_ARRAY_BUFFER, 0);
	{
		const char *base = client ? (const char *) quad : 0;
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 40, base);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 40, base + 16);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 40, base + 24);
	}
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glClearColor(0, 0, 0, 1);

	/* pass 0: binds and draws; pass 1: binds only */
	for (pass = 0; pass < 2; ++pass) {
		double start = 0;
		for (f = -3; f < frames; ++f) {
			if (f == 0) start = nowMs();
			glClear(GL_COLOR_BUFFER_BIT);
			for (i = 0; i < draws; ++i) {
				glBindTexture(GL_TEXTURE_2D, tex[i & 7]);
				if (!pass) glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			}
			if (!pass && f == -1) glReadPixels(WIDTH / 2, HEIGHT / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
			aglSwapBuffers();
		}
		ms[pass] = (nowMs() - start) / frames;
	}
	printf("o05: %d draws a frame%s: %.2f ms a frame with draws, %.2f ms with the binds only; %.2f us a draw\n",
		draws, client ? " (vertices in Amiga memory)" : "", ms[0], ms[1], (ms[0] - ms[1]) * 1000.0 / draws);
	printf("o05: %s centre pixel %d %d %d (the last texture's grey, not black)\n",
		pixel[0] ? "PASS" : "FAIL", pixel[0], pixel[1], pixel[2]);

	aglDestroyContext(ctx);
	CloseWindow(win);
	OGLES2Close();
	CloseDevice((struct IORequest *) &timerReq);
	return pixel[0] ? 0 : 5;
}
