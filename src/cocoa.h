#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwre.h>
#import <Cocoa/Cocoa.h>

struct PrWnd {
	NSWindow *nsWnd;
	PrEventHandler evtHdr;
	void (*onFree)(PrWnd);
};

PrWnd _alloc_PrWnd(size_t size, int x, int y, int width, int height);
