#include "plat.h"

#ifdef PWRE_PLAT_WIN32

#include "win32.hpp"
#include <GL/gl.h>

namespace Pwre {
	struct GLWindow::_BlackBox {
		HDC DC;
		HGLRC RC;
	};

	GLWindow::GLWindow(uint64_t hints):Window(hints) {
		_GLM = new _BlackBox;

		_GLM->DC = GetDC(_M->HWnd);
		if (!_GLM->DC) {
			std::cout << "Pwre: Win32.GetDC error!" << std::endl;
			Destroy();
			return;
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

		int pixelFormat = ChoosePixelFormat(_GLM->DC, &pfd);
		if (!pixelFormat) {
			std::cout << "Pwre: Win32.ChoosePixelFormat error!" << std::endl;
			Destroy();
			return;
		}

		if (!SetPixelFormat(_GLM->DC, pixelFormat, &pfd)) {
			std::cout << "Pwre: Win32.SetPixelFormat error!" << std::endl;
			Destroy();
			return;
		}

		_GLM->RC = wglCreateContext(_GLM->DC);
		if (!_GLM->RC) {
			std::cout << "Pwre: Win32.wglCreateContext error!" << std::endl;
			Destroy();
			return;
		}

		return;
	}

	GLWindow::~GLWindow() {
		if (wglGetCurrentDC() == _GLM->DC) {
			wglMakeCurrent(NULL, NULL);
		}
		wglDeleteContext(_GLM->RC);

		delete _GLM;
	}

	uintptr_t GLWindow::NativeGLCtx() {
		return (uintptr_t)_GLM->RC;
	}

	void GLWindow::MakeCurrent() {
		wglMakeCurrent(_GLM->DC, _GLM->RC);
	}

	static inline void swapBuffers(HDC dc) {
		SwapBuffers(dc);
	}

	void GLWindow::SwapBuffers() {
		swapBuffers(_GLM->DC);
	}
} /* Pwre */

#endif // PWRE_PLAT_WIN32