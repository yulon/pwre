#include "ubw.h"

#ifdef UBWINDOW_X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>

static Display *dpy;
static Window root;
static _UBWPVT *wndList[256];

int ubwInit(void) {
	XInitThreads();
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		puts("UBWindow: X11.XOpenDisplay error!");
		return 0;
	}
	root = XRootWindow(dpy, 0);
	return 1;
}

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
		if ((Window)wndList[i]->pNtv == XWnd) {
			return i;
		}
	}
	return -1;
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
			ubwSum--;
			if (!ubwSum) {
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
	wnd->pNtv = (void *)XCreateWindow(
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

	if (!wnd->pNtv) {
		puts("UBWindow: X11.XCreateSimpleWindow error!");
		return NULL;
	}

	Atom wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(dpy, (Window)wnd->pNtv, &wmDelete, 1);
	XSelectInput(dpy, (Window)wnd->pNtv, ExposureMask | KeyPressMask | StructureNotifyMask);

	wnd->width = 10;
	wnd->height = 10;

	ubwSum++;
	wndListAdd(wnd);
	return (UBW)wnd;
}

#define _XWND (Window)((_UBWPVT *)wnd)->pNtv

int ubwGetTitle(UBW wnd, char *str8) {
	//XGetWMName(dpy, _XWND, &xtpTitle);

	return 0;
}

int ubwSetTitle(UBW wnd, char *str8) {
	if (!str8) {
		return 0;
	}
	XTextProperty xtpTitle;
	Status ok = XStringListToTextProperty(&str8, 1, &xtpTitle);
	if (!ok) {
		return 0;
	}
	XSetWMName(dpy, _XWND, &xtpTitle);
	XSetWMIconName(dpy, _XWND, &xtpTitle);
	return 1;
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
