/*
 * Copyright © 2009 Intel Corporation
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

#pragma once
#ifndef PIGLIT_FRAMEWORK_H
#define PIGLIT_FRAMEWORK_H

#include <assert.h>
#include <stdbool.h>

/**
 * A bitmask of these enums specifies visual attributes for the test's window.
 *
 * Each enum has the same value of its corresponding GLUT enum. That is, for
 * each X, `PIGLIT_GL_VISUAL_X == GLUT_X`.
 *
 * \see piglit_gl_test_config::window_visual
 */
enum piglit_gl_visual {
	PIGLIT_GL_VISUAL_RGB 		= 0,
	PIGLIT_GL_VISUAL_RGBA 		= 0,
	PIGLIT_GL_VISUAL_SINGLE 	= 0,
	PIGLIT_GL_VISUAL_DOUBLE 	= 1 << 1,
	PIGLIT_GL_VISUAL_ACCUM 		= 1 << 2,
	PIGLIT_GL_VISUAL_ALPHA 		= 1 << 3,
	PIGLIT_GL_VISUAL_DEPTH 		= 1 << 4,
	PIGLIT_GL_VISUAL_STENCIL 	= 1 << 5,
};

/**
 * @brief Configuration for running an OpenGL test.
 *
 * To run a test, pass this to piglit_gl_test_run().
 *
 * This is named piglit_gl_test_config, rather than piglit_test_config, in
 * order to distinguish it from other test types, such as EGL and GLX tests.
 *
 * TODO: Add fields here that declare test requirements on GL context
 * TODO: flavor, extensions, and window system.
 */
struct piglit_gl_test_config {
	int window_width;
	int window_height;

	/**
	 * A bitmask of `enum piglit_gl_visual`.
	 */
	int window_visual;

	/**
	 * This is called once per test, after the GL context has been created
	 * and made current but before display() is called.
	 */
	void
	(*init)(int argc, char *argv[]);

	/**
	 * If the test is run in auto mode, then this is called once after
	 * init(). Otherwise, it is called repeatedly from some ill-defined
	 * event loop.
	 */
	enum piglit_result
	(*display)(void);
};

/**
 * Initialize @a config with default values.
 */
void
piglit_gl_test_config_init(struct piglit_gl_test_config *config);

/**
 * Run the OpenGL test described by @a config. Does not return.
 */
void
piglit_gl_test_run(int argc, char *argv[],
		   const struct piglit_gl_test_config *config);

#ifdef __cplusplus
#  define PIGLIT_EXTERN_C_BEGIN extern "C" {
#  define PIGLIT_EXTERN_C_END   }
#else
#  define PIGLIT_EXTERN_C_BEGIN
#  define PIGLIT_EXTERN_C_END
#endif

#define PIGLIT_GL_TEST_CONFIG_BEGIN                                          \
                                                                             \
        PIGLIT_EXTERN_C_BEGIN                                                \
                                                                             \
        void                                                                 \
        piglit_init(int argc, char *argv[]);                                 \
                                                                             \
        enum piglit_result                                                   \
        piglit_display(void);                                                \
                                                                             \
        PIGLIT_EXTERN_C_END                                                  \
                                                                             \
        int                                                                  \
        main(int argc, char *argv[])                                         \
        {                                                                    \
                struct piglit_gl_test_config config;                         \
                                                                             \
                piglit_gl_test_config_init(&config);                         \
                                                                             \
                config.init = piglit_init;                                   \
                config.display = piglit_display;                             \

#define PIGLIT_GL_TEST_CONFIG_END                                            \
                                                                             \
                piglit_gl_test_run(argc, argv, &config);                     \
                                                                             \
                assert(false);                                               \
                return 0;                                                    \
        }

extern int piglit_automatic;

extern int piglit_width;
extern int piglit_height;
extern bool piglit_use_fbo;
extern unsigned int piglit_winsys_fbo;

void piglit_swap_buffers(void);
extern void piglit_present_results();
void piglit_post_redisplay(void);
void piglit_set_keyboard_func(void (*func)(unsigned char key, int x, int y));
void piglit_set_reshape_func(void (*func)(int w, int h));

#endif /* PIGLIT_FRAMEWORK_H */