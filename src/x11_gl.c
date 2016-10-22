#include "plat.h"

#ifdef PWRE_PLAT_X11

#include "x11.h"
#include <GL/glx.h>
#include <X11/extensions/Xrender.h>

typedef struct gl_wnd {
	struct pwre_wnd wnd;
	GLXContext ctx;
} *gl_wnd_t;

static void gl_free(pwre_wnd_t wnd) {
	if (glXGetCurrentContext() == ((gl_wnd_t)wnd)->ctx) {
		glXMakeCurrent(dpy, None, NULL);
	}
	glXDestroyContext(dpy, ((gl_wnd_t)wnd)->ctx);
}

const GLint gl_attr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

const int gl_attr_a[] = {
	GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
	GLX_RENDER_TYPE, GLX_RGBA_BIT,
	GLX_DOUBLEBUFFER, True,
	GLX_RED_SIZE, 1,
	GLX_GREEN_SIZE, 1,
	GLX_BLUE_SIZE, 1,
	GLX_ALPHA_SIZE, 1,
	GLX_DEPTH_SIZE, 1,
	None
};

pwre_wnd_t pwre_new_wnd_with_gl(uint64_t hints) {
	XVisualInfo *vi;

	if ((hints & PWRE_HINT_ALPHA) == PWRE_HINT_ALPHA) {
		int fbconf_len;
		GLXFBConfig *fbconf = glXChooseFBConfig(dpy, 0, gl_attr_a, &fbconf_len);
		XRenderPictFormat *pict_fmt = NULL;
		for (int i = 0; i < fbconf_len; i++) {
			vi = glXGetVisualFromFBConfig (dpy, fbconf[i]);
			if (!vi) {
				continue;
			}
			pict_fmt = XRenderFindVisualFormat (dpy, vi->visual);
			if (!pict_fmt) {
				continue;
			}
			if (pict_fmt->direct.alphaMask > 0) {
				vi = glXGetVisualFromFBConfig (dpy, fbconf[i]);
				break;
			}
			XFree(vi);
		}
	} else {
		vi = glXChooseVisual(dpy, 0, (int *)&gl_attr);
	}

	XSetWindowAttributes swa;
	swa.background_pixel = 0;
	swa.border_pixel = 0;
	swa.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

	gl_wnd_t glwnd = (gl_wnd_t)alloc_wnd(
		sizeof(struct gl_wnd),
		hints,
		vi->depth, vi->visual, CWBackPixel | CWBorderPixel | CWEventMask | CWColormap, &swa
	);
	glwnd->wnd.on_free = gl_free;

	glwnd->ctx = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	XFree(vi);
	if (!glwnd->ctx) {
		puts("Pwre: X11.glXCreateContext error!");
		pwre_wnd_destroy((pwre_wnd_t)glwnd);
		return NULL;
	}
	return (pwre_wnd_t)glwnd;
}

void pwre_gl_make_current(pwre_wnd_t wnd) {
	glXMakeCurrent(dpy, wnd->XWnd, ((gl_wnd_t)wnd)->ctx);
}

void pwre_gl_swap_buffers(pwre_wnd_t wnd) {
	glXSwapBuffers(dpy, wnd->XWnd);
}

#endif // PWRE_PLAT_X11
