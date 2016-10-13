#include "plat.h"

#ifdef PWRE_COCOA

#include "cocoa.h"
#include "uni.h"

static NSAutoreleasePool *pool;
static NSUInteger uiStyle;
static NSBackingStoreType backingStoreStyle;

bool pwreInit(PrEventHandler evtHdr) {
	pool = [[NSAutoreleasePool alloc] init];
	uiStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable| NSWindowStyleMaskResizable | NSWindowStyleMaskClosable;
	backingStoreStyle = NSBackingStoreBuffered;

	dftEvtHdr = evtHdr;
	return true;
}

bool pwreStep(void) {
	for (;;) {
		NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:0 inMode:NSDefaultRunLoopMode dequeue:YES];
		if (!event) {
			break;
		}
		[NSApp sendEvent:event];
	}
	return true;
}

void pwreRun(void) {
	[NSApp run];
	[pool drain];
}

static void fixPos(int *x, int *y, int width, int height) {
	NSSize scrSize = [[NSScreen mainScreen] frame].size;
	if (*x == PrPos_ScreenCenter) {
		*x = (scrSize.width - width) / 2;
	}
	if (*y == PrPos_ScreenCenter) {
		*y = (scrSize.height - height) / 2;
	}
}

PrWnd _alloc_PrWnd(size_t size, int x, int y, int width, int height) {
	fixPos(&x, &y, width, height);

	NSWindow *nsWnd = [[NSWindow alloc] initWithContentRect:NSMakeRect(x, y, width, height) styleMask:uiStyle backing:backingStoreStyle defer:NO];
	[nsWnd makeKeyAndOrderFront:nsWnd];
	[NSApp hide:nsWnd];
	nsWnd.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;

	wndCount++;
	PrWnd wnd = calloc(1, size);
	wnd->nsWnd = nsWnd;
	wnd->evtHdr = dftEvtHdr;

	return wnd;
}

PrWnd new_PrWnd(int x, int y, int width, int height) {
	return _alloc_PrWnd(sizeof(struct PrWnd), x, y, width, height);
}

const char *PrWnd_getTitle(PrWnd wnd) {
	return wnd->nsWnd.title.UTF8String;
}

void PrWnd_setTitle(PrWnd wnd, const char *title) {
	wnd->nsWnd.title = [NSString stringWithUTF8String:title];
}

void PrWnd_move(PrWnd wnd, int x, int y) {
	fixPos(&x, &y, wnd->nsWnd.frame.size.width, wnd->nsWnd.frame.size.height);
	[wnd->nsWnd setFrameOrigin:NSMakePoint(x, y)];
}

void PrWnd_size(PrWnd wnd, int *width, int *height) {
	NSSize size = [wnd->nsWnd contentLayoutRect].size;
	if (width) {
		*width = size.width;
	}
	if (height) {
		*height = size.height;
	}
}

void PrWnd_resize(PrWnd wnd, int width, int height) {
	[wnd->nsWnd setContentSize:NSMakeSize(width, height)];
}

static void visible(PrWnd wnd) {
	if (!wnd->nsWnd.visible) {
		[NSApp unhide:wnd->nsWnd];
	}
}

void PrWnd_view(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_Visible:
			visible(wnd);
			break;
		case PrView_Minimize:
			visible(wnd);
			[wnd->nsWnd miniaturize:wnd->nsWnd];
			break;
		case PrView_Maximize:
			visible(wnd);
			if (!wnd->nsWnd.zoomed) {
				[wnd->nsWnd zoom:wnd->nsWnd];
			} else if (wnd->nsWnd.miniaturized) {
				[wnd->nsWnd deminiaturize:wnd->nsWnd];
			}
			break;
		case PrView_Fullscreen:
			visible(wnd);
			if (!(wnd->nsWnd.styleMask & NSWindowStyleMaskFullScreen)) {
				[wnd->nsWnd toggleFullScreen:wnd->nsWnd];
			} else if (wnd->nsWnd.miniaturized) {
				[wnd->nsWnd deminiaturize:wnd->nsWnd];
			}
	}
}

void PrWnd_unview(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_Visible:
			[NSApp hide:wnd->nsWnd];
			break;
		case PrView_Minimize:
			visible(wnd);
			[wnd->nsWnd deminiaturize:wnd->nsWnd];
			break;
		case PrView_Maximize:
			visible(wnd);
			if (wnd->nsWnd.zoomed) {
				[wnd->nsWnd zoom:wnd->nsWnd];
			} else if (wnd->nsWnd.miniaturized) {
				[wnd->nsWnd deminiaturize:wnd->nsWnd];
			}
			break;
		case PrView_Fullscreen:
			visible(wnd);
			if (wnd->nsWnd.styleMask & NSWindowStyleMaskFullScreen) {
				[wnd->nsWnd toggleFullScreen:wnd->nsWnd];
			} else if (wnd->nsWnd.miniaturized) {
				[wnd->nsWnd deminiaturize:wnd->nsWnd];
			}
	}
}

bool PrWnd_viewed(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_Visible:
			return wnd->nsWnd.visible;
		case PrView_Minimize:
			return wnd->nsWnd.miniaturized;
		case PrView_Maximize:
			return wnd->nsWnd.zoomed;
		case PrView_Fullscreen:
			return wnd->nsWnd.styleMask & NSWindowStyleMaskFullScreen;
	}
	return false;
}

#endif // PWRE_COCOA
