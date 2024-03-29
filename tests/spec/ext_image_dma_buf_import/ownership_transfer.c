/*
 * Copyright © 2013 Intel Corporation
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
 */

#include "piglit-util-egl.h"
#include <drm_fourcc.h>
#include <unistd.h>
#include <errno.h>
#include "image_common.h"

/**
 * @file ownership_transfer.c
 *
 * From the EXT_image_dma_buf_import spec:
 *
 * "3. Does ownership of the file descriptor pass to the EGL library?
 *
 *   ANSWER: If eglCreateImageKHR is successful, EGL assumes ownership of the
 *   file descriptors and is responsible for closing them."
 *
 *
 * Here one checks that the creator of the buffer can drop its reference once
 * it has given the buffer to EGL, i.e., after calling 'eglCreateImageKHR()'.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 10;

PIGLIT_GL_TEST_CONFIG_END

static enum piglit_result
test_create_and_destroy(unsigned w, unsigned h, void *buf, int fd,
		unsigned stride, unsigned offset)
{
	EGLint error;
	EGLImageKHR img;
	EGLint attr[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ARGB8888,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		EGL_NONE
	};

	img = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)0, attr);

	/* Release the creator side of the buffer. */
	piglit_destroy_dma_buf(buf);

	error = eglGetError();

	/* EGL may not support the format, this is not an error. */
	if (!img && error == EGL_BAD_MATCH)
		return PIGLIT_SKIP;

	if (error != EGL_SUCCESS) {
		fprintf(stderr, "eglCreateImageKHR() failed: %s 0x%x\n",
			piglit_get_egl_error_name(error), error);
		return PIGLIT_FAIL;
	}

	if (!img) {
		fprintf(stderr, "image creation succeed but returned NULL\n");
		return PIGLIT_FAIL;
	}

	eglDestroyImageKHR(eglGetCurrentDisplay(), img);

	if (!piglit_check_egl_error(EGL_SUCCESS))
		return PIGLIT_FAIL;

	/**
	 * EGL stack is allowed to keep the importing file descriptor open until
	 * all resources are released. Therefore close the display first.
	 */
	if (!eglTerminate(eglGetCurrentDisplay())) {
		fprintf(stderr, "eglTerminate() failed\n");
		return PIGLIT_FAIL;
	}

	/* EGL stack should have closed the importing file descriptor by now */
	return close(fd) && errno == EBADF ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
piglit_display(void)
{
	const unsigned w = 2;
	const unsigned h = 2;
	const unsigned cpp = 4;
	const unsigned char pixels[w * h* cpp];
	struct piglit_dma_buf *buf;
	unsigned stride;
	unsigned offset;
	int fd;
	enum piglit_result res;

	res = piglit_create_dma_buf(w, h, cpp, pixels, w * cpp,
				&buf, &fd, &stride, &offset);
	if (res != PIGLIT_PASS)
		return res;

	return test_create_and_destroy(w, h, buf, fd, stride, offset);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_egl_extension("EGL_EXT_image_dma_buf_import");
}
