#include <pwre.hpp>

#ifdef PWRE_PLAT_COCOA

namespace pwre {
	size_t count = 0;
	bool life = true;
} /* pwre */

@interface PwreNSWindow : NSWindow {
	@public
		pwre::window &wnd;
}
@end

@implementation PwreNSWindow
	- (void)mouseDown:(NSEvent *)e {
		NSPoint pos = [e locationInWindow];
		wnd.on_mouse_down.calls(e.buttonNumber, {(int)pos.x, (int)pos.y});
	}

	////////////////////////////////////////////////////////////////////////////

	- (void)onClose:(NSNotification *)n {
		switch (self.releasedWhenClosed) {
			default:
			if (wnd.on_close.calls()) {
				self.releasedWhenClosed = YES;
				case YES:
				wnd.on_destroy.calls();
				[self release];
				if (!--pwre::count) {
					pwre::life = false;
				}
			}
		}
	}

	- (void)onSize:(NSNotification *)n {
		wnd.on_size.calls();
	}
@end

namespace pwre {
	NSAutoreleasePool *pool;

	void init_cocoa() {
		pool = [[NSAutoreleasePool alloc] init];
	}

	bool checkout_events() {
		NSEvent *event;
		while (life) {
			event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:0 inMode:NSDefaultRunLoopMode dequeue:YES];
			if (event) {
				[NSApp sendEvent:event];
			} else {
				return life;
			}
		}
		return false;
	}

	bool recv_event() {
		if (life) {
			NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:YES];
			if (event) {
				[NSApp sendEvent:event];
			}
			return life;
		}
		return false;
	}

	window::window(uint64_t hints) {
		_nwnd = [[PwreNSWindow alloc]
			initWithContentRect:NSMakeRect(0, 0, 150, 150)
			styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable| NSWindowStyleMaskResizable | NSWindowStyleMaskClosable)
			backing:NSBackingStoreBuffered
			defer:NO
		];
		if (!_nwnd) {
			return;
		}

		++count;

		_nwnd.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;
		[_nwnd setAcceptsMouseMovedEvents:YES];

		_nwnd->wnd = *this;

		_nwnd.releasedWhenClosed = NO;
		[[NSNotificationCenter defaultCenter]
			addObserver:_nwnd
			selector:@selector(onClose:)
			name:NSWindowWillCloseNotification
			object:_nwnd
		];

		[[NSNotificationCenter defaultCenter]
			addObserver:_nwnd
			selector:@selector(onSize:)
			name:NSWindowDidResizeNotification
			object:_nwnd
		];

		on_destroy.add([this]() {
			this->_nwnd = NULL;
		});
	}

	void window::close() {
		[_nwnd close];
	}

	void window::destroy() {
		_nwnd.releasedWhenClosed = YES;
		[_nwnd close];
	}

	std::string window::title() {
		return _nwnd.title.UTF8String;
	}

	void window::retitle(const std::string &title) {
		_nwnd.title = [NSString stringWithUTF8String:title.c_str()];
	}

	window::pos_type window::pos() {
		return {0, 0};
	}

	#define _SCREEN_W [[NSScreen mainScreen] frame].size.width
	#define _SCREEN_H [[NSScreen mainScreen] frame].size.height
	#include "../fix_pos.hpp"

	void window::move(window::pos_type pos) {
		fix_pos(pos.x, pos.y, _nwnd.frame.size.width, _nwnd.frame.size.height);
		[_nwnd setFrameOrigin:NSMakePoint(pos.x, pos.y)];
	}

	window::size_type window::size() {
		NSSize size = [_nwnd contentLayoutRect].size;
		return {(int)size.width, (int)size.height};
	}

	void window::resize(window::size_type sz) {
		[_nwnd setContentSize:NSMakeSize(sz.width, sz.height)];
	}

	void visible(NSWindow *nwnd) {
		if (!nwnd.visible) {
			[nwnd makeKeyAndOrderFront:nwnd];
		}
	}

	void window::add_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				visible(_nwnd);
				break;
			case PWRE_STATE_MINIMIZE:
				visible(_nwnd);
				[_nwnd miniaturize:_nwnd];
				break;
			case PWRE_STATE_MAXIMIZE:
				visible(_nwnd);
				if (!_nwnd.zoomed) {
					[_nwnd zoom:_nwnd];
				} else if (_nwnd.miniaturized) {
					[_nwnd deminiaturize:_nwnd];
				}
				break;
			case PWRE_STATE_FULLSCREEN:
				visible(_nwnd);
				if (!(_nwnd.styleMask & NSWindowStyleMaskFullScreen)) {
					[_nwnd toggleFullScreen:_nwnd];
				} else if (_nwnd.miniaturized) {
					[_nwnd deminiaturize:_nwnd];
				}
		}
	}

	void window::rm_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				[_nwnd orderOut:_nwnd];
				break;
			case PWRE_STATE_MINIMIZE:
				visible(_nwnd);
				[_nwnd deminiaturize:_nwnd];
				break;
			case PWRE_STATE_MAXIMIZE:
				visible(_nwnd);
				if (_nwnd.zoomed) {
					[_nwnd zoom:_nwnd];
				} else if (_nwnd.miniaturized) {
					[_nwnd deminiaturize:_nwnd];
				}
				break;
			case PWRE_STATE_FULLSCREEN:
				visible(_nwnd);
				if (_nwnd.styleMask & NSWindowStyleMaskFullScreen) {
					[_nwnd toggleFullScreen:_nwnd];
				} else if (_nwnd.miniaturized) {
					[_nwnd deminiaturize:_nwnd];
				}
		}
	}

	bool window::has_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				return _nwnd.visible;
			case PWRE_STATE_MINIMIZE:
				return _nwnd.miniaturized;
			case PWRE_STATE_MAXIMIZE:
				return _nwnd.zoomed;
			case PWRE_STATE_FULLSCREEN:
				return _nwnd.styleMask & NSWindowStyleMaskFullScreen;
		}
		return false;
	}

	void window::less(bool lessed) {

	}
} /* pwre */

#endif // PWRE_PLAT_COCOA
