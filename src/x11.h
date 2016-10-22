#include "pwre.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <zk/mux.h>

struct pwre_wnd {
	Window XWnd;
	char *title_buf;
	size_t title_buf_len;
	ZKMux data_mux;
	void (*on_free)(pwre_wnd_t);
};

pwre_wnd_t alloc_wnd(
	size_t struct_size,
	uint64_t hints,
	int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
);

extern Display *_pwre_x11_dpy;
#define dpy _pwre_x11_dpy

extern Window _pwre_x11_root;
#define root _pwre_x11_root

#define _SCREEN_W DisplayWidth(dpy, 0)
#define _SCREEN_H DisplayHeight(dpy, 0)
