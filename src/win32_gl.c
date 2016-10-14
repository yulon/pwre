#include "plat.h"

#ifdef PWRE_WIN32

#include "win32.h"

typedef struct PrWnd_GL {
	struct PrWnd wnd;
	HDC dc;
	HDC mdc;
	HGLRC rc;
} *PrWnd_GL;

static void _PrWnd_GL_free(PrWnd wnd) {
	if (wglGetCurrentDC() == ((PrWnd_GL)wnd)->dc) {
		wglMakeCurrent(NULL, NULL);
	}
	wglDeleteContext(((PrWnd_GL)wnd)->rc);
}

PrWnd new_PrWnd_with_GL(int x, int y, int width, int height, int flags) {
	PrWnd_GL glWnd = (PrWnd_GL)_alloc_PrWnd(sizeof(struct PrWnd_GL), x, y, width, height);
	glWnd->wnd.onFree = _PrWnd_GL_free;

	glWnd->dc = GetDC(glWnd->wnd.hWnd);
	if (!glWnd->dc) {
		puts("Pwre: Win32.GetDC error!");
		return NULL;
	}

	PIXELFORMATDESCRIPTOR pfd = (PIXELFORMATDESCRIPTOR){
		40,
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
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

	if ((flags & PWRE_GL_ALPHA) == PWRE_GL_ALPHA) {
		/*DWM_BLURBEHIND bb = {0};
		HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
		bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
		bb.hRgnBlur = hRgn;
		bb.fEnable = TRUE;
		bb.fTransitionOnMaximized = 1;
		DwmEnableBlurBehindWindow(glWnd->wnd.hWnd, &bb);

		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_TYPE_RGBA | PFD_SUPPORT_COMPOSITION;
		*/
		

	} else {
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	}

	int PixelFormat = ChoosePixelFormat(glWnd->dc, &pfd);
	if (!PixelFormat) {
		puts("Pwre: Win32.ChoosePixelFormat error!");
		return NULL;
	}

	if (!SetPixelFormat(glWnd->dc, PixelFormat, &pfd)) {
		puts("Pwre: Win32.SetPixelFormat error!");
		return NULL;
	}

	glWnd->rc = wglCreateContext(glWnd->dc);
	if (!glWnd->rc) {
		puts("Pwre: Win32.wglCreateContext error!");
		return NULL;
	}

	return (PrWnd)glWnd;
}

void PrWnd_GL_makeCurrent(PrWnd wnd) {
	wglMakeCurrent(((PrWnd_GL)wnd)->dc, ((PrWnd_GL)wnd)->rc);
}

void PrWnd_GL_swapBuffers(PrWnd wnd) {
	SwapBuffers(((PrWnd_GL)wnd)->dc);
	//int w, h;
	//PrWnd_size(wnd, &w, &h);
	//BitBlt(((PrWnd_GL)wnd)->dc, 0, 0, w, h, ((PrWnd_GL)wnd)->mdc, 0, 0, SRCCOPY);
}

#endif // PWRE_WIN32
