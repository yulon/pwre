#include "plat.h"

#ifdef PWRE_PLAT_X11

#include "x11.hpp"
#include <GL/glx.h>
#include <X11/extensions/Xrender.h>

namespace Pwre {
	namespace GL {
		struct Window::_BlackBox {
			GLXContext ctx;
		};

		const GLint glAttr[]{ GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

		const int glAttrA[]{
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

		Window::Window(uint64_t hints) : Pwre::Window((uint64_t)-1) {
			_glm = new _BlackBox;

			XVisualInfo *vi;
			if ((hints & PWRE_HINT_ALPHA) == PWRE_HINT_ALPHA) {
				int fbConfLen;
				GLXFBConfig *fbConf = glXChooseFBConfig(dpy, 0, glAttrA, &fbConfLen);
				XRenderPictFormat *pictFmt = NULL;
				for (int i = 0; i < fbConfLen; i++) {
					vi = glXGetVisualFromFBConfig(dpy, fbConf[i]);
					if (!vi) {
						continue;
					}
					pictFmt = XRenderFindVisualFormat(dpy, vi->visual);
					if (!pictFmt) {
						continue;
					}
					if (pictFmt->direct.alphaMask > 0) {
						vi = glXGetVisualFromFBConfig(dpy, fbConf[i]);
						break;
					}
					XFree(vi);
				}
			} else {
				vi = glXChooseVisual(dpy, 0, (int *)&glAttr);
			}

			XSetWindowAttributes swa;
			swa.background_pixel = 0;
			swa.border_pixel = 0;
			swa.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
			swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

			if (!WindowCoreConstructor(
				this,
				hints,
				vi->depth, vi->visual, CWBackPixel | CWBorderPixel | CWEventMask | CWColormap, &swa
			)) {
				XFree(vi);
				return;
			}

			_glm->ctx = glXCreateContext(dpy, vi, NULL, GL_TRUE);
			XFree(vi);
			if (!_glm->ctx) {
				std::cout << "Pwre: X11.glXCreateContext error!" << std::endl;
				Destroy();
				return;
			}

			OnDestroy.Add([this]() {
				if (glXGetCurrentContext() == this->_glm->ctx) {
					glXMakeCurrent(dpy, None, NULL);
				}
				glXDestroyContext(dpy, this->_glm->ctx);
			});
		}

		Window::~Window() {
			delete _glm;
		}

		uintptr_t Window::NativeGLCtx() {
			return (uintptr_t)_glm->ctx;
		}

		void Window::MakeCurrent() {
			glXMakeCurrent(dpy, _m->xWnd, _glm->ctx);
		}

		void Window::SwapBuffers() {
			glXSwapBuffers(dpy, _m->xWnd);
		}
	} /* GL */
} /* Pwre */

#endif // PWRE_PLAT_X11
