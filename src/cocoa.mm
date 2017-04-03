#include "plat.h"

#ifdef PWRE_PLAT_COCOA

#include "cocoa.hpp"

namespace Pwre {
	size_t wndCount;

	void GUIThrdEntryPoint::Init() {
		pool = [[NSAutoreleasePool alloc] init];

		wndCount = 0;
	}

	GUIThrdEntryPoint guiThrdInfo;

	bool CheckoutEvents() {
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

	bool WaitEvent() {
		NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:YES];
		if (event) {
			[NSApp sendEvent:event];
		}
		return true;
	}

	void WakeUp() {

	}

	Window::Window(uint64_t hints) {
		AssertNonGUIThrd(Window);

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
		AssertNonGUIThrd(Window);

		return (uintptr_t)_m->nsWnd;
	}

	std::string Window::Title() {
		AssertNonGUIThrd(Window);

		return _m->nsWnd.title.UTF8String;
	}

	void Window::Retitle(const std::string &title) {
		AssertNonGUIThrd(Window);

		_m->nsWnd.title = [NSString stringWithUTF8String:title.c_str()];
	}

	#include "fixpos.hpp"

	void Window::Move(int x, int y) {
		AssertNonGUIThrd(Window);

		FixPos(x, y, _m->nsWnd.frame.size.width, _m->nsWnd.frame.size.height);
		[_m->nsWnd setFrameOrigin:NSMakePoint(x, y)];
	}

	void Window::Size(int &width, int &height) {
		AssertNonGUIThrd(Window);

		NSSize size = [_m->nsWnd contentLayoutRect].size;
		if (width) {
			width = size.width;
		}
		if (height) {
			height = size.height;
		}
	}

	void Window::Resize(int width, int height) {
		AssertNonGUIThrd(Window);

		[_m->nsWnd setContentSize:NSMakeSize(width, height)];
	}

	void Visible(Window::_BlackBox *_m) {
		AssertNonGUIThrd(Window);

		if (!_m->nsWnd.visible) {
			[NSApp unhide:_m->nsWnd];
		}
	}

	void Window::AddStates(uint32_t type) {
		AssertNonGUIThrd(Window);

		switch (type) {
			case State::Visible:
				Visible(_m);
				break;
			case State::Minimize:
				Visible(_m);
				[_m->nsWnd miniaturize:_m->nsWnd];
				break;
			case State::Maximize:
				Visible(_m);
				if (!_m->nsWnd.zoomed) {
					[_m->nsWnd zoom:_m->nsWnd];
				} else if (_m->nsWnd.miniaturized) {
					[_m->nsWnd deminiaturize:_m->nsWnd];
				}
				break;
			case State::FullScreen:
				Visible(_m);
				if (!(_m->nsWnd.styleMask & NSWindowStyleMaskFullScreen)) {
					[_m->nsWnd toggleFullScreen:_m->nsWnd];
				} else if (_m->nsWnd.miniaturized) {
					[_m->nsWnd deminiaturize:_m->nsWnd];
				}
		}
	}

	void Window::RmStates(uint32_t type) {
		AssertNonGUIThrd(Window);

		switch (type) {
			case State::Visible:
				[NSApp hide:_m->nsWnd];
				break;
			case State::Minimize:
				Visible(_m);
				[_m->nsWnd deminiaturize:_m->nsWnd];
				break;
			case State::Maximize:
				Visible(_m);
				if (_m->nsWnd.zoomed) {
					[_m->nsWnd zoom:_m->nsWnd];
				} else if (_m->nsWnd.miniaturized) {
					[_m->nsWnd deminiaturize:_m->nsWnd];
				}
				break;
			case State::FullScreen:
				Visible(_m);
				if (_m->nsWnd.styleMask & NSWindowStyleMaskFullScreen) {
					[_m->nsWnd toggleFullScreen:_m->nsWnd];
				} else if (_m->nsWnd.miniaturized) {
					[_m->nsWnd deminiaturize:_m->nsWnd];
				}
		}
	}

	bool Window::HasStates(uint32_t type) {
		AssertNonGUIThrd(Window);

		switch (type) {
			case State::Visible:
				return _m->nsWnd.visible;
			case State::Minimize:
				return _m->nsWnd.miniaturized;
			case State::Maximize:
				return _m->nsWnd.zoomed;
			case State::FullScreen:
				return _m->nsWnd.styleMask & NSWindowStyleMaskFullScreen;
		}
		return false;
	}

	void Window::Less(bool less) {
		AssertNonGUIThrd(Window);


	}
} /* Pwre */

#endif // PWRE_PLAT_COCOA
