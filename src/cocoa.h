#include "pwre.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#import <Cocoa/Cocoa.h>

struct PrWnd {
	NSWindow *nsWnd;
	PrEventHandler evtHdr;
	void (*onFree)(PrWnd);
};

PrWnd _alloc_PrWnd(size_t memSize, uint64_t mask);
