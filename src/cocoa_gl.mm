#include "plat.h"

#ifdef PWRE_PLAT_COCOA

#include "cocoa.hpp"

namespace Pwre {
	struct GLWindow::_BlackBox {
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

	GLWindow::GLWindow(uint64_t hints):Window(hints) {
		_glm = new _BlackBox;

		NSOpenGLPixelFormatAttribute glAttr;
		if (hints & PWRE_HINT_GL_V3 == PWRE_HINT_GL_V3) {
			glAttr = gl3;
		} else {
			glAttr = gl2;
		}

		NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:glAttr];
		_glm->ctx = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
		[_glm->ctx setView:[_m->nsWnd contentView]];
		[pixelFormat release];

		OnDestroy.AddCallBack([this]() {
			if ([NSOpenGLContext currentContext] == this->_glm->ctx) {
				[NSOpenGLContext clearCurrentContext];
			}
			[this->_glm->ctx release];
		});
	}

	GLWindow::~GLWindow() {
		delete _glm;
	}

	uintptr_t GLWindow::NativeGLCtx() {
		return (uintptr_t)_glm->ctx;
	}

	void GLWindow::MakeCurrent() {
		[_glm->ctx makeCurrentContext];
	}

	void GLWindow::SwapBuffers() {
		[_glm->ctx flushBuffer];
	}
} /* Pwre */

#endif // PWRE_PLAT_COCOA
