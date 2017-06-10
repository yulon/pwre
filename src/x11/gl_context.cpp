#include <pwre.hpp>

#ifdef PWRE_PLAT_X11

#include "window.hpp"

#include <X11/extensions/Xrender.h>

namespace pwre {
	constexpr GLint gl_attr[] { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

	constexpr int gl_attr_a[] {
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

	gl_window::gl_window(uint64_t hints) : window(-1) {
		XVisualInfo *vi;
		if ((hints & PWRE_HINT_ALPHA) == PWRE_HINT_ALPHA) {
			int fb_conf_len;
			GLXFBConfig *fb_conf = glXChooseFBConfig(dpy, 0, gl_attr_a, &fb_conf_len);
			XRenderPictFormat *pict_fmt = NULL;
			for (int i = 0; i < fb_conf_len; i++) {
				vi = glXGetVisualFromFBConfig(dpy, fb_conf[i]);
				if (!vi) {
					continue;
				}
				pict_fmt = XRenderFindVisualFormat(dpy, vi->visual);
				if (!pict_fmt) {
					continue;
				}
				if (pict_fmt->direct.alphaMask > 0) {
					vi = glXGetVisualFromFBConfig(dpy, fb_conf[i]);
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

		ctor(*this, hints, vi->depth, vi->visual, CWBackPixel | CWBorderPixel | CWEventMask | CWColormap, &swa);
		if (!available()) {
			XFree(vi);
			return;
		}

		render_context._nrc = glXCreateContext(dpy, vi, NULL, GL_TRUE);

		XFree(vi);

		if (!render_context._nrc) {
			destroy();
			return;
		}

		render_context._nwnd = native_handle();

		on_destroy.add([this]() {
			if (glXGetCurrentContext() == this->render_context._nrc) {
				glXMakeCurrent(dpy, None, NULL);
			}
			glXDestroyContext(dpy, this->render_context._nrc);
		});
	}

	void gl_window::render_context_type::make_current() {
		glXMakeCurrent(dpy, _nwnd, _nrc);
	}

	void gl_window::render_context_type::swap_buffers() {
		glXSwapBuffers(dpy, _nwnd);
	}
} /* pwre */

#endif // PWRE_PLAT_X11
