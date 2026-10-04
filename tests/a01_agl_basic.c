/*
 * agl.library immediate mode: glOrtho (doubles in fp0-fp5), a shaded
 * triangle (glColor3f/glVertex2f, floats in fp registers) and a quad drawn
 * with glVertex3fv (pointer, swapped in place by agl.library).
 */
#include "agl_common.h"

const char test_name[] = "a01_agl_basic";

int test_setup(void) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glShadeModel(GL_SMOOTH);
	return 0;
}

void test_draw(int frame) {
	static const GLfloat quad[4][3] = {
		{0.2f, -0.9f, 0.0f}, {0.9f, -0.9f, 0.0f}, {0.9f, -0.2f, 0.0f}, {0.2f, -0.2f, 0.0f}
	};
	int i;

	glClearColor(0.1f, 0.1f, 0.25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(-0.9f, -0.9f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(0.0f, -0.9f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(-0.45f, 0.9f);
	glEnd();

	glBegin(GL_QUADS);
	for (i = 0; i < 4; ++i) {
		glColor4f(i & 1 ? 1.0f : 0.3f, i & 2 ? 1.0f : 0.3f, 0.5f, 1.0f);
		glVertex3fv(quad[i]);
	}
	glEnd();

	/* The array must be unchanged after agl.library swapped it for the host. */
	if (quad[2][0] != 0.9f || quad[2][1] != -0.2f) fail("glVertex3fv modified its argument", 0);
}

void test_cleanup(void) {
}
