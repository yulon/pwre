#include "pdbe.h"

#ifdef UBWINDOW_COCOA

#include "ubw.h"
#import <Cocoa/Cocoa.h>

static NSAutoreleasePool *pool;
static NSApplication *app;
static NSUInteger uiStyle;
static NSBackingStoreType backingStoreStyle;

int ubwInit(UbwEventHandler evtHdr) {
	pool = [[NSAutoreleasePool alloc] init];
	app = [NSApplication sharedApplication];
	uiStyle = NSTitledWindowMask | NSResizableWindowMask | NSClosableWindowMask;
	backingStoreStyle = NSBackingStoreBuffered;

	dftEvtHdr = evtHdr;
	return 1;
}

void ubwRun(void) {
	[NSApp run];
	[pool drain];
}

Ubw ubwCreate(void) {
	NSWindow *win = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 500, 500) styleMask:uiStyle backing:backingStoreStyle defer:NO];
	[win makeKeyAndOrderFront:win];
	[win makeMainWindow];

	wndCount++;
	_UbwPvt wnd = calloc(1, sizeof(struct _UbwPvt));
	wnd->ntvPtr = (void *)win;
	wnd->evtHdr = dftEvtHdr;

	return (Ubw)wnd;
}

#define _NSWND (NSWindow)((_UbwPvt)wnd)->ntvPtr

int ubwGetTitle(Ubw wnd, char *title) {
	return 0;
}

void ubwSetTitle(Ubw wnd, const char *title) {

}

void ubwMove(Ubw wnd, int x, int y) {

}

void ubwMoveToScreenCenter(Ubw wnd) {

}

void ubwResize(Ubw wnd, int width, int height) {

}

void ubwShow(Ubw wnd) {

}

void ubwHide(Ubw wnd) {

}

#undef _NSWND
#endif // UBWINDOW_COCOA
