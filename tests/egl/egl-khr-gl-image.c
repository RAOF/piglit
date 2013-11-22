/*
 * Copyright © 2013 Canonical Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Author: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#include "piglit-util-gl-common.h"
#include "egl-util.h"

#ifdef EGL_KHR_gl_texture_2D_image

const char *extensions[] = { "EGL_KHR_gl_texture_2D_image", "EGL_KHR_surfaceless_context", NULL };

static enum piglit_result
draw(struct egl_state *state)
{
	float aubergine[] = { 0.46, 0.13, 0.44, 1.0 };
	float orange[] = { 0.86, 0.28, 0.08, 1.0 };
	GLuint fb = 0;
	GLuint exportTex, importTex;
	EGLImageKHR texImg;
	PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC EGLImageTargetTexture2DOES;

	piglit_require_extension("GL_OES_EGL_image");
	piglit_require_extension("GL_EXT_framebuffer_object");

	eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
	EGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");

	/* Allocate render texture */
	glGenTextures(1, &exportTex);

    glBindTexture(GL_TEXTURE_2D, exportTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 100, 100, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    /* Bind to eglImageOES */
    texImg = eglCreateImageKHR(state->egl_dpy, state->ctx,
    						   EGL_GL_TEXTURE_2D_KHR,
    						   (EGLClientBuffer)exportTex,
    						   NULL);

    /* Bind EGL image to output texture */
    glGenTextures(1, &importTex);

    glBindTexture(GL_TEXTURE_2D, importTex);
    EGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)texImg);

	if (!eglMakeCurrent(state->egl_dpy,
			    EGL_NO_SURFACE, EGL_NO_SURFACE, state->ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

    glGenFramebuffersEXT(1, &fb);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, exportTex, 0);

    if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
    	fprintf(stderr, "FBO incomplete\n");
    	piglit_report_result(PIGLIT_FAIL);
    }

	/* Clear texture to orange */
	glClearColor(orange[0], orange[1], orange[2], orange[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	if (!eglMakeCurrent(state->egl_dpy,
			    state->surf, state->surf, state->ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glViewport(0, 0, state->width, state->height);
	piglit_ortho_projection(state->width, state->height, GL_FALSE);

	glClearColor(aubergine[0], aubergine[1], aubergine[2], aubergine[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, importTex);
	piglit_draw_rect_tex(20, 20, 100, 100,  0, 0, 1, 1);
	eglSwapBuffers(state->egl_dpy, state->surf);	

	if (!piglit_probe_pixel_rgba(10, 10, aubergine) ||
	    !piglit_probe_pixel_rgba(50, 10, aubergine) ||
	    !piglit_probe_pixel_rgba(10, 50, aubergine) ||
	    !piglit_probe_pixel_rgba(50, 50, orange) ||
	    !piglit_probe_pixel_rgba(110, 110, orange) ||
	    !piglit_probe_pixel_rgba(130, 130, aubergine))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

int
main(int argc, char *argv[])
{
	struct egl_test test;

	egl_init_test(&test);
	test.extensions = extensions;
	test.draw = draw;

	return egl_util_run(&test, argc, argv);
}

#else

int
main(int argc, char *argv[])
{
	piglit_report_result(PIGLIT_SKIP);

	return 0;
}

#endif
