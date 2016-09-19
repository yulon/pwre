#include "ubw.h"

#ifdef UBWINDOW_X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>

static Display* dpy;
static Window root;
static _Ubw* _UbwList[256];

int ubwInit() {
	XInitThreads();
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		puts("UBWindow: X11.XOpenDisplay error!");
		return 0;
	}
	root = XRootWindow(dpy, 0);
	return 1;
}

static void _UbwListAdd(_Ubw* wnd) {
	for (int i = 0; i < 256; i++) {
		if (!_UbwList[i]) {
			_UbwList[i] = wnd;
			return;
		}
	}
	puts("UBWindow: Create the number exceeds the upper limit!");
}

static int _UbwListIx(Window xWnd) {
	for (int i = 0; i < 256; i++) {
		if (!_UbwList[i]) {
			return -1;
		}
		if ((Window)_UbwList[i]->pNtv == xWnd) {
			return i;
		}
	}
	return -1;
}

static unsigned char event[sizeof(XEvent)];

int ubwHandleEvent() {
	XNextEvent(dpy, (XEvent*)event);
	int ixWnd = _UbwListIx(((XAnyEvent*)event)->window);
	if (ixWnd != -1) {
		switch (((XAnyEvent*)event)->type)
		{
		case ConfigureNotify:
			if (((XConfigureEvent*)event)->x != 0 || ((XConfigureEvent*)event)->y != 0) {
				_UbwList[ixWnd]->x = ((XConfigureEvent*)event)->x;
				_UbwList[ixWnd]->y = ((XConfigureEvent*)event)->y;
			}
			_UbwList[ixWnd]->width = ((XConfigureEvent*)event)->width;
			_UbwList[ixWnd]->height = ((XConfigureEvent*)event)->height;
			break;
		case Expose:

			break;
		case ClientMessage:
			ubwSum--;
			if (!ubwSum) {
				XCloseDisplay(dpy);
				return 0;
			}
			_UbwList[ixWnd] = NULL;
			break;
		}
	}
	return 1;
}

Ubw ubwCreate() {
	_Ubw* wnd = calloc(1, sizeof(_Ubw));

	XSetWindowAttributes attr = {};
	attr.background_pixel = XWhitePixel(dpy, 0);
	wnd->pNtv = (void*)XCreateWindow(
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
	_UbwListAdd(wnd);
	return (Ubw)wnd;
}

#define _UBW_XWND (Window)((_Ubw*)wnd)->pNtv

int ubwGetTitle(Ubw wnd, char* str8) {
	//XGetWMName(dpy, _UBW_XWND, &xtpTitle);

	return 0;
}

int ubwSetTitle(Ubw wnd, char* str8) {
	if (!str8) {
		return 0;
	}
	XTextProperty xtpTitle;
	Status ok = XStringListToTextProperty(&str8, 1, &xtpTitle);
	if (!ok) {
		return 0;
	}
	XSetWMName(dpy, _UBW_XWND, &xtpTitle);
	XSetWMIconName(dpy, _UBW_XWND, &xtpTitle);
	return 1;
}

void ubwMove(Ubw wnd, int x, int y) {
	if (XMoveWindow(dpy, _UBW_XWND, x, y)) {
		((_Ubw*)wnd)->x = x;
		((_Ubw*)wnd)->y = y;
	}
}

void ubwMoveToScreenCenter(Ubw wnd) {
	ubwMove(wnd, (DisplayWidth(dpy, 0) - ((_Ubw*)wnd)->width) / 2, (DisplayHeight(dpy, 0) - ((_Ubw*)wnd)->height) / 2);
}

void ubwResize(Ubw wnd, int width, int height) {
	if (XResizeWindow(dpy, _UBW_XWND, width, height)) {
		((_Ubw*)wnd)->width = width;
		((_Ubw*)wnd)->height = width;
	}
}

void ubwShow(Ubw wnd) {
	XMapWindow(dpy, _UBW_XWND);
}

void ubwHide(Ubw wnd) {
	XUnmapWindow(dpy, _UBW_XWND);
}

#undef _UBW_XWND
#endif // UBWINDOW_X11
