#include "../plat.h"

#ifdef PWRE_PLAT_WIN32

#include "window.hpp"
#include <GL/gl.h>

namespace pwre {
	constexpr PIXELFORMATDESCRIPTOR pfd {
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

	class _gl_context : public gl_context {
		public:
			HDC dc;
			HGLRC rc;

			////////////////////////////////////////////////////////////////////////

			_gl_context(_window *_wnd) {
				dc = GetDC(_wnd->hwnd);
				uassert(dc, "Pwre", "GetDC");

				int pix_fmt = ChoosePixelFormat(dc, &pfd);
				uassert(pix_fmt, "Pwre", "ChoosePixelFormat");

				SetPixelFormat(dc, pix_fmt, &pfd);

				rc = wglCreateContext(dc);
				uassert(rc, "Pwre", "wglCreateContext");

				_wnd->on_destroy.add([this]() {
					if (wglGetCurrentDC() == this->dc) {
						wglMakeCurrent(NULL, NULL);
					}
					wglDeleteContext(this->rc);
					delete this;
				});
			}

			virtual uintptr_t native_handle() {
				return (uintptr_t)rc;
			}

			virtual void make_current() {
				wglMakeCurrent(dc, rc);
			}

			virtual void swap_buffers() {
				SwapBuffers(dc);
			}
	};

	window *create(gl_context *&glc, uint64_t hints) {
		auto _wnd = new _window(hints);
		glc = static_cast<gl_context *>(new _gl_context(_wnd));
		return static_cast<window *>(_wnd);
	}
}

#endif // PWRE_PLAT_WIN32
