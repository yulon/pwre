#include "ubw.h"

#ifdef UBWINDOW_X11

#include <X11/Xlib.h>

static Display* dpy;
static Window root;

int ubwInit() {
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		puts("UBWindow: X11.XOpenDisplay error!");
		return NULL;
	}
	root = XRootWindow(dpy, 0);
}

static XClientMessageEvent event;

int ubwHandleEvent() {
	XNextEvent(dpy, (XEvent*)&event);
	if (event.type == ClientMessage)
	{
		XCloseDisplay(dpy);
		return 0;
	}
	return 1;
}

Ubw ubwCreate() {
	_Ubw* wnd = calloc(1, sizeof(_Ubw));

	wnd->pNtv = (void*)XCreateSimpleWindow(
		dpy, RootWindow(dpy, 0), 10, 10, 500, 500, 1,
		BlackPixel(dpy, 0), WhitePixel(dpy, 0)
	);
	if (!wnd->pNtv) {
		puts("UBWindow: X11.XCreateSimpleWindow error!");
		return NULL;
	}

	Atom wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(dpy, (Window)wnd->pNtv, &wmDelete, 1);
	XSelectInput(dpy, (Window)wnd->pNtv, ExposureMask | KeyPressMask);
	XMapWindow(dpy, (Window)wnd->pNtv);

	ubwSum++;
	return (Ubw)wnd;
}

int ubwGetTitle(Ubw wnd, char* str8) {

	return 0;
}

int ubwSetTitle(Ubw wnd, char* str8) {

	return 0;
}

void ubwGetRect(Ubw wnd, UbwRect* pRect) {

}

void ubwSetRect(Ubw wnd, UbwRect rect) {

}

void ubwMoveToScreenCenter(Ubw wnd) {

}

void ubwActive(Ubw wnd) {

}

void ubwVisible(Ubw wnd) {

}

#endif // UBWINDOW_X11
