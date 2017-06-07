#include "../plat.h"

#ifdef PWRE_PLAT_X11

#include "window.hpp"
#include <GL/glx.h>
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

	class _gl_context : public gl_context {
		public:
			Window x;
			GLXContext c;

			////////////////////////////////////////////////////////////////////

			_gl_context(Window xwnd, GLXContext glxc) : x(xwnd), c(glxc) {}

			virtual uintptr_t native_handle() {
				return (uintptr_t)c;
			}

			virtual void make_current() {
				glXMakeCurrent(dpy, x, c);
			}

			virtual void swap_buffers() {
				glXSwapBuffers(dpy, x);
			}
	};

	window *create(gl_context *&glc, uint64_t hints) {
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

		auto wnd = new _window(
			hints,
			vi->depth, vi->visual, CWBackPixel | CWBorderPixel | CWEventMask | CWColormap, &swa
		);
		if (!wnd->x) {
			delete wnd;
			XFree(vi);
			return NULL;
		}

		auto glxc = glXCreateContext(dpy, vi, NULL, GL_TRUE);

		XFree(vi);

		uassert(glxc, "Pwre", "glXCreateContext");

		wnd->on_destroy.add([glxc]() {
			if (glXGetCurrentContext() == glxc) {
				glXMakeCurrent(dpy, None, NULL);
			}
			glXDestroyContext(dpy, glxc);
		});

		glc = static_cast<gl_context *>(new _gl_context(wnd->x, glxc));

		return static_cast<window *>(wnd);
	}
} /* pwre */

#endif // PWRE_PLAT_X11
