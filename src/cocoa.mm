#include "plat.h"

#ifdef PWRE_PLAT_COCOA

#include "cocoa.hpp"

namespace Pwre {
	std::atomic<int> wndCount;

	class WindowSystem {
		public:
			NSAutoreleasePool *pool;

			WindowSystem() {
				pool = [[NSAutoreleasePool alloc] init];

				wndCount = 0;
			}
	} wndSys;

	bool CleanEvents() {
		for (;;) {
			NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:0 inMode:NSDefaultRunLoopMode dequeue:YES];
			if (!event) {
				break;
			}
			[NSApp sendEvent:event];
		}
		return true;
	}

	bool WaitEvent() {
		NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:YES];
		if (event) {
			[NSApp sendEvent:event];
		}
		return true;
	}

	Window::Window(uint64_t hints) {
		_m = new _BlackBox;

		_m->nsWnd = [[NSWindow alloc]
			initWithContentRect:NSMakeRect(0, 0, 150, 150)
			styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable| NSWindowStyleMaskResizable | NSWindowStyleMaskClosable)
			backing:NSBackingStoreBuffered
			defer:NO
		];
		[_m->nsWnd makeKeyAndOrderFront:_m->nsWnd];
		[NSApp hide:_m->nsWnd];
		_m->nsWnd.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;

		wndCount++;
	}

	Window::~Window() {
		delete _m;
	}

	uintptr_t Window::NativeObj() {
		return (uintptr_t)_m->nsWnd;
	}

	std::string Window::Title() {
		return _m->nsWnd.title.UTF8String;
	}

	void Window::Retitle(const std::string &title) {
		_m->nsWnd.title = [NSString stringWithUTF8String:title.c_str()];
	}

	#include "fixpos.hpp"

	void Window::Move(int x, int y) {
		FixPos(x, y, _m->nsWnd.frame.size.width, _m->nsWnd.frame.size.height);
		[_m->nsWnd setFrameOrigin:NSMakePoint(x, y)];
	}

	void Window::Size(int &width, int &height) {
		NSSize size = [_m->nsWnd contentLayoutRect].size;
		if (width) {
			width = size.width;
		}
		if (height) {
			height = size.height;
		}
	}

	void Window::Resize(int width, int height) {
		[_m->nsWnd setContentSize:NSMakeSize(width, height)];
	}

	void Visible(Window::_BlackBox *_m) {
		if (!_m->nsWnd.visible) {
			[NSApp unhide:_m->nsWnd];
		}
	}

	void Window::AddStates(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				Visible(_m);
				break;
			case PWRE_STATE_MINIMIZE:
				Visible(_m);
				[_m->nsWnd miniaturize:_m->nsWnd];
				break;
			case PWRE_STATE_MAXIMIZE:
				Visible(_m);
				if (!_m->nsWnd.zoomed) {
					[_m->nsWnd zoom:_m->nsWnd];
				} else if (_m->nsWnd.miniaturized) {
					[_m->nsWnd deminiaturize:_m->nsWnd];
				}
				break;
			case PWRE_STATE_FULLSCREEN:
				Visible(_m);
				if (!(_m->nsWnd.styleMask & NSWindowStyleMaskFullScreen)) {
					[_m->nsWnd toggleFullScreen:_m->nsWnd];
				} else if (_m->nsWnd.miniaturized) {
					[_m->nsWnd deminiaturize:_m->nsWnd];
				}
		}
	}

	void Window::RmStates(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				[NSApp hide:_m->nsWnd];
				break;
			case PWRE_STATE_MINIMIZE:
				Visible(_m);
				[_m->nsWnd deminiaturize:_m->nsWnd];
				break;
			case PWRE_STATE_MAXIMIZE:
				Visible(_m);
				if (_m->nsWnd.zoomed) {
					[_m->nsWnd zoom:_m->nsWnd];
				} else if (_m->nsWnd.miniaturized) {
					[_m->nsWnd deminiaturize:_m->nsWnd];
				}
				break;
			case PWRE_STATE_FULLSCREEN:
				Visible(_m);
				if (_m->nsWnd.styleMask & NSWindowStyleMaskFullScreen) {
					[_m->nsWnd toggleFullScreen:_m->nsWnd];
				} else if (_m->nsWnd.miniaturized) {
					[_m->nsWnd deminiaturize:_m->nsWnd];
				}
		}
	}

	bool Window::HasStates(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				return _m->nsWnd.visible;
			case PWRE_STATE_MINIMIZE:
				return _m->nsWnd.miniaturized;
			case PWRE_STATE_MAXIMIZE:
				return _m->nsWnd.zoomed;
			case PWRE_STATE_FULLSCREEN:
				return _m->nsWnd.styleMask & NSWindowStyleMaskFullScreen;
		}
		return false;
	}

	void Window::Less(bool less) {
	}
} /* Pwre */

#endif // PWRE_PLAT_COCOA
