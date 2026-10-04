/*
 * agl.library vertex arrays and queries: glVertexPointer/glColorPointer with
 * glDrawArrays (emulated by agl.library with glIsEnabled), glGetPointerv,
 * glGetIntegerv and glGetFloatv. The query results are logged and drawn as
 * bars, so they show up in the comparison too.
 */
#include <stdio.h>
#include "agl_common.h"

const char test_name[] = "a05_agl_queries";

static GLfloat vertices[8][2];
static GLfloat colors[8][3];

int test_setup(void) {
	int i;
	for (i = 0; i < 8; ++i) {
		vertices[i][0] = -0.9f + (i / 2) * 0.6f;
		vertices[i][1] = (i & 1) ? -0.1f : -0.9f;
		colors[i][0] = (i & 1) ? 1.0f : 0.0f;
		colors[i][1] = i / 7.0f;
		colors[i][2] = (i & 1) ? 0.0f : 1.0f;
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glColorPointer(3, GL_FLOAT, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	return 0;
}

/* A horizontal bar from x = -0.9 with the given length (0..1.8). */
static void bar(float y, float length, float r, float g, float b) {
	glColor3f(r, g, b);
	glBegin(GL_QUADS);
	glVertex2f(-0.9f, y);
	glVertex2f(-0.9f + length, y);
	glVertex2f(-0.9f + length, y + 0.12f);
	glVertex2f(-0.9f, y + 0.12f);
	glEnd();
}

void test_draw(int frame) {
	GLint viewport[4] = {0, 0, 0, 0};
	GLfloat color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	GLvoid *pointer = NULL;
	ULONG enabled;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glGetIntegerv(GL_VIEWPORT, viewport);
	glColor4f(0.25f, 0.5f, 0.75f, 1.0f);
	glGetFloatv(GL_CURRENT_COLOR, color);
	glGetPointerv(GL_VERTEX_ARRAY_POINTER, &pointer);
	enabled = glIsEnabled(GL_VERTEX_ARRAY);

	/* Viewport width and height, the current colour, the pointer check */
	bar(0.05f, viewport[2] / (float) WIDTH * 1.8f, 1.0f, 1.0f, 1.0f);
	bar(0.22f, viewport[3] / (float) HEIGHT * 1.8f, 0.8f, 0.8f, 0.8f);
	bar(0.39f, color[0] * 1.8f, 1.0f, 0.3f, 0.3f);
	bar(0.56f, color[1] * 1.8f, 0.3f, 1.0f, 0.3f);
	bar(0.73f, color[2] * 1.8f, 0.3f, 0.3f, 1.0f);
	if (pointer == (GLvoid *) vertices) bar(-0.05f, 1.8f, 1.0f, 1.0f, 0.0f);

	if (frame == FRAMES - 1) {
		printf("%s: viewport %ld %ld %ld %ld\n", test_name,
			(long) viewport[0], (long) viewport[1], (long) viewport[2], (long) viewport[3]);
		printf("%s: current colour %ld %ld %ld %ld (x1000)\n", test_name,
			(long) (color[0] * 1000), (long) (color[1] * 1000), (long) (color[2] * 1000), (long) (color[3] * 1000));
		printf("%s: glGetPointerv %08lx, vertices at %08lx\n", test_name,
			(unsigned long) pointer, (unsigned long) vertices);
		if (viewport[2] != WIDTH || viewport[3] != HEIGHT) fail("glGetIntegerv(GL_VIEWPORT)", viewport[2]);
		if (color[1] != 0.5f) fail("glGetFloatv(GL_CURRENT_COLOR)", (ULONG) (color[1] * 1000));
		if (pointer != (GLvoid *) vertices) fail("glGetPointerv(GL_VERTEX_ARRAY_POINTER)", (ULONG) pointer);
		if (enabled) fail("glIsEnabled(GL_VERTEX_ARRAY) after disabling", enabled);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
}

void test_cleanup(void) {
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}
