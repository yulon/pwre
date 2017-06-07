#include "../plat.h"

#ifdef PWRE_PLAT_COCOA

#include "window.hpp"

namespace pwre {
	constexpr NSOpenGLPixelFormatAttribute gl2[] {
		NSOpenGLPFADoubleBuffer,
		0
	};

	/*constexpr NSOpenGLPixelFormatAttribute gl3[] {
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAOpenGLProfile,
		NSOpenGLProfileVersion3_2Core,
		0
	};*/

	class _gl_context : public gl_context {
		public:
			NSOpenGLContext *c;

			////////////////////////////////////////////////////////////////////

			_gl_context(_window *wnd) {
				NSOpenGLPixelFormat *pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:gl2];

				c = [[NSOpenGLContext alloc] initWithFormat:pixFmt shareContext:nil];
				[c setView:[wnd->nsWnd contentView]];
				[pixFmt release];

				wnd->on_size.add([this]() {
					[this->c update];
				});

				wnd->on_destroy.add([this]() {
					if ([NSOpenGLContext currentContext] == this->c) {
						[NSOpenGLContext clearCurrentContext];
					}
					[this->c release];
				});
			}

			virtual uintptr_t native_handle() {
				return (uintptr_t)c;
			}

			virtual void make_current() {
				[c makeCurrentContext];
			}

			virtual void swap_buffers() {
				[c flushBuffer];
			}
	};

	window *create(gl_context *&glc, uint64_t hints) {
		auto wnd = new _window(hints);
		glc = static_cast<gl_context *>(new _gl_context(wnd));
		return static_cast<window *>(wnd);
	}
} /* pwre */

#endif // PWRE_PLAT_COCOA
