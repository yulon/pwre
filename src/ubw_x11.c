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
		return NULL;
	}
	root = XRootWindow(dpy, 0);
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
				_UbwList[ixWnd]->rect.left = ((XConfigureEvent*)event)->x;
				_UbwList[ixWnd]->rect.top = ((XConfigureEvent*)event)->y;
			}
			_UbwList[ixWnd]->rect.width = ((XConfigureEvent*)event)->width;
			_UbwList[ixWnd]->rect.height = ((XConfigureEvent*)event)->height;
			printf("ConfigureNotify %d %d %d %d\n", ((XConfigureEvent*)event)->x, ((XConfigureEvent*)event)->y, ((XConfigureEvent*)event)->width, ((XConfigureEvent*)event)->height);
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
		100,
		100,
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

	wnd->rect.width = 10;
	wnd->rect.height = 10;

	wnd->szNonCont.width = 10;
	wnd->szNonCont.height = 50;

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

static Window get_toplevel_parent(Display * display, Window window)
{
	Window parent;
	Window root;
	Window * children;
	unsigned int num_children;

	while (1) {
		if (0 == XQueryTree(display, window, &root,
			&parent, &children, &num_children)) {
			fprintf(stderr, "XQueryTree error\n");
			abort(); //change to whatever error handling you prefer
		}
		if (children) { //must test for null
			XFree(children);
		}
		if (window == root || parent == root) {
			return window;
		}
		else {
			window = parent;
		}
	}
}

void ubwGetRect(Ubw wnd, UbwRect* pRect) {
	Window toplevel_parent_of_focus;
	XWindowAttributes attr;
	toplevel_parent_of_focus = get_toplevel_parent(dpy, _UBW_XWND);
	XGetWindowAttributes(dpy, toplevel_parent_of_focus, &attr);
	printf("XGetWindowAttributes %d %d %d %d\n", attr.x, attr.y, attr.width, attr.height);
	//pRect->left = attr.x;
	//pRect->top = attr.y;
	//pRect->width = attr.width;
	//pRect->height = attr.height;

	pRect->left = ((_Ubw*)wnd)->rect.left;
	pRect->top = ((_Ubw*)wnd)->rect.top;
	pRect->width = ((_Ubw*)wnd)->rect.width;
	pRect->height = ((_Ubw*)wnd)->rect.height;
}

void ubwSetRect(Ubw wnd, UbwRect rect) {
	if (XMoveResizeWindow(dpy, _UBW_XWND, rect.left, rect.top, rect.width, rect.height)) {
		((_Ubw*)wnd)->rect.left = rect.left;
		((_Ubw*)wnd)->rect.top = rect.top;
		((_Ubw*)wnd)->rect.width = rect.width;
		((_Ubw*)wnd)->rect.height = rect.width;
	}
}

void ubwMove(Ubw wnd, UbwPoint point) {
	if (XMoveWindow(dpy, _UBW_XWND, point.left, point.top)) {
		((_Ubw*)wnd)->rect.left = point.left;
		((_Ubw*)wnd)->rect.top = point.top;
	}
}

void ubwMoveToScreenCenter(Ubw wnd) {
	UbwRect rect = {0, 0, 10, 10};
	ubwGetRect(wnd, &rect);
	rect.left = (DisplayWidth(dpy, 0) - rect.width) / 2;
	rect.top = (DisplayHeight(dpy, 0) - rect.height) / 2;
	ubwSetRect(wnd, rect);
}

void ubwResize(Ubw wnd, UbwSize size) {
	if (XResizeWindow(dpy, _UBW_XWND, size.width, size.height)) {
		((_Ubw*)wnd)->rect.width = size.width;
		((_Ubw*)wnd)->rect.height = size.width;
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
