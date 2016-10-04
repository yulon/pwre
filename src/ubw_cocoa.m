#include "pdbe.h"

#ifdef UBWINDOW_COCOA

#include "ubw.h"
#import <Cocoa/Cocoa.h>

static NSAutoreleasePool *pool;
static NSApplication *app;
static NSUInteger uiStyle;
static NSBackingStoreType backingStoreStyle;

bool ubwInit(UbwEventHandler evtHdr) {
	pool = [[NSAutoreleasePool alloc] init];
	app = [NSApplication sharedApplication];
	uiStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable| NSWindowStyleMaskResizable | NSWindowStyleMaskClosable;
	backingStoreStyle = NSBackingStoreBuffered;

	dftEvtHdr = evtHdr;
	return true;
}

void ubwRun(void) {
	[NSApp run];
	[pool drain];
}

Ubw ubwCreate(void) {
	NSWindow *nsWnd = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 1, 1) styleMask:uiStyle backing:backingStoreStyle defer:NO];
	[nsWnd makeKeyAndOrderFront:nsWnd];
	[app hide:nsWnd];
	nsWnd.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;

	wndCount++;
	_UbwPvt wnd = calloc(1, sizeof(struct _UbwPvt));
	wnd->ntvPtr = (void *)nsWnd;
	wnd->evtHdr = dftEvtHdr;

	return (Ubw)wnd;
}

#define _NSWND ((NSWindow *)((_UbwPvt)wnd)->ntvPtr)

int ubwGetTitle(Ubw wnd, char *title) {
	if (title) {
		strcpy(title, _NSWND.title.UTF8String);
	}
	return strlen(_NSWND.title.UTF8String);
}

void ubwSetTitle(Ubw wnd, const char *title) {
	_NSWND.title = [NSString stringWithUTF8String:title];
}

void ubwMove(Ubw wnd, int x, int y) {
	[_NSWND setFrameOrigin:NSMakePoint(x, y)];
}

void ubwMoveToScreenCenter(Ubw wnd) {
	NSSize scrSize = [[NSScreen mainScreen] frame].size;
	ubwMove(wnd, (scrSize.width - _NSWND.frame.size.width) / 2, (scrSize.height - _NSWND.frame.size.height) / 2);
}

void ubwSize(Ubw wnd, int *width, int *height) {
	NSSize size = [_NSWND contentLayoutRect].size;
	if (width) {
		*width = size.width;
	}
	if (height) {
		*height = size.height;
	}
}

void ubwResize(Ubw wnd, int width, int height) {
	[_NSWND setContentSize:NSMakeSize(width, height)];
}

static void visible(Ubw wnd) {
	if (!_NSWND.visible) {
		[app unhide:_NSWND];
	}
}

void ubwView(Ubw wnd, int type) {
	switch (type) {
		case UBW_VIEW_VISIBLE:
			visible(wnd);
			break;
		case UBW_VIEW_MINIMIZE:
			visible(wnd);
			[_NSWND miniaturize:_NSWND];
			break;
		case UBW_VIEW_MAXIMIZE:
			visible(wnd);
			if (!_NSWND.zoomed) {
				[_NSWND zoom:_NSWND];
			} else if (_NSWND.miniaturized) {
				[_NSWND deminiaturize:_NSWND];
			}
			break;
		case UBW_VIEW_FULLSCREEN:
			visible(wnd);
			if (!(_NSWND.styleMask & NSWindowStyleMaskFullScreen)) {
				[_NSWND toggleFullScreen:_NSWND];
			} else if (_NSWND.miniaturized) {
				[_NSWND deminiaturize:_NSWND];
			}
	}
}

void ubwUnview(Ubw wnd, int type) {
	switch (type) {
		case UBW_VIEW_VISIBLE:
			[app hide:_NSWND];
			break;
		case UBW_VIEW_MINIMIZE:
			visible(wnd);
			[_NSWND deminiaturize:_NSWND];
			break;
		case UBW_VIEW_MAXIMIZE:
			visible(wnd);
			if (_NSWND.zoomed) {
				[_NSWND zoom:_NSWND];
			} else if (_NSWND.miniaturized) {
				[_NSWND deminiaturize:_NSWND];
			}
			break;
		case UBW_VIEW_FULLSCREEN:
			visible(wnd);
			if (_NSWND.styleMask & NSWindowStyleMaskFullScreen) {
				[_NSWND toggleFullScreen:_NSWND];
			} else if (_NSWND.miniaturized) {
				[_NSWND deminiaturize:_NSWND];
			}
	}
}

bool ubwViewed(Ubw wnd, int type) {
	switch (type) {
		case UBW_VIEW_VISIBLE:
			return _NSWND.visible;
		case UBW_VIEW_MINIMIZE:
			return _NSWND.miniaturized;
		case UBW_VIEW_MAXIMIZE:
			return _NSWND.zoomed;
		case UBW_VIEW_FULLSCREEN:
			return _NSWND.styleMask & NSWindowStyleMaskFullScreen;
	}
	return false;
}

#undef _NSWND
#endif // UBWINDOW_COCOA
