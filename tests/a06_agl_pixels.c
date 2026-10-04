/*
 * agl.library pixel data wider than one byte, which has to reach the host in
 * its byte order:
 * - glDrawPixels of a 7 x 5 GL_RGB GL_UNSIGNED_SHORT image, zoomed 8 times;
 *   with the default unpack alignment of 4 its rows are padded to 44 bytes;
 * - a 4 x 4 GL_RGBA GL_UNSIGNED_SHORT texture with a 2 x 2 GL_RGB GL_FLOAT
 *   sub-image (glTexSubImage2D);
 * - glCallLists with GL_UNSIGNED_SHORT list names drawing three squares;
 * - glReadPixels of one pixel as GL_UNSIGNED_SHORT and as GL_FLOAT, logged
 *   and drawn as bars.
 * (QuarkTex 0.53 swapped this data in place, but stepped one byte at a time
 * through it, swapped one element per pixel instead of one per component,
 * and ignored row padding.)
 */
#include <stdio.h>
#include "agl_common.h"

const char test_name[] = "a06_agl_pixels";

#define IMAGE_W 7
#define IMAGE_H 5
#define ROW_SHORTS 22 /* 7 * 3 = 21 shorts, padded to 44 bytes */

static GLushort image[IMAGE_H * ROW_SHORTS];
static GLushort texels[4 * 4 * 4];
static GLfloat green[2 * 2 * 3];
static GLuint texture;
static GLuint lists;

int test_setup(void) {
	int x, y, i;
	for (y = 0; y < IMAGE_H; ++y) {
		for (x = 0; x < IMAGE_W; ++x) {
			image[y * ROW_SHORTS + x * 3] = (GLushort) (x * 65535 / (IMAGE_W - 1));
			image[y * ROW_SHORTS + x * 3 + 1] = (GLushort) (y * 65535 / (IMAGE_H - 1));
			image[y * ROW_SHORTS + x * 3 + 2] = 0x8000;
		}
		image[y * ROW_SHORTS + 21] = 0xFFFF; /* padding, never read */
	}
	for (i = 0; i < 16; ++i) {
		int red = ((i % 4) + (i / 4)) & 1;
		texels[i * 4] = red ? 0xFFFF : 0;
		texels[i * 4 + 1] = 0;
		texels[i * 4 + 2] = red ? 0 : 0xFFFF;
		texels[i * 4 + 3] = 0xFFFF;
	}
	for (i = 0; i < 4; ++i) {
		green[i * 3] = 0.0f;
		green[i * 3 + 1] = 1.0f;
		green[i * 3 + 2] = 0.25f;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, WIDTH, 0.0, HEIGHT, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_SHORT, texels);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, 2, 2, GL_RGB, GL_FLOAT, green);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	lists = glGenLists(3);
	for (i = 0; i < 3; ++i) {
		glNewList(lists + i, GL_COMPILE);
		glColor3f(i == 0, i == 1, i == 2);
		glBegin(GL_QUADS);
		glVertex2f(200 + i * 35, 180);
		glVertex2f(230 + i * 35, 180);
		glVertex2f(230 + i * 35, 210);
		glVertex2f(200 + i * 35, 210);
		glEnd();
		glEndList();
	}
	return 0;
}

static void bar(float y, float length, float r, float g, float b) {
	glColor3f(r, g, b);
	glBegin(GL_QUADS);
	glVertex2f(20, y);
	glVertex2f(20 + length, y);
	glVertex2f(20 + length, y + 10);
	glVertex2f(20, y + 10);
	glEnd();
}

void test_draw(int frame) {
	GLushort names[3];
	GLushort pixel16[3] = {0, 0, 0};
	GLfloat pixelf[3] = {0, 0, 0};

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glRasterPos2f(20, 180);
	glPixelZoom(8, 8);
	glDrawPixels(IMAGE_W, IMAGE_H, GL_RGB, GL_UNSIGNED_SHORT, image);
	glPixelZoom(1, 1);

	glEnable(GL_TEXTURE_2D);
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(110, 120);
	glTexCoord2f(1, 0); glVertex2f(190, 120);
	glTexCoord2f(1, 1); glVertex2f(190, 200);
	glTexCoord2f(0, 1); glVertex2f(110, 200);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	names[0] = (GLushort) (lists + 2);
	names[1] = (GLushort) lists;
	names[2] = (GLushort) (lists + 1);
	glCallLists(3, GL_UNSIGNED_SHORT, names);

	/* Pixel (6, 2) of the image: full red, half green, half blue */
	glReadPixels(20 + 6 * 8 + 4, 180 + 2 * 8 + 4, 1, 1, GL_RGB, GL_UNSIGNED_SHORT, pixel16);
	glReadPixels(20 + 6 * 8 + 4, 180 + 2 * 8 + 4, 1, 1, GL_RGB, GL_FLOAT, pixelf);
	bar(80, pixel16[0] / 65535.0f * 280, 1, 0, 0);
	bar(65, pixel16[1] / 65535.0f * 280, 0, 1, 0);
	bar(50, pixelf[0] * 280, 1, 0.5f, 0.5f);
	bar(35, pixelf[1] * 280, 0.5f, 1, 0.5f);
	bar(20, pixelf[2] * 280, 0.5f, 0.5f, 1);

	if (frame == FRAMES - 1) {
		printf("%s: glReadPixels as shorts %u %u %u, as floats %ld %ld %ld (x1000)\n", test_name,
			pixel16[0], pixel16[1], pixel16[2],
			(long) (pixelf[0] * 1000 + 0.5f), (long) (pixelf[1] * 1000 + 0.5f), (long) (pixelf[2] * 1000 + 0.5f));
		if (names[0] != (GLushort) (lists + 2)) fail("glCallLists changed its names", names[0]);
	}
}

void test_cleanup(void) {
	glDeleteLists(lists, 3);
	glDeleteTextures(1, &texture);
}
