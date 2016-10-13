#include "plat.h"

#ifdef PWRE_X11

#include "x11.h"
#include <GL/glx.h>

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

PrWnd new_PrWnd_with_GL(int x, int y, int width, int height) {
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo *vi = glXChooseVisual(dpy, 0, (int *)&att);

	XSetWindowAttributes swa;
	swa.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

	PrWnd_GL glWnd = (PrWnd_GL)_alloc_PrWnd(
		sizeof(struct PrWnd),
		x, y, width, height,
		vi->depth, vi->visual, CWEventMask | CWColormap, &swa
	);
	glWnd->wnd.onFree = _PrWnd_GL_free;

	glWnd->ctx = glXCreateContext(dpy, vi, NULL, GL_TRUE);
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
