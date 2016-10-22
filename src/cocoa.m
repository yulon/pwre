#include "plat.h"

#ifdef PWRE_PLAT_COCOA

#include "cocoa.h"
#include "uni.h"

static NSAutoreleasePool *pool;

bool pwre_init(pwre_event_handler_t evt_hdr) {
	pool = [[NSAutoreleasePool alloc] init];

	event_handler = evt_hdr;
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

pwre_wnd_t alloc_wnd(size_t struct_size, uint64_t hints) {
	NSWindow *NSWnd = [[NSWindow alloc]
		initWithContentRect:NSMakeRect(x, y, width, height)
		styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable| NSWindowStyleMaskResizable | NSWindowStyleMaskClosable)
		backing:NSBackingStoreBuffered
		defer:NO
	];
	[NSWnd makeKeyAndOrderFront:NSWnd];
	[NSApp hide:NSWnd];
	NSWnd.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;

	wnd_count++;
	pwre_wnd_t wnd = calloc(1, struct_size);
	wnd->NSWnd = NSWnd;

	return wnd;
}

pwre_wnd_t pwre_new_wnd(uint64_t hints) {
	return alloc_wnd(sizeof(struct pwre_wnd), hints);
}

const char *pwre_wnd_title(pwre_wnd_t wnd) {
	return wnd->NSWnd.title.UTF8String;
}

void pwre_wnd_retitle(pwre_wnd_t wnd, const char *title) {
	wnd->NSWnd.title = [NSString stringWithUTF8String:title];
}

void pwre_wnd_move(pwre_wnd_t wnd, int x, int y) {
	fix_pos(&x, &y, wnd->NSWnd.frame.size.width, wnd->NSWnd.frame.size.height);
	[wnd->NSWnd setFrameOrigin:NSMakePoint(x, y)];
}

void pwre_wnd_size(pwre_wnd_t wnd, int *width, int *height) {
	NSSize size = [wnd->NSWnd contentLayoutRect].size;
	if (width) {
		*width = size.width;
	}
	if (height) {
		*height = size.height;
	}
}

void pwre_wnd_resize(pwre_wnd_t wnd, int width, int height) {
	[wnd->NSWnd setContentSize:NSMakeSize(width, height)];
}

static void visible(pwre_wnd_t wnd) {
	if (!wnd->NSWnd.visible) {
		[NSApp unhide:wnd->NSWnd];
	}
}

void pwre_wnd_state_add(pwre_wnd_t wnd, PWRE_STATE type) {
	switch (type) {
		case PWRE_STATE_VISIBLE:
			visible(wnd);
			break;
		case PWRE_STATE_MINIMIZE:
			visible(wnd);
			[wnd->NSWnd miniaturize:wnd->NSWnd];
			break;
		case PWRE_STATE_MAXIMIZE:
			visible(wnd);
			if (!wnd->NSWnd.zoomed) {
				[wnd->NSWnd zoom:wnd->NSWnd];
			} else if (wnd->NSWnd.miniaturized) {
				[wnd->NSWnd deminiaturize:wnd->NSWnd];
			}
			break;
		case PWRE_STATE_FULLSCREEN:
			visible(wnd);
			if (!(wnd->NSWnd.styleMask & NSWindowStyleMaskFullScreen)) {
				[wnd->NSWnd toggleFullScreen:wnd->NSWnd];
			} else if (wnd->NSWnd.miniaturized) {
				[wnd->NSWnd deminiaturize:wnd->NSWnd];
			}
	}
}

void pwre_wnd_state_rm(pwre_wnd_t wnd, PWRE_STATE type) {
	switch (type) {
		case PWRE_STATE_VISIBLE:
			[NSApp hide:wnd->NSWnd];
			break;
		case PWRE_STATE_MINIMIZE:
			visible(wnd);
			[wnd->NSWnd deminiaturize:wnd->NSWnd];
			break;
		case PWRE_STATE_MAXIMIZE:
			visible(wnd);
			if (wnd->NSWnd.zoomed) {
				[wnd->NSWnd zoom:wnd->NSWnd];
			} else if (wnd->NSWnd.miniaturized) {
				[wnd->NSWnd deminiaturize:wnd->NSWnd];
			}
			break;
		case PWRE_STATE_FULLSCREEN:
			visible(wnd);
			if (wnd->NSWnd.styleMask & NSWindowStyleMaskFullScreen) {
				[wnd->NSWnd toggleFullScreen:wnd->NSWnd];
			} else if (wnd->NSWnd.miniaturized) {
				[wnd->NSWnd deminiaturize:wnd->NSWnd];
			}
	}
}

bool pwre_wnd_state_has(pwre_wnd_t wnd, PWRE_STATE type) {
	switch (type) {
		case PWRE_STATE_VISIBLE:
			return wnd->NSWnd.visible;
		case PWRE_STATE_MINIMIZE:
			return wnd->NSWnd.miniaturized;
		case PWRE_STATE_MAXIMIZE:
			return wnd->NSWnd.zoomed;
		case PWRE_STATE_FULLSCREEN:
			return wnd->NSWnd.styleMask & NSWindowStyleMaskFullScreen;
	}
	return false;
}

#endif // PWRE_PLAT_COCOA
