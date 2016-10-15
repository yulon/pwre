#include "plat.h"

#ifdef PWRE_X11

#include "x11.h"
#include <GL/glx.h>
#include <X11/extensions/Xrender.h>

typedef struct PrWnd_GL {
	struct PrWnd wnd;
	GLXContext ctx;
} *PrWnd_GL;

static void _PrWnd_GL_free(PrWnd wnd) {
	if (glXGetCurrentContext() == ((PrWnd_GL)wnd)->ctx) {
		glXMakeCurrent(dpy, None, NULL);
	}
	glXDestroyContext(dpy, ((PrWnd_GL)wnd)->ctx);
}

const GLint glAttr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

const int glAlphaAttr[] = {
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

PrWnd new_PrWnd_with_GL(int x, int y, int width, int height, int flags) {
	XVisualInfo *vi;

	if ((flags & PWRE_GL_ALPHA) == PWRE_GL_ALPHA) {
		int iNumOfFBConfigs;
		GLXFBConfig *pFBConfigs = glXChooseFBConfig(dpy, 0, glAlphaAttr, &iNumOfFBConfigs);
		XRenderPictFormat *pPictFormat = NULL;
		for (int i = 0; i < iNumOfFBConfigs; i++) {
			vi = glXGetVisualFromFBConfig (dpy, pFBConfigs[i]);
			if (!vi) {
				continue;
			}
			pPictFormat = XRenderFindVisualFormat (dpy, vi->visual);
			if (!pPictFormat) {
				continue;
			}
			if (pPictFormat->direct.alphaMask > 0) {
				vi = glXGetVisualFromFBConfig (dpy, pFBConfigs[i]);
				break;
			}
			XFree(vi);
		}
	} else {
		vi = glXChooseVisual(dpy, 0, (int *)&glAttr);
	}

	XSetWindowAttributes swa;
	swa.background_pixel = 0;
	swa.border_pixel = 0;
	swa.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

	PrWnd_GL glWnd = (PrWnd_GL)_alloc_PrWnd(
		sizeof(struct PrWnd),
		x, y, width, height,
		vi->depth, vi->visual, CWBackPixel | CWBorderPixel | CWEventMask | CWColormap, &swa
	);
	glWnd->wnd.onFree = _PrWnd_GL_free;

	glWnd->ctx = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	XFree(vi);
	if (!glWnd->ctx) {
		puts("Pwre: X11.glXCreateContext error!");
		return NULL;
	}
	return (PrWnd)glWnd;
}

void PrWnd_GL_makeCurrent(PrWnd wnd) {
	glXMakeCurrent(dpy, wnd->xWnd, ((PrWnd_GL)wnd)->ctx);
}

void PrWnd_GL_swapBuffers(PrWnd wnd) {
	glXSwapBuffers(dpy, wnd->xWnd);
}

#endif // PWRE_X11
