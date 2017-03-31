#include "plat.h"

#ifdef PWRE_PLAT_WIN32

#include "win32.hpp"
#include <GL/gl.h>

namespace Pwre {
	namespace GL {
		struct Window::_BlackBox {
			HDC DC;
			HGLRC RC;
		};

		const PIXELFORMATDESCRIPTOR pfd{
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

		Window::Window(uint64_t hints) : Pwre::Window(hints) {
			_glm = new _BlackBox;

			_glm->DC = GetDC(_m->hWnd);
			if (!_glm->DC) {
				std::cout << "Pwre: Win32.GetDC error!" << std::endl;
				Destroy();
				return;
			}

			int pixelFormat = ChoosePixelFormat(_glm->DC, &pfd);
			if (!pixelFormat) {
				std::cout << "Pwre: Win32.ChoosePixelFormat error!" << std::endl;
				Destroy();
				return;
			}

			if (!SetPixelFormat(_glm->DC, pixelFormat, &pfd)) {
				std::cout << "Pwre: Win32.SetPixelFormat error!" << std::endl;
				Destroy();
				return;
			}

			_glm->RC = wglCreateContext(_glm->DC);
			if (!_glm->RC) {
				std::cout << "Pwre: Win32.wglCreateContext error!" << std::endl;
				Destroy();
				return;
			}

			OnDestroy.Add([this]() {
				if (wglGetCurrentDC() == this->_glm->DC) {
					wglMakeCurrent(NULL, NULL);
				}
				wglDeleteContext(this->_glm->RC);
			});

			return;
		}

		Window::~Window() {
			delete _glm;
		}

		uintptr_t Window::NativeGLCtx() {
			return (uintptr_t)_glm->RC;
		}

		void Window::MakeCurrent() {
			wglMakeCurrent(_glm->DC, _glm->RC);
		}

		static inline void swapBuffers(HDC dc) {
			SwapBuffers(dc);
		}

		void Window::SwapBuffers() {
			swapBuffers(_glm->DC);
		}
	} /* GL */
} /* Pwre */

#endif // PWRE_PLAT_WIN32
