#include <pwre.hpp>

#ifdef PWRE_PLAT_COCOA

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

	gl_window::gl_window(uint64_t hints) : window(hints) {
		if (!available()) {
			return;
		}

		NSOpenGLPixelFormat *pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:gl2];
		if (!pixFmt) {
			destroy();
			return;
		}

		render_context._nrc = [[NSOpenGLContext alloc] initWithFormat:pixFmt shareContext:nil];
		if (!render_context._nrc) {
			destroy();
			return;
		}

		[pixFmt release];
		[render_context._nrc setView:[_nwnd contentView]];

		on_size.add([this]() {
			[this->render_context._nrc update];
		});

		on_destroy.add([this]() {
			if ([NSOpenGLContext currentContext] == this->render_context._nrc) {
				[NSOpenGLContext clearCurrentContext];
			}
			[this->render_context._nrc release];
		});
	}

	void gl_window::render_context_type::make_current() {
		[_nrc makeCurrentContext];
	}

	void gl_window::render_context_type::swap_buffers() {
		[_nrc flushBuffer];
	}
} /* pwre */

#endif // PWRE_PLAT_COCOA
