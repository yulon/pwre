#include "pdbe.h"

#ifdef PWRE_BE_COCOA

#include "uni.h"
#import <Cocoa/Cocoa.h>

static NSAutoreleasePool *pool;
static NSApplication *app;
static NSUInteger uiStyle;
static NSBackingStoreType backingStoreStyle;

bool pwreInit(PrEventHandler evtHdr) {
	pool = [[NSAutoreleasePool alloc] init];
	app = [NSApplication sharedApplication];
	uiStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable| NSWindowStyleMaskResizable | NSWindowStyleMaskClosable;
	backingStoreStyle = NSBackingStoreBuffered;

	dftEvtHdr = evtHdr;
	return true;
}

void pwreRun(void) {
	[NSApp run];
	[pool drain];
}

PrWnd new_PrWnd(void) {
	NSWindow *nsWnd = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 1, 1) styleMask:uiStyle backing:backingStoreStyle defer:NO];
	[nsWnd makeKeyAndOrderFront:nsWnd];
	[app hide:nsWnd];
	nsWnd.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;

	wndCount++;
	PrWndPvt wnd = calloc(1, sizeof(struct PrWndPvt));
	wnd->ntvPtr = (void *)nsWnd;
	wnd->evtHdr = dftEvtHdr;

	return (PrWnd)wnd;
}

#define _NSWND ((NSWindow *)((PrWndPvt)wnd)->ntvPtr)

int PrWnd_getTitle(PrWnd wnd, char *title) {
	if (title) {
		strcpy(title, _NSWND.title.UTF8String);
	}
	return strlen(_NSWND.title.UTF8String);
}

void PrWnd_setTitle(PrWnd wnd, const char *title) {
	_NSWND.title = [NSString stringWithUTF8String:title];
}

void PrWnd_move(PrWnd wnd, int x, int y) {
	[_NSWND setFrameOrigin:NSMakePoint(x, y)];
}

void PrWnd_moveToScreenCenter(PrWnd wnd) {
	NSSize scrSize = [[NSScreen mainScreen] frame].size;
	PrWnd_move(wnd, (scrSize.width - _NSWND.frame.size.width) / 2, (scrSize.height - _NSWND.frame.size.height) / 2);
}

void PrWnd_size(PrWnd wnd, int *width, int *height) {
	NSSize size = [_NSWND contentLayoutRect].size;
	if (width) {
		*width = size.width;
	}
	if (height) {
		*height = size.height;
	}
}

void PrWnd_resize(PrWnd wnd, int width, int height) {
	[_NSWND setContentSize:NSMakeSize(width, height)];
}

static void visible(PrWnd wnd) {
	if (!_NSWND.visible) {
		[app unhide:_NSWND];
	}
}

void PrWnd_view(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_visible:
			visible(wnd);
			break;
		case PrView_minimize:
			visible(wnd);
			[_NSWND miniaturize:_NSWND];
			break;
		case PrView_maximize:
			visible(wnd);
			if (!_NSWND.zoomed) {
				[_NSWND zoom:_NSWND];
			} else if (_NSWND.miniaturized) {
				[_NSWND deminiaturize:_NSWND];
			}
			break;
		case PrView_fullscreen:
			visible(wnd);
			if (!(_NSWND.styleMask & NSWindowStyleMaskFullScreen)) {
				[_NSWND toggleFullScreen:_NSWND];
			} else if (_NSWND.miniaturized) {
				[_NSWND deminiaturize:_NSWND];
			}
	}
}

void PrWnd_unview(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_visible:
			[app hide:_NSWND];
			break;
		case PrView_minimize:
			visible(wnd);
			[_NSWND deminiaturize:_NSWND];
			break;
		case PrView_maximize:
			visible(wnd);
			if (_NSWND.zoomed) {
				[_NSWND zoom:_NSWND];
			} else if (_NSWND.miniaturized) {
				[_NSWND deminiaturize:_NSWND];
			}
			break;
		case PrView_fullscreen:
			visible(wnd);
			if (_NSWND.styleMask & NSWindowStyleMaskFullScreen) {
				[_NSWND toggleFullScreen:_NSWND];
			} else if (_NSWND.miniaturized) {
				[_NSWND deminiaturize:_NSWND];
			}
	}
}

bool PrWnd_viewed(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_visible:
			return _NSWND.visible;
		case PrView_minimize:
			return _NSWND.miniaturized;
		case PrView_maximize:
			return _NSWND.zoomed;
		case PrView_fullscreen:
			return _NSWND.styleMask & NSWindowStyleMaskFullScreen;
	}
	return false;
}

#undef _NSWND
#endif // PWRE_BE_COCOA
