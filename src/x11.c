#include "plat.h"

#ifdef PWRE_X11

#include "x11.h"
#include "uni.h"
#include "titlebuf.h"

#include "modmap.h"
#include <limits.h>
#include <X11/Xutil.h>

static ModMap wndMap;
static Mutex wndMapMux;

Display *_pwre_x11_dpy;
Window _pwre_x11_root;

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

bool pwreInit(PrEventHandler evtHdr) {
	XInitThreads();
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		puts("Pwre: X11.XOpenDisplay error!");
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

	wndCountMux = new_Mutex();
	dftEvtHdr = evtHdr;
	wndMap = new_ModMap(256);
	wndMapMux = new_Mutex();
	return true;
}

static void _PrWnd_free(PrWnd wnd) {
	if (wnd->onFree) {
		wnd->onFree(wnd);
	}
	Mutex_lock(wnd->dataMux);
	if (wnd->titleBuf) {
		free(wnd->titleBuf);
	}
	Mutex_unlock(wnd->dataMux);
	Mutex_free(wnd->dataMux);
	free(wnd);
}

static bool handleXEvent(XEvent *event) {
	XNextEvent(dpy, event);

	Mutex_lock(wndMapMux);
	eventTarget((PrWnd)ModMap_get(wndMap, ((XAnyEvent *)event)->window))
	Mutex_unlock(wndMapMux);
	if (wnd) {
		switch (((XAnyEvent *)event)->type) {
			case ConfigureNotify:
				eventPost(
					PrSize size;
					size.width = ((XConfigureEvent *)event)->width;
					size.height = ((XConfigureEvent *)event)->height;
					,
					PrEvent_Size, (void *)&size)
				break;
			case Expose:
				eventPost(, PrEvent_Paint, NULL)
				break;
			case ClientMessage:
				if (((XClientMessageEvent *)event)->message_type == wmProtocols && (Atom)((XClientMessageEvent *)event)->data.l[0] == wmDelWnd) {
					eventSend(, PrEvent_Close, NULL,
						return true;
					)

					XDestroyWindow(dpy, ((XAnyEvent *)event)->window);
					while (handleXEvent(event)) {
						if (
							((XAnyEvent *)event)->window == wnd->xWnd &&
							((XAnyEvent *)event)->type == DestroyNotify
						) {
							return true;
						}
					}
					return false;
				}
				break;
			case DestroyNotify:
				eventPost(, PrEvent_Destroy, NULL)

				Mutex_lock(wndMapMux);
				ModMap_delete(wndMap, wnd->xWnd);
				Mutex_unlock(wndMapMux);

				Mutex_lock(wndCountMux);
				wndCount--;
				_PrWnd_free(wnd);
				if (!wndCount) {
					Mutex_unlock(wndCountMux);
					Mutex_free(wndCountMux);
					Mutex_free(wndMapMux);
					XCloseDisplay(dpy);
					return false;
				}
				Mutex_unlock(wndCountMux);
		}
	}
	return true;
}

bool pwreStep(void) {
	XEvent event;
	if (XPending(dpy)) {
		return handleXEvent(&event);
	}
	return true;
}

void pwreRun(void) {
	XEvent event;
	while (handleXEvent(&event));
}

static void fixPos(int *x, int *y, int width, int height) {
	if (*x == PrPos_ScreenCenter) {
		*x = (DisplayWidth(dpy, 0) - width) / 2;
	}
	if (*y == PrPos_ScreenCenter) {
		*y = (DisplayHeight(dpy, 0) - height) / 2;
	}
}

PrWnd _alloc_PrWnd(
	size_t size,
	int x, int y, int width, int height,
	int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
) {
	fixPos(&x, &y, width, height);

	Window xWnd = XCreateWindow(
		dpy,
		root,
		x,
		y,
		width,
		height,
		0,
		depth,
		InputOutput,
		visual,
		valuemask,
		swa
	);
	if (!xWnd) {
		puts("Pwre: X11.XCreateSimpleWindow error!");
		return NULL;
	}

	XSetWMProtocols(dpy, xWnd, &wmDelWnd, 1);

	Mutex_lock(wndCountMux); wndCount++; Mutex_unlock(wndCountMux);

	PrWnd wnd = calloc(1, size);
	wnd->xWnd = xWnd;
	wnd->evtHdr = dftEvtHdr;

 	Mutex_lock(wndMapMux);
	ModMap_set(wndMap, xWnd, wnd);
	Mutex_unlock(wndMapMux);
	return wnd;
}

PrWnd new_PrWnd(int x, int y, int width, int height) {
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
	return _alloc_PrWnd(
		sizeof(struct PrWnd),
		x, y, width, height,
		XDefaultDepth(dpy, 0), XDefaultVisual(dpy, 0), CWEventMask, &swa
	);
}

void PrWnd_close(PrWnd wnd) {
	if (wnd->evtHdr && !wnd->evtHdr(wnd, PrEvent_Close, NULL)) {
		XDestroyWindow(dpy, wnd->xWnd);
	}
}

void PrWnd_destroy(PrWnd wnd) {
	XDestroyWindow(dpy, wnd->xWnd);
}

const char *PrWnd_getTitle(PrWnd wnd) {
	Mutex_lock(wnd->dataMux);
	Atom type;
	int format;
	unsigned long nitems, after;
	unsigned char *data;
	if (Success == XGetWindowProperty(dpy, wnd->xWnd, netWmName, 0, LONG_MAX, False, utf8str, &type, &format, &nitems, &after, &data) && data) {
		_PrWnd_flushTitleBuf(wnd, (const char *)data);
		XFree(data);
	} else {
		_PrWnd_clearTitleBuf(wnd, 0);
	}
	Mutex_unlock(wnd->dataMux);
	return (const char *)wnd->titleBuf;
}

void PrWnd_setTitle(PrWnd wnd, const char *title) {
	XChangeProperty(dpy, wnd->xWnd, netWmName, utf8str, 8, PropModeReplace, (const unsigned char *)title, strlen(title));
}

void PrWnd_move(PrWnd wnd, int x, int y) {
	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, wnd->xWnd, &attrs);
	fixPos(&x, &y, attrs.width, attrs.height);

	int err = XMoveWindow(dpy, wnd->xWnd, x, y);
	if (err != BadValue && err != BadWindow && err != BadMatch) {
		XEvent event;
		while (handleXEvent(&event)) {
			if (
				((XAnyEvent *)&event)->window == wnd->xWnd &&
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

void PrWnd_size(PrWnd wnd, int *width, int *height) {
	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, wnd->xWnd, &attrs);
	if (width) {
		*width = attrs.width;
	}
	if (height) {
		*height = attrs.height;
	}
}

void PrWnd_resize(PrWnd wnd, int width, int height) {
	int err = XResizeWindow(dpy, wnd->xWnd, width, height);
	if (err != BadValue && err != BadWindow) {
		XEvent event;
		while (handleXEvent(&event)) {
			if (
				((XAnyEvent *)&event)->window == wnd->xWnd &&
				((XAnyEvent *)&event)->type == ConfigureNotify &&
				((XConfigureEvent *)&event)->width == width &&
				((XConfigureEvent *)&event)->height == height
			) {
				return;
			}
		}
	}
}

static void visible(PrWnd wnd) {
	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, wnd->xWnd, &attrs);
	XEvent event;
	if (attrs.map_state != IsViewable && XMapWindow(dpy, wnd->xWnd) != BadWindow && attrs.map_state == IsUnmapped) {
		while (handleXEvent(&event)) {
			if (
				((XAnyEvent *)&event)->window == wnd->xWnd &&
				((XAnyEvent *)&event)->type == MapNotify
			) {
				break;
			}
		}
	}
}

void PrWnd_view(PrWnd wnd, PrView type) {
	XEvent event;
	switch (type) {
		case PrView_Visible:
			visible(wnd);
			break;
		case PrView_Minimize:
			visible(wnd);
			XIconifyWindow(dpy, wnd->xWnd, 0);
			break;
		case PrView_Maximize:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = wnd->xWnd;
			event.xclient.message_type = netWmState;
			event.xclient.format = 32;
			event.xclient.data.l[0] = netWmStateAdd;
			event.xclient.data.l[1] = netWmStateMaxVert;
			event.xclient.data.l[2] = netWmStateMaxHorz;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
			break;
		case PrView_Fullscreen:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = wnd->xWnd;
			event.xclient.message_type = netWmState;
			event.xclient.format = 32;
			event.xclient.data.l[0] = netWmStateAdd;
			event.xclient.data.l[1] = netWmStateFullscreen;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
	}
	return;
}

void PrWnd_unview(PrWnd wnd, PrView type) {
	XEvent event;
	switch (type) {
		case PrView_Visible:
			visible(wnd);
			break;
		case PrView_Minimize:
			visible(wnd);
			break;
		case PrView_Maximize:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = wnd->xWnd;
			event.xclient.message_type = netWmState;
			event.xclient.format = 32;
			event.xclient.data.l[0] = netWmStateRemove;
			event.xclient.data.l[1] = netWmStateMaxVert;
			event.xclient.data.l[2] = netWmStateMaxHorz;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
			break;
		case PrView_Fullscreen:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = wnd->xWnd;
			event.xclient.message_type = netWmState;
			event.xclient.format = 32;
			event.xclient.data.l[0] = netWmStateRemove;
			event.xclient.data.l[1] = netWmStateFullscreen;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
	}
}

#endif // PWRE_X11
