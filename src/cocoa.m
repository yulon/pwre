#include "plat.h"

#ifdef PWRE_COCOA

#define ZK_SCOPE pwre
#define ZK_IMPL

#include "cocoa.h"
#include "uni.h"

static NSAutoreleasePool *pool;
static NSUInteger uiStyle;
static NSBackingStoreType backingStoreStyle;

bool pwre_init(PrEventHandler evtHdr) {
	pool = [[NSAutoreleasePool alloc] init];
	uiStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable| NSWindowStyleMaskResizable | NSWindowStyleMaskClosable;
	backingStoreStyle = NSBackingStoreBuffered;

	eventHandler = evtHdr;
	return true;
}

bool pwre_step(void) {
	for (;;) {
		NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:0 inMode:NSDefaultRunLoopMode dequeue:YES];
		if (!event) {
			break;
		}
		[NSApp sendEvent:event];
	}
	return true;
}

void pwre_run(void) {
	[NSApp run];
	[pool drain];
}

static void fixPos(int *x, int *y, int width, int height) {
	NSSize scrSize = [[NSScreen mainScreen] frame].size;
	if (*x == PWRE_POS_AUTO) {
		*x = (scrSize.width - width) / 2;
	}
	if (*y == PWRE_POS_AUTO) {
		*y = (scrSize.height - height) / 2;
	}
}

PrWnd _alloc_PrWnd(size_t memSize, uint64_t hints) {
	NSWindow *nsWnd = [[NSWindow alloc] initWithContentRect:NSMakeRect(x, y, width, height) styleMask:uiStyle backing:backingStoreStyle defer:NO];
	[nsWnd makeKeyAndOrderFront:nsWnd];
	[NSApp hide:nsWnd];
	nsWnd.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;

	wndCount++;
	PrWnd wnd = calloc(1, memSize);
	wnd->nsWnd = nsWnd;
	wnd->evtHdr = eventHandler;

	return wnd;
}

PrWnd new_PrWnd(uint64_t hints) {
	return _alloc_PrWnd(sizeof(struct PrWnd), hints);
}

const char *PrWnd_GetTitle(PrWnd wnd) {
	return wnd->nsWnd.title.UTF8String;
}

void PrWnd_SetTitle(PrWnd wnd, const char *title) {
	wnd->nsWnd.title = [NSString stringWithUTF8String:title];
}

void PrWnd_Move(PrWnd wnd, int x, int y) {
	fixPos(&x, &y, wnd->nsWnd.frame.size.width, wnd->nsWnd.frame.size.height);
	[wnd->nsWnd setFrameOrigin:NSMakePoint(x, y)];
}

void PrWnd_Size(PrWnd wnd, int *width, int *height) {
	NSSize size = [wnd->nsWnd contentLayoutRect].size;
	if (width) {
		*width = size.width;
	}
	if (height) {
		*height = size.height;
	}
}

void PrWnd_ReSize(PrWnd wnd, int width, int height) {
	[wnd->nsWnd setContentSize:NSMakeSize(width, height)];
}

static void visible(PrWnd wnd) {
	if (!wnd->nsWnd.visible) {
		[NSApp unhide:wnd->nsWnd];
	}
}

void PrWnd_View(PrWnd wnd, PWRE_VIEW type) {
	switch (type) {
		case PWRE_VIEW_VISIBLE:
			visible(wnd);
			break;
		case PWRE_VIEW_MINIMIZE:
			visible(wnd);
			[wnd->nsWnd miniaturize:wnd->nsWnd];
			break;
		case PWRE_VIEW_MAXIMIZE:
			visible(wnd);
			if (!wnd->nsWnd.zoomed) {
				[wnd->nsWnd zoom:wnd->nsWnd];
			} else if (wnd->nsWnd.miniaturized) {
				[wnd->nsWnd deminiaturize:wnd->nsWnd];
			}
			break;
		case PWRE_VIEW_FULLSCREEN:
			visible(wnd);
			if (!(wnd->nsWnd.styleMask & NSWindowStyleMaskFullScreen)) {
				[wnd->nsWnd toggleFullScreen:wnd->nsWnd];
			} else if (wnd->nsWnd.miniaturized) {
				[wnd->nsWnd deminiaturize:wnd->nsWnd];
			}
	}
}

void PrWnd_UnView(PrWnd wnd, PWRE_VIEW type) {
	switch (type) {
		case PWRE_VIEW_VISIBLE:
			[NSApp hide:wnd->nsWnd];
			break;
		case PWRE_VIEW_MINIMIZE:
			visible(wnd);
			[wnd->nsWnd deminiaturize:wnd->nsWnd];
			break;
		case PWRE_VIEW_MAXIMIZE:
			visible(wnd);
			if (wnd->nsWnd.zoomed) {
				[wnd->nsWnd zoom:wnd->nsWnd];
			} else if (wnd->nsWnd.miniaturized) {
				[wnd->nsWnd deminiaturize:wnd->nsWnd];
			}
			break;
		case PWRE_VIEW_FULLSCREEN:
			visible(wnd);
			if (wnd->nsWnd.styleMask & NSWindowStyleMaskFullScreen) {
				[wnd->nsWnd toggleFullScreen:wnd->nsWnd];
			} else if (wnd->nsWnd.miniaturized) {
				[wnd->nsWnd deminiaturize:wnd->nsWnd];
			}
	}
}

bool PrWnd_Viewed(PrWnd wnd, PWRE_VIEW type) {
	switch (type) {
		case PWRE_VIEW_VISIBLE:
			return wnd->nsWnd.visible;
		case PWRE_VIEW_MINIMIZE:
			return wnd->nsWnd.miniaturized;
		case PWRE_VIEW_MAXIMIZE:
			return wnd->nsWnd.zoomed;
		case PWRE_VIEW_FULLSCREEN:
			return wnd->nsWnd.styleMask & NSWindowStyleMaskFullScreen;
	}
	return false;
}

#endif // PWRE_COCOA
