#include "../plat.h"

#ifdef PWRE_PLAT_COCOA

#include "window.hpp"

namespace pwre {
	size_t count = 0;
} /* pwre */

@implementation PwreController
	- (void)mouseDown:(NSEvent *)e {
		NSPoint pos = [e locationInWindow];
		wnd->on_mouse_down.calls(e.buttonNumber, {(int)pos.x, (int)pos.y});
	}
@end

@implementation PwreObserver
	- (void)onSize:(NSNotification *)n {
		wnd->on_size.calls();
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
		ns = [[NSWindow alloc]
			initWithContentRect:NSMakeRect(0, 0, 150, 150)
			styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable| NSWindowStyleMaskResizable | NSWindowStyleMaskClosable)
			backing:NSBackingStoreBuffered
			defer:NO
		];
		[ns makeKeyAndOrderFront:ns];
		[NSApp hide:ns];
		ns.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;
		[ns setAcceptsMouseMovedEvents:YES];

		ctrlr = [PwreController alloc];
		ctrlr->wnd = this;
		[ctrlr initWithWindow:ns];

		obsrv = [PwreObserver alloc];
		obsrv->wnd = this;
		[[NSNotificationCenter defaultCenter]
			addObserver:obsrv
			selector:@selector(onSize:)
			name:NSWindowDidResizeNotification
			object:ns
		];

		++count;
	}

	window *create(uint64_t hints) {
		return static_cast<window *>(new _window(hints));
	}

	void _window::close() {
		[ns performClose:nil];
	}

	void _window::destroy() {
		[ns close];
	}

	uintptr_t _window::native_handle() {
		return (uintptr_t)ns;
	}

	std::string _window::title() {
		return ns.title.UTF8String;
	}

	void _window::retitle(const std::string &title) {
		ns.title = [NSString stringWithUTF8String:title.c_str()];
	}

	point _window::pos() {
		return {0, 0};
	}

	#define _SCREEN_W [[NSScreen mainScreen] frame].size.width
	#define _SCREEN_H [[NSScreen mainScreen] frame].size.height
	#include "../fix_pos.hpp"

	void _window::move(point pos) {
		fix_pos(pos.x, pos.y, ns.frame.size.width, ns.frame.size.height);
		[ns setFrameOrigin:NSMakePoint(pos.x, pos.y)];
	}

	pwre::size _window::size() {
		NSSize size = [ns contentLayoutRect].size;
		return {(int)size.width, (int)size.height};
	}

	void _window::resize(pwre::size sz) {
		[ns setContentSize:NSMakeSize(sz.width, sz.height)];
	}

	void visible(_window *wnd) {
		if (!wnd->ns.visible) {
			[NSApp unhide:wnd->ns];
		}
	}

	void _window::add_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				visible(this);
				break;
			case PWRE_STATE_MINIMIZE:
				visible(this);
				[ns miniaturize:ns];
				break;
			case PWRE_STATE_MAXIMIZE:
				visible(this);
				if (!ns.zoomed) {
					[ns zoom:ns];
				} else if (ns.miniaturized) {
					[ns deminiaturize:ns];
				}
				break;
			case PWRE_STATE_FULLSCREEN:
				visible(this);
				if (!(ns.styleMask & NSWindowStyleMaskFullScreen)) {
					[ns toggleFullScreen:ns];
				} else if (ns.miniaturized) {
					[ns deminiaturize:ns];
				}
		}
	}

	void _window::rm_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				[NSApp hide:ns];
				break;
			case PWRE_STATE_MINIMIZE:
				visible(this);
				[ns deminiaturize:ns];
				break;
			case PWRE_STATE_MAXIMIZE:
				visible(this);
				if (ns.zoomed) {
					[ns zoom:ns];
				} else if (ns.miniaturized) {
					[ns deminiaturize:ns];
				}
				break;
			case PWRE_STATE_FULLSCREEN:
				visible(this);
				if (ns.styleMask & NSWindowStyleMaskFullScreen) {
					[ns toggleFullScreen:ns];
				} else if (ns.miniaturized) {
					[ns deminiaturize:ns];
				}
		}
	}

	bool _window::has_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				return ns.visible;
			case PWRE_STATE_MINIMIZE:
				return ns.miniaturized;
			case PWRE_STATE_MAXIMIZE:
				return ns.zoomed;
			case PWRE_STATE_FULLSCREEN:
				return ns.styleMask & NSWindowStyleMaskFullScreen;
		}
		return false;
	}

	void _window::less(bool less) {

	}
} /* pwre */

#endif // PWRE_PLAT_COCOA
