/*
 * agl.library lighting: glLightfv and glMaterialfv take float arrays
 * (pointers, swapped in place by agl.library), glNormal3f per face.
 */
#include "agl_common.h"

const char test_name[] = "a04_agl_lighting";

int test_setup(void) {
	static const GLfloat position[4] = {1.0f, 1.0f, 2.0f, 0.0f};
	static const GLfloat diffuse[4] = {1.0f, 0.9f, 0.6f, 1.0f};
	static const GLfloat ambient[4] = {0.15f, 0.15f, 0.2f, 1.0f};
	static const GLfloat material[4] = {0.3f, 0.6f, 1.0f, 1.0f};

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -2.0, 2.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	return 0;
}

/* One quad per column, its normal turning from left to right. */
void test_draw(int frame) {
	int i;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	for (i = 0; i < 8; ++i) {
		float x = -0.95f + i * 0.24f;
		float nx = -1.0f + i * (2.0f / 7.0f);
		glNormal3f(nx, 0.3f, 1.0f);
		glBegin(GL_QUADS);
		glVertex3f(x, -0.8f, 0.0f);
		glVertex3f(x + 0.2f, -0.8f, 0.0f);
		glVertex3f(x + 0.2f, 0.8f, 0.0f);
		glVertex3f(x, 0.8f, 0.0f);
		glEnd();
	}
}

void test_cleanup(void) {
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
}
