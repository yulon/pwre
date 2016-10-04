#include "pdbe.h"

#ifdef UBWINDOW_X11

#include "ubw.h"
#include "modmap.h"
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static ModMap wndMap;

static Display *dpy;
static Window root;

Atom netWmName;
Atom utf8str;
Atom wmDelWnd;
Atom wmProtocols;

Atom netWmState;
#define netWmStateRemove 0
#define netWmStateAdd 1
#define netWmStateToggle 2
Atom netWmStateHide;
Atom netWmStateMaxVert;
Atom netWmStateMaxHorz;
Atom netWmStateFullscreen;

bool ubwInit(UbwEventHandler evtHdr) {
	XInitThreads();
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		puts("UBWindow: X11.XOpenDisplay error!");
		return false;
	}
	root = XRootWindow(dpy, 0);
	netWmName = XInternAtom(dpy, "_NET_WM_NAME", False);
	utf8str = XInternAtom(dpy, "UTF8_STRING", False);
	wmDelWnd = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmProtocols = XInternAtom(dpy, "WM_PROTOCOLS", False);

	netWmState =  XInternAtom(dpy, "_NET_WM_STATE", False);
	netWmStateHide =  XInternAtom(dpy, "_NET_WM_STATE_HIDDEN", False);
	netWmStateMaxVert = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	netWmStateMaxHorz = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	netWmStateFullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

	dftEvtHdr = evtHdr;
	wndMap = new_ModMap(256);
	return true;
}

static bool handleXEvent(XEvent *event) {
	XNextEvent(dpy, event);
	_EVT_VARS(ModMap_get(wndMap, (void *)((XAnyEvent *)event)->window))
	if (wnd) {

		switch (((XAnyEvent *)event)->type) {
			case ConfigureNotify:
				_EVT_POST(
					size.width = ((XConfigureEvent *)event)->width;
					size.height = ((XConfigureEvent *)event)->height;
					,
					UBW_EVENT_SIZE, (void *)&size)
				break;
			case Expose:
				_EVT_POST(, UBW_EVENT_PAINT, NULL)
				break;
			case ClientMessage:
				if (((XClientMessageEvent *)event)->message_type == wmProtocols && (Atom)((XClientMessageEvent *)event)->data.l[0] == wmDelWnd) {
					_EVT_SEND(, UBW_EVENT_CLOSE, NULL,
						return true;
					)
					XDestroyWindow(dpy, ((XAnyEvent *)event)->window);
				}
				break;
			case DestroyNotify:
				_EVT_POST(, UBW_EVENT_DESTROY, NULL)
				wndCount--;
				if (!wndCount) {
					XCloseDisplay(dpy);
					return false;
				}
				ModMap_delete(wndMap, wnd->ntvPtr);
				free(wnd);
		}
	}
	return true;
}

bool ubwStep(void) {
	XEvent event;
	if (XPending(dpy)) {
		return handleXEvent(&event);
	}
	return true;
}

void ubwRun(void) {
	XEvent event;
	while (handleXEvent(&event));
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

	XSetWMProtocols(dpy, xWnd, &wmDelWnd, 1);
	XSelectInput(dpy, xWnd, ExposureMask | KeyPressMask | StructureNotifyMask);

	wndCount++;
	_UbwPvt wnd = calloc(1, sizeof(struct _UbwPvt));
	wnd->ntvPtr = (void *)xWnd;
	wnd->evtHdr = dftEvtHdr;

 	ModMap_set(wndMap, (void *)xWnd, (void *)wnd);
	return (Ubw)wnd;
}

#define _XWND (Window)((_UbwPvt)wnd)->ntvPtr

void ubwClose(Ubw wnd) {
	if (((_UbwPvt)wnd)->evtHdr && !(*((_UbwPvt)wnd)->evtHdr)(wnd, UBW_EVENT_CLOSE, NULL)) {
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
	int err = XMoveWindow(dpy, _XWND, x, y);
	if (err != BadValue && err != BadWindow && err != BadMatch) {
		XEvent event;
		while (handleXEvent(&event)) {
			if (
				((XAnyEvent *)&event)->window == _XWND &&
				((XAnyEvent *)&event)->type == ConfigureNotify &&
				(
					((XConfigureEvent *)&event)->x != 0 ||
					((XConfigureEvent *)&event)->y != 0
				)
			) {
				return;
			}
		}
	}
}

void ubwMoveToScreenCenter(Ubw wnd) {
	int width, height;
	ubwSize(wnd, &width, &height);
	ubwMove(wnd, (DisplayWidth(dpy, 0) - width) / 2, (DisplayHeight(dpy, 0) - height) / 2);
}

void ubwSize(Ubw wnd, int *width, int *height) {
	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, _XWND, &attrs);
	if (width) {
		*width = attrs.width;
	}
	if (height) {
		*height = attrs.height;
	}
}

void ubwResize(Ubw wnd, int width, int height) {
	int err = XResizeWindow(dpy, _XWND, width, height);
	if (err != BadValue && err != BadWindow) {
		XEvent event;
		while (handleXEvent(&event)) {
			if (
				((XAnyEvent *)&event)->window == _XWND &&
				((XAnyEvent *)&event)->type == ConfigureNotify &&
				((XConfigureEvent *)&event)->width == width &&
				((XConfigureEvent *)&event)->height == height
			) {
				return;
			}
		}
	}
}

static void visible(Ubw wnd) {
	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, _XWND, &attrs);
	XEvent event;
	if (attrs.map_state != IsViewable && XMapWindow(dpy, _XWND) != BadWindow && attrs.map_state == IsUnmapped) {
		while (handleXEvent(&event)) {
			if (
				((XAnyEvent *)&event)->window == _XWND &&
				((XAnyEvent *)&event)->type == MapNotify
			) {
				break;
			}
		}
	}
}

void ubwView(Ubw wnd, int type) {
	XEvent event;
	switch (type) {
		case UBW_VIEW_VISIBLE:
			visible(wnd);
			break;
		case UBW_VIEW_MINIMIZE:
			visible(wnd);
			XIconifyWindow(dpy, _XWND, 0);
			break;
		case UBW_VIEW_MAXIMIZE:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = _XWND;
			event.xclient.message_type = netWmState;
			event.xclient.format = 32;
			event.xclient.data.l[0] = netWmStateAdd;
			event.xclient.data.l[1] = netWmStateMaxVert;
			event.xclient.data.l[2] = netWmStateMaxHorz;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
			break;
		case UBW_VIEW_FULLSCREEN:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = _XWND;
			event.xclient.message_type = netWmState;
			event.xclient.format = 32;
			event.xclient.data.l[0] = netWmStateAdd;
			event.xclient.data.l[1] = netWmStateFullscreen;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
	}
	return;
}

void ubwUnview(Ubw wnd, int type) {
	XEvent event;
	switch (type) {
		case UBW_VIEW_VISIBLE:
			visible(wnd);
			break;
		case UBW_VIEW_MINIMIZE:
			visible(wnd);
			break;
		case UBW_VIEW_MAXIMIZE:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = _XWND;
			event.xclient.message_type = netWmState;
			event.xclient.format = 32;
			event.xclient.data.l[0] = netWmStateRemove;
			event.xclient.data.l[1] = netWmStateMaxVert;
			event.xclient.data.l[2] = netWmStateMaxHorz;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
			break;
		case UBW_VIEW_FULLSCREEN:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = _XWND;
			event.xclient.message_type = netWmState;
			event.xclient.format = 32;
			event.xclient.data.l[0] = netWmStateRemove;
			event.xclient.data.l[1] = netWmStateFullscreen;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
	}
}

#undef _XWND
#endif // UBWINDOW_X11
