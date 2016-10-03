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
	nsWnd.canHide = NO;
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

int ubwView(Ubw wnd, int flag) {
	if (!flag) {
		return 0;
	}
	switch (flag) {
		case UBW_VIEW_VISIBLE:
			if (_NSWND.miniaturized) {
				[_NSWND deminiaturize:_NSWND];
			} else if (!_NSWND.canHide) {
				[app unhide:_NSWND];
				_NSWND.canHide = YES;
			}
			break;
		case UBW_VIEW_HIDDEN:
			[app unhide:_NSWND];
			_NSWND.canHide = NO;
			break;
		case UBW_VIEW_MINIMIZATION:
			[_NSWND miniaturize:_NSWND];
			break;
		case UBW_VIEW_MAXIMIZATION:
			if (!_NSWND.zoomed) {
				[_NSWND zoom:_NSWND];
			}
			break;
		case UBW_VIEW_ADJUSTABLE:
			if (_NSWND.zoomed) {
				[_NSWND zoom:_NSWND];
			}
			break;
		case UBW_VIEW_FULLSCREEN:
			[_NSWND toggleFullScreen:_NSWND];
			break;
	}
	return 0;
}

#undef _NSWND
#endif // UBWINDOW_COCOA
