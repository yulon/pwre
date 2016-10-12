#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwre.h>
#include <X11/Xlib.h>
#include "mutex.h"

struct PrWnd {
	Window xWnd;
	PrEventHandler evtHdr;
	char *titleBuf;
	size_t titleBufLen;
	Mutex dataMux;
	void (*onFree)(PrWnd);
};

PrWnd _alloc_PrWnd(
	size_t size,
	int x, int y, int width, int height,
	int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
);

extern Display *_pwre_x11_dpy;
#define dpy _pwre_x11_dpy

extern Window _pwre_x11_root;
#define root _pwre_x11_root
