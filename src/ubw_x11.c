#include "pdbe.h"

#ifdef UBWINDOW_X11

#include "ubw.h"
#include <string.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static _UbwPvt *wndList[256];

static void wndListAdd(_UbwPvt *wnd) {
	for (int i = 0; i < 256; i++) {
		if (!wndList[i]) {
			wndList[i] = wnd;
			return;
		}
	}
	puts("UBWindow: Create the number exceeds the upper limit!");
}

static int wndListIx(Window XWnd) {
	for (int i = 0; i < 256; i++) {
		if (!wndList[i]) {
			return -1;
		}
		if ((Window)wndList[i]->ntvPtr == XWnd) {
			return i;
		}
	}
	return -1;
}

static Display *dpy;
static Window root;
Atom netWmName;
Atom utf8str;
Atom wmDelete;
Atom wmProtocols;

int ubwInit(UbwEventHandler evtHdr) {
	XInitThreads();
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		puts("UBWindow: X11.XOpenDisplay error!");
		return 0;
	}
	root = XRootWindow(dpy, 0);
	netWmName = XInternAtom(dpy, "_NET_WM_NAME", False);
	utf8str = XInternAtom(dpy, "UTF8_STRING", False);
	wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmProtocols = XInternAtom(dpy, "WM_PROTOCOLS", False);

	dftEvtHdr = evtHdr;
	return 1;
}

static int handleXEvent(XAnyEvent *event) {
	int wndIx = wndListIx(event->window);
	if (wndIx != -1) {
		_EVT_VARS(wndList[wndIx])
		switch (event->type) {
			case ConfigureNotify:
				if (((XConfigureEvent *)event)->x || ((XConfigureEvent *)event)->y) {
					wnd->x = ((XConfigureEvent *)event)->x;
					wnd->y = ((XConfigureEvent *)event)->y;
				}
				wnd->width = ((XConfigureEvent *)event)->width;
				wnd->height = ((XConfigureEvent *)event)->height;

				_EVT_POST(
					size.width = wnd->width;
					size.height = wnd->height;
					,
					UBW_EVENT_SIZE, (void *)&size)
				break;
			case Expose:
				_EVT_POST(, UBW_EVENT_PAINT, NULL)
				break;
			case ClientMessage:
				if (((XClientMessageEvent *)event)->message_type == wmProtocols) {
					_EVT_SEND(, UBW_EVENT_CLOSE, NULL,
						return 1;
					)
					XDestroyWindow(dpy, event->window);
				}
				break;
			case DestroyNotify:
				_EVT_POST(, UBW_EVENT_DESTROY, NULL)
				wndCount--;
				if (!wndCount) {
					XCloseDisplay(dpy);
					return 0;
				}
				wndList[wndIx] = NULL;
				free(wnd);
		}
	}
	return 1;
}

int ubwStep(void) {
	XEvent event;
	if (XPeekEvent(dpy, &event)) {
		XNextEvent(dpy, &event);
		return handleXEvent((XAnyEvent *)&event);
	}
	return 1;
}

void ubwRun(void) {
	XEvent event;
	int run = 1;
	while (run) {
		XNextEvent(dpy, &event);
		run = handleXEvent((XAnyEvent *)&event);
	}
}

Ubw ubwCreate() {
	XSetWindowAttributes attr;
	memset(&attr, 0, sizeof(XSetWindowAttributes));
	attr.background_pixel = XWhitePixel(dpy, 0);
	Window xWnd = XCreateWindow(
		dpy,
		root,
		0,
		0,
		10,
		10,
		0,
		XDefaultDepth(dpy, 0),
		InputOutput,
		XDefaultVisual(dpy, 0),
		CWBackPixel,
		&attr
	);
	if (!xWnd) {
		puts("UBWindow: X11.XCreateSimpleWindow error!");
		return NULL;
	}

	XSetWMProtocols(dpy, xWnd, &wmDelete, 1);
	XSelectInput(dpy, xWnd, ExposureMask | KeyPressMask | StructureNotifyMask);

	wndCount++;
	_UbwPvt *wnd = calloc(1, sizeof(_UbwPvt));
	wnd->ntvPtr = (void *)xWnd;
	wnd->width = 10;
	wnd->height = 10;

 	wndListAdd(wnd);
	return (Ubw)wnd;
}

#define _XWND (Window)((_UbwPvt *)wnd)->ntvPtr

void ubwClose(Ubw wnd) {
	if (((_UbwPvt *)wnd)->evtHdr && !(*((_UbwPvt *)wnd)->evtHdr)(wnd, UBW_EVENT_CLOSE, NULL)) {
		XDestroyWindow(dpy, _XWND);
	}
}

void ubwDestroy(Ubw wnd) {
	XDestroyWindow(dpy, _XWND);
}

int ubwGetTitle(Ubw wnd, char *title) {
	Atom type;
	int format;
	unsigned long nitems, after;
	unsigned char *data;
	if (Success == XGetWindowProperty(dpy, _XWND, netWmName, 0, LONG_MAX, False, utf8str, &type, &format, &nitems, &after, &data) && data) {
		if (title) {
			strcpy(title, (const char *)data);
		}
		int titleLen = strlen((const char *)data);
		XFree(data);
		return titleLen;
	}
	return 0;
}

void ubwSetTitle(Ubw wnd, const char *title) {
	XChangeProperty(dpy, _XWND, netWmName, utf8str, 8, PropModeReplace, (const unsigned char *)title, strlen(title));
}

void ubwMove(Ubw wnd, int x, int y) {
	if (XMoveWindow(dpy, _XWND, x, y)) {
		((_UbwPvt *)wnd)->x = x;
		((_UbwPvt *)wnd)->y = y;
	}
}

void ubwMoveToScreenCenter(Ubw wnd) {
	ubwMove(wnd, (DisplayWidth(dpy, 0) - ((_UbwPvt *)wnd)->width) / 2, (DisplayHeight(dpy, 0) - ((_UbwPvt *)wnd)->height) / 2);
}

void ubwSize(Ubw wnd, int *width, int *height) {
	if (width) {
		*width = ((_UbwPvt *)wnd)->width;
	}
	if (height) {
		*height = ((_UbwPvt *)wnd)->height;
	}
}

void ubwResize(Ubw wnd, int width, int height) {
	if (XResizeWindow(dpy, _XWND, width, height)) {
		((_UbwPvt *)wnd)->width = width;
		((_UbwPvt *)wnd)->height = width;
	}
}

void ubwShow(Ubw wnd) {
	XMapWindow(dpy, _XWND);
}

void ubwHide(Ubw wnd) {
	XUnmapWindow(dpy, _XWND);
}

#undef _XWND
#endif // UBWINDOW_X11
