#include "plat.h"

#ifdef PWRE_PLAT_WIN32

#include "win32.h"
#include <GL/gl.h>

typedef struct gl_wnd {
	struct pwre_wnd wnd;
	HDC dc;
	HGLRC rc;
} *gl_wnd_t;

static void gl_free(pwre_wnd_t wnd) {
	if (wglGetCurrentDC() == ((gl_wnd_t)wnd)->dc) {
		wglMakeCurrent(NULL, NULL);
	}
	wglDeleteContext(((gl_wnd_t)wnd)->rc);
}

pwre_wnd_t pwre_new_wnd_with_gl(uint64_t hints) {
	gl_wnd_t glwnd = (gl_wnd_t)alloc_wnd(sizeof(struct gl_wnd), hints);
	glwnd->wnd.on_free = gl_free;

	glwnd->dc = GetDC(glwnd->wnd.hWnd);
	if (!glwnd->dc) {
		puts("Pwre: Win32.GetDC error!");
		pwre_wnd_destroy((pwre_wnd_t)glwnd);
		return NULL;
	}

	PIXELFORMATDESCRIPTOR pfd = (PIXELFORMATDESCRIPTOR){
		40,
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SUPPORT_COMPOSITION,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		8, 24, 0, 0, 0, 0, 0,
		32,
		0, 0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0,
	};

	int PixelFormat = ChoosePixelFormat(glwnd->dc, &pfd);
	if (!PixelFormat) {
		puts("Pwre: Win32.ChoosePixelFormat error!");
		pwre_wnd_destroy((pwre_wnd_t)glwnd);
		return NULL;
	}

	if (!SetPixelFormat(glwnd->dc, PixelFormat, &pfd)) {
		puts("Pwre: Win32.SetPixelFormat error!");
		pwre_wnd_destroy((pwre_wnd_t)glwnd);
		return NULL;
	}

	glwnd->rc = wglCreateContext(glwnd->dc);
	if (!glwnd->rc) {
		puts("Pwre: Win32.wglCreateContext error!");
		pwre_wnd_destroy((pwre_wnd_t)glwnd);
		return NULL;
	}

	return (pwre_wnd_t)glwnd;
}

void pwre_gl_make_current(pwre_wnd_t wnd) {
	wglMakeCurrent(((gl_wnd_t)wnd)->dc, ((gl_wnd_t)wnd)->rc);
}

void pwre_gl_swap_buffers(pwre_wnd_t wnd) {
	SwapBuffers(((gl_wnd_t)wnd)->dc);
}

#endif // PWRE_PLAT_WIN32
