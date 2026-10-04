/*
 * agl.library texturing: glGenTextures (the host writes the name back into
 * Amiga memory), glTexImage2D with RGBA bytes, filtering, and a textured quad
 * modulated by vertex colours.
 */
#include "agl_common.h"

const char test_name[] = "a03_agl_texture";

#define SIZE 16

static GLuint texture;
static GLubyte image[SIZE][SIZE][4];

int test_setup(void) {
	int x, y;
	for (y = 0; y < SIZE; ++y) {
		for (x = 0; x < SIZE; ++x) {
			int on = ((x / 4) + (y / 4)) & 1;
			image[y][x][0] = on ? 255 : x * 16;
			image[y][x][1] = on ? 255 : y * 16;
			image[y][x][2] = on ? 64 : 200;
			image[y][x][3] = 255;
		}
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	texture = 0;
	glGenTextures(1, &texture);
	if (texture == 0) fail("glGenTextures returned no name", 0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	return 0;
}

void test_draw(int frame) {
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-0.9f, -0.9f);
	glColor3f(1.0f, 0.5f, 0.5f);
	glTexCoord2f(2.0f, 0.0f);
	glVertex2f(0.9f, -0.9f);
	glColor3f(0.5f, 1.0f, 0.5f);
	glTexCoord2f(2.0f, 1.0f);
	glVertex2f(0.9f, 0.9f);
	glColor3f(0.5f, 0.5f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-0.9f, 0.9f);
	glEnd();
}

void test_cleanup(void) {
	glDisable(GL_TEXTURE_2D);
	if (texture) glDeleteTextures(1, &texture);
}
