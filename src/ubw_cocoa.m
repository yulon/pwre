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

UBW ubwCreate(void) {
	NSWindow *win = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 500, 500) styleMask:uiStyle backing:backingStoreStyle defer:NO];
	[win makeKeyAndOrderFront:win];
	[win makeMainWindow];
	_UBWPVT *wnd = calloc(1, sizeof(_UBWPVT));
	wnd->ntvPtr = (void *)win;
	wndCount++;
	return (UBW)wnd;
}

#define _NSWND (NSWindow)((_UBWPVT *)wnd)->ntvPtr

int ubwGetTitle(UBW wnd, char *title) {
	return 0;
}

void ubwSetTitle(UBW wnd, const char *title) {

}

void ubwMove(UBW wnd, int x, int y) {

}

void ubwMoveToScreenCenter(UBW wnd) {

}

void ubwResize(UBW wnd, int width, int height) {

}

void ubwShow(UBW wnd) {

}

void ubwHide(UBW wnd) {

}

#undef _NSWND
#endif // UBWINDOW_COCOA
