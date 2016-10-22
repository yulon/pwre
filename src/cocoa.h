#include "pwre.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#import <Cocoa/Cocoa.h>

struct pwre_wnd {
	NSWindow *NSWnd;
	void (*on_free)(pwre_wnd_t);
};

pwre_wnd_t alloc_wnd(size_t struct_size, uint64_t hints);

#define _SCREEN_W [[NSScreen mainScreen] frame].size.width
#define _SCREEN_H [[NSScreen mainScreen] frame].size.height
