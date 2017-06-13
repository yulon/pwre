#include <pwre.hpp>

#ifdef PWRE_PLAT_WIN32

#include <GL/gl.h>

#include <cassert>
#include "../uassert.h"

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

	gl_window::gl_window(uint64_t hints) : window(hints) {
		if (!available()) {
			return;
		}

		render_context._dc = GetDC(native_handle());
		if (!render_context._dc) {
			destroy();
			return;
		}

		auto pix_fmt = ChoosePixelFormat(render_context._dc, &pfd);
		if (!pix_fmt) {
			destroy();
			return;
		}

		SetPixelFormat(render_context._dc, pix_fmt, &pfd);

		render_context._nrc = wglCreateContext(render_context._dc);
		if (!render_context._nrc) {
			destroy();
			return;
		}

		on_destroy.add([this]() {
			if (wglGetCurrentDC() == this->render_context._dc) {
				wglMakeCurrent(NULL, NULL);
			}
			wglDeleteContext(this->render_context._nrc);
		});
	}

	void gl_window::render_context_type::make_current() {
		wglMakeCurrent(_dc, _nrc);
	}

	void gl_window::render_context_type::swap_buffers() {
		SwapBuffers(_dc);
	}
}

#endif // PWRE_PLAT_WIN32
