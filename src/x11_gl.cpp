#include "plat.h"

#ifdef PWRE_PLAT_X11

#include "x11.hpp"
#include <GL/glx.h>
#include <X11/extensions/Xrender.h>

namespace Pwre {
	const GLint gl_attr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

	const int gl_attr_a[] = {
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

	GLWindow::GLWindow(uint64_t hints):Window(-1) {
		XVisualInfo *vi;

		if ((hints & PWRE_HINT_ALPHA) == PWRE_HINT_ALPHA) {
			int fbConfLen;
			GLXFBConfig *fbConf = glXChooseFBConfig(System::dpy, 0, gl_attr_a, &fbConfLen);
			XRenderPictFormat *pictFmt = NULL;
			for (int i = 0; i < fbConfLen; i++) {
				vi = glXGetVisualFromFBConfig(System::dpy, fbConf[i]);
				if (!vi) {
					continue;
				}
				pictFmt = XRenderFindVisualFormat(System::dpy, vi->visual);
				if (!pictFmt) {
					continue;
				}
				if (pictFmt->direct.alphaMask > 0) {
					vi = glXGetVisualFromFBConfig(System::dpy, fbConf[i]);
					break;
				}
				XFree(vi);
			}
		} else {
			vi = glXChooseVisual(System::dpy, 0, (int *)&gl_attr);
		}

		XSetWindowAttributes swa;
		swa.background_pixel = 0;
		swa.border_pixel = 0;
		swa.colormap = XCreateColormap(System::dpy, System::root, vi->visual, AllocNone);
		swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

		WindowCoreConstructor(
			this,
			hints,
			vi->depth, vi->visual, CWBackPixel | CWBorderPixel | CWEventMask | CWColormap, &swa
		);

		_glm = (_BlackBox *)glXCreateContext(System::dpy, vi, NULL, GL_TRUE);
		XFree(vi);
		if (!_glm) {
			std::cout << "Pwre: X11.glXCreateContext error!" << std::endl;
			Destroy();
		}
	}

	GLWindow::~GLWindow() {
		if (glXGetCurrentContext() == (GLXContext)_glm) {
			glXMakeCurrent(System::dpy, None, NULL);
		}
		glXDestroyContext(System::dpy, (GLXContext)_glm);
	}

	uintptr_t GLWindow::NativeGLCtx() {
		return (uintptr_t)_glm;
	}

	void GLWindow::MakeCurrent() {
		glXMakeCurrent(System::dpy, _m->xWnd, (GLXContext)_glm);
	}

	void GLWindow::SwapBuffers() {
		glXSwapBuffers(System::dpy, _m->xWnd);
	}
} /* Pwre */

#endif // PWRE_PLAT_X11
