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
			NSOpenGLContext *nsglCtx;

			////////////////////////////////////////////////////////////////////

			_gl_context(_window *_wnd) {
				NSOpenGLPixelFormat *pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:gl2];

				nsglCtx = [[NSOpenGLContext alloc] initWithFormat:pixFmt shareContext:nil];
				[nsglCtx setView:[_wnd->nsWnd contentView]];
				[pixFmt release];

				_wnd->on_size.add([this]() {
					[this->nsglCtx update];
				});

				_wnd->on_destroy.add([this]() {
					if ([NSOpenGLContext currentContext] == this->nsglCtx) {
						[NSOpenGLContext clearCurrentContext];
					}
					[this->nsglCtx release];
				});
			}

			virtual uintptr_t native_handle() {
				return (uintptr_t)nsglCtx;
			}

			virtual void make_current() {
				[nsglCtx makeCurrentContext];
			}

			virtual void swap_buffers() {
				[nsglCtx flushBuffer];
			}
	};

	window *create(gl_context *&glc, uint64_t hints) {
		auto _wnd = new _window(hints);
		glc = static_cast<gl_context *>(new _gl_context(_wnd));
		return static_cast<window *>(_wnd);
	}
} /* pwre */

#endif // PWRE_PLAT_COCOA
