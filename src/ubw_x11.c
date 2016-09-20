#include "ubw.h"

#ifdef UBWINDOW_X11

#include <string.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static _UBWPVT *wndList[256];

static void wndListAdd(_UBWPVT *wnd) {
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

int ubwInit(void) {
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
	return 1;
}

static unsigned char event[sizeof(XEvent)];

int ubwHandleEvent(void) {
	XNextEvent(dpy, (XEvent *)event);
	int ixWnd = wndListIx(((XAnyEvent *)event)->window);
	if (ixWnd != -1) {
		switch (((XAnyEvent *)event)->type)
		{
		case ConfigureNotify:
			if (((XConfigureEvent *)event)->x != 0 || ((XConfigureEvent *)event)->y != 0) {
				wndList[ixWnd]->x = ((XConfigureEvent *)event)->x;
				wndList[ixWnd]->y = ((XConfigureEvent *)event)->y;
			}
			wndList[ixWnd]->width = ((XConfigureEvent *)event)->width;
			wndList[ixWnd]->height = ((XConfigureEvent *)event)->height;
			break;
		case Expose:

			break;
		case ClientMessage:
			wndCount--;
			if (!wndCount) {
				XCloseDisplay(dpy);
				return 0;
			}
			wndList[ixWnd] = NULL;
			break;
		}
	}
	return 1;
}

UBW ubwCreate(void) {
	_UBWPVT *wnd = calloc(1, sizeof(_UBWPVT));

	XSetWindowAttributes attr = {};
	attr.background_pixel = XWhitePixel(dpy, 0);
	wnd->ntvPtr = (void *)XCreateWindow(
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

	if (!wnd->ntvPtr) {
		puts("UBWindow: X11.XCreateSimpleWindow error!");
		return NULL;
	}

	XSetWMProtocols(dpy, (Window)wnd->ntvPtr, &wmDelete, 1);
	XSelectInput(dpy, (Window)wnd->ntvPtr, ExposureMask | KeyPressMask | StructureNotifyMask);

	wnd->width = 10;
	wnd->height = 10;

	wndCount++;
	wndListAdd(wnd);
	return (UBW)wnd;
}

#define _XWND (Window)((_UBWPVT *)wnd)->ntvPtr

int ubwGetTitle(UBW wnd, char *title) {
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

void ubwSetTitle(UBW wnd, const char *title) {
	XChangeProperty(dpy, _XWND, netWmName, utf8str, 8, PropModeReplace, (const unsigned char *)title, strlen(title));
	return;
}

void ubwMove(UBW wnd, int x, int y) {
	if (XMoveWindow(dpy, _XWND, x, y)) {
		((_UBWPVT *)wnd)->x = x;
		((_UBWPVT *)wnd)->y = y;
	}
}

void ubwMoveToScreenCenter(UBW wnd) {
	ubwMove(wnd, (DisplayWidth(dpy, 0) - ((_UBWPVT *)wnd)->width) / 2, (DisplayHeight(dpy, 0) - ((_UBWPVT *)wnd)->height) / 2);
}

void ubwResize(UBW wnd, int width, int height) {
	if (XResizeWindow(dpy, _XWND, width, height)) {
		((_UBWPVT *)wnd)->width = width;
		((_UBWPVT *)wnd)->height = width;
	}
}

void ubwShow(UBW wnd) {
	XMapWindow(dpy, _XWND);
}

void ubwHide(UBW wnd) {
	XUnmapWindow(dpy, _XWND);
}

#undef _XWND
#endif // UBWINDOW_X11
