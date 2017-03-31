#include "plat.h"

#ifdef PWRE_PLAT_COCOA

#include "cocoa.hpp"

namespace Pwre {
	namespace GL {
		struct Window::_BlackBox {
			NSOpenGLContext *ctx;
		};

		const NSOpenGLPixelFormatAttribute gl2[]{
			NSOpenGLPFADoubleBuffer,
			0
		};

		const NSOpenGLPixelFormatAttribute gl3[]{
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFAOpenGLProfile,
			NSOpenGLProfileVersion3_2Core,
			0
		};

		Window::Window(uint64_t hints) : Pwre::Window(hints) {
			_glm = new _BlackBox;

			NSOpenGLPixelFormat *pixelFormat;
			if ((hints & PWRE_HINT_GL_V3) == PWRE_HINT_GL_V3) {
				pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:gl3];
			} else {
				pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:gl2];
			}

			_glm->ctx = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
			[_glm->ctx setView:[_m->nsWnd contentView]];
			[pixelFormat release];

			OnDestroy.Add([this]() {
				if ([NSOpenGLContext currentContext] == this->_glm->ctx) {
					[NSOpenGLContext clearCurrentContext];
				}
				[this->_glm->ctx release];
			});
		}

		Window::~Window() {
			delete _glm;
		}

		uintptr_t Window::NativeGLCtx() {
			return (uintptr_t)_glm->ctx;
		}

		void Window::MakeCurrent() {
			[_glm->ctx makeCurrentContext];
		}

		void Window::SwapBuffers() {
			[_glm->ctx flushBuffer];
		}
	} /* GL */
} /* Pwre */

#endif // PWRE_PLAT_COCOA
