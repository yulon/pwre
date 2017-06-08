#include "../plat.h"

#ifdef PWRE_PLAT_COCOA

#include "window.hpp"

namespace pwre {
	size_t count = 0;
} /* pwre */

@implementation PwreNSWindow
	- (void)mouseDown:(NSEvent *)e {
		NSPoint pos = [e locationInWindow];
		_wnd->on_mouse_down.calls(e.buttonNumber, {(int)pos.x, (int)pos.y});
	}

	////////////////////////////////////////////////////////////////////////////

	- (void)onSize:(NSNotification *)n {
		_wnd->on_size.calls();
	}
@end

namespace pwre {
	NSAutoreleasePool *pool;

	void init() {
		pool = [[NSAutoreleasePool alloc] init];
	}

	bool checkout_events() {
		NSEvent *event;
		for (;;) {
			event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:0 inMode:NSDefaultRunLoopMode dequeue:YES];
			if (event) {
				[NSApp sendEvent:event];
			} else {
				break;
			}
		}
		return true;
	}

	bool recv_event() {
		NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:YES];
		if (event) {
			[NSApp sendEvent:event];
		}
		return true;
	}

	_window::_window(uint64_t hints) {
		nsWnd = [[PwreNSWindow alloc]
			initWithContentRect:NSMakeRect(0, 0, 150, 150)
			styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable| NSWindowStyleMaskResizable | NSWindowStyleMaskClosable)
			backing:NSBackingStoreBuffered
			defer:NO
		];
		nsWnd.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;
		[nsWnd setAcceptsMouseMovedEvents:YES];

		nsWnd->_wnd = this;
		[[NSNotificationCenter defaultCenter]
			addObserver:nsWnd
			selector:@selector(onSize:)
			name:NSWindowDidResizeNotification
			object:nsWnd
		];

		++count;
	}

	window *create(uint64_t hints) {
		return static_cast<window *>(new _window(hints));
	}

	void _window::close() {
		[nsWnd performClose:nil];
	}

	void _window::destroy() {
		[nsWnd close];
	}

	uintptr_t _window::native_handle() {
		return (uintptr_t)nsWnd;
	}

	std::string _window::title() {
		return nsWnd.title.UTF8String;
	}

	void _window::retitle(const std::string &title) {
		nsWnd.title = [NSString stringWithUTF8String:title.c_str()];
	}

	point _window::pos() {
		return {0, 0};
	}

	#define _SCREEN_W [[NSScreen mainScreen] frame].size.width
	#define _SCREEN_H [[NSScreen mainScreen] frame].size.height
	#include "../fix_pos.hpp"

	void _window::move(point pos) {
		fix_pos(pos.x, pos.y, nsWnd.frame.size.width, nsWnd.frame.size.height);
		[nsWnd setFrameOrigin:NSMakePoint(pos.x, pos.y)];
	}

	pwre::size _window::size() {
		NSSize size = [nsWnd contentLayoutRect].size;
		return {(int)size.width, (int)size.height};
	}

	void _window::resize(pwre::size sz) {
		[nsWnd setContentSize:NSMakeSize(sz.width, sz.height)];
	}

	void visible(_window *_wnd) {
		if (!_wnd->nsWnd.visible) {
			[_wnd->nsWnd makeKeyAndOrderFront:_wnd->nsWnd];
		}
	}

	void _window::add_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				visible(this);
				break;
			case PWRE_STATE_MINIMIZE:
				visible(this);
				[nsWnd miniaturize:nsWnd];
				break;
			case PWRE_STATE_MAXIMIZE:
				visible(this);
				if (!nsWnd.zoomed) {
					[nsWnd zoom:nsWnd];
				} else if (nsWnd.miniaturized) {
					[nsWnd deminiaturize:nsWnd];
				}
				break;
			case PWRE_STATE_FULLSCREEN:
				visible(this);
				if (!(nsWnd.styleMask & NSWindowStyleMaskFullScreen)) {
					[nsWnd toggleFullScreen:nsWnd];
				} else if (nsWnd.miniaturized) {
					[nsWnd deminiaturize:nsWnd];
				}
		}
	}

	void _window::rm_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				[nsWnd orderOut:nsWnd];
				break;
			case PWRE_STATE_MINIMIZE:
				visible(this);
				[nsWnd deminiaturize:nsWnd];
				break;
			case PWRE_STATE_MAXIMIZE:
				visible(this);
				if (nsWnd.zoomed) {
					[nsWnd zoom:nsWnd];
				} else if (nsWnd.miniaturized) {
					[nsWnd deminiaturize:nsWnd];
				}
				break;
			case PWRE_STATE_FULLSCREEN:
				visible(this);
				if (nsWnd.styleMask & NSWindowStyleMaskFullScreen) {
					[nsWnd toggleFullScreen:nsWnd];
				} else if (nsWnd.miniaturized) {
					[nsWnd deminiaturize:nsWnd];
				}
		}
	}

	bool _window::has_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				return nsWnd.visible;
			case PWRE_STATE_MINIMIZE:
				return nsWnd.miniaturized;
			case PWRE_STATE_MAXIMIZE:
				return nsWnd.zoomed;
			case PWRE_STATE_FULLSCREEN:
				return nsWnd.styleMask & NSWindowStyleMaskFullScreen;
		}
		return false;
	}

	void _window::less(bool less) {

	}
} /* pwre */

#endif // PWRE_PLAT_COCOA
