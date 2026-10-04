/*
 * agl.library transformations and depth: glFrustum, matrix stack,
 * glTranslatef/glRotatef, glClearDepth (double in fp0) and the depth test,
 * on three intersecting quads.
 */
#include "agl_common.h"

const char test_name[] = "a02_agl_transform";

int test_setup(void) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.4, 0.4, -0.3, 0.3, 1.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(1.0);
	return 0;
}

static void quad(float r, float g, float b) {
	glColor3f(r, g, b);
	glBegin(GL_QUADS);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glEnd();
}

void test_draw(int frame) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -4.0f);

	glPushMatrix();
	glRotatef(60.0f, 0.0f, 1.0f, 0.0f);
	quad(1.0f, 0.2f, 0.2f);
	glPopMatrix();

	glPushMatrix();
	glRotatef(-60.0f, 0.0f, 1.0f, 0.0f);
	quad(0.2f, 1.0f, 0.2f);
	glPopMatrix();

	glPushMatrix();
	glRotatef(70.0f, 1.0f, 0.0f, 0.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	quad(0.3f, 0.3f, 1.0f);
	glPopMatrix();

	glPopMatrix();
}

void test_cleanup(void) {
	glDisable(GL_DEPTH_TEST);
}
