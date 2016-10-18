#include "plat.h"

#ifdef PWRE_X11

#define ZK_SCOPE pwre
#define ZK_IMPL

#include "x11.h"
#include "uni.h"
#include "titlebuf.h"

#include <limits.h>
#include <X11/Xutil.h>

#include <zk/map.h>

static ZKMap wndMap;
static ZKMux wndMapAndEvtMux;

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
Atom motifWmHints;

bool pwre_init(PrEventHandler evtHdr) {
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

	motifWmHints = XInternAtom(dpy, "_MOTIF_WM_HINTS", True);

	wndCountMux = new_ZKMux();
	eventHandler = evtHdr;
	wndMap = new_ZKMap(256);
	wndMapAndEvtMux = new_ZKMux();
	return true;
}

static void _PrWnd_free(PrWnd wnd) {
	if (wnd->onFree) {
		wnd->onFree(wnd);
	}
	ZKMux_Lock(wnd->dataMux);
	if (wnd->titleBuf) {
		free(wnd->titleBuf);
	}
	ZKMux_UnLock(wnd->dataMux);
	ZKMux_Free(wnd->dataMux);
	free(wnd);
}

#define xSync(_wnd, _event, _conds) { \
	XEvent event; \
	while (handleXEvent(&event, false)) { \
		if ( \
			((XAnyEvent *)&event)->window == _wnd && \
			((XAnyEvent *)&event)->type == _event \
			_conds \
		) { \
			break; \
		} \
	} \
}

static bool handleXEvent(XEvent *event, bool lock) {
	PrWnd wnd;

	if (lock) {
		ZKMux_Lock(wndMapAndEvtMux);
		XNextEvent(dpy, event);
		wnd = (PrWnd)ZKMap_Get(wndMap, ((XAnyEvent *)event)->window);
		ZKMux_UnLock(wndMapAndEvtMux);
	} else {
		XNextEvent(dpy, event);
		wnd = (PrWnd)ZKMap_Get(wndMap, ((XAnyEvent *)event)->window);
	}

	if (wnd) {
		switch (((XAnyEvent *)event)->type) {
			case ConfigureNotify:
				eventPost(
					PrSize size;
					size.width = ((XConfigureEvent *)event)->width;
					size.height = ((XConfigureEvent *)event)->height;
					,
					PWRE_EVENT_SIZE, (void *)&size)
				break;
			case Expose:
				eventPost(, PWRE_EVENT_PAINT, NULL)
				break;
			case ClientMessage:
				if (((XClientMessageEvent *)event)->message_type == wmProtocols && (Atom)((XClientMessageEvent *)event)->data.l[0] == wmDelWnd) {
					eventSend(, PWRE_EVENT_CLOSE, NULL,
						return true;
					)

					ZKMux_Lock(wndMapAndEvtMux);
					XDestroyWindow(dpy, ((XAnyEvent *)event)->window);
					xSync(wnd->xWnd, DestroyNotify,)
					ZKMux_UnLock(wndMapAndEvtMux);

					return false;
				}
				break;
			case DestroyNotify:
				eventPost(, PWRE_EVENT_DESTROY, NULL)

				ZKMux_Lock(wndMapAndEvtMux);
				ZKMap_Delete(wndMap, wnd->xWnd);
				ZKMux_UnLock(wndMapAndEvtMux);

				ZKMux_Lock(wndCountMux);
				wndCount--;
				_PrWnd_free(wnd);
				if (!wndCount) {
					ZKMux_UnLock(wndCountMux);
					ZKMux_Free(wndCountMux);
					ZKMux_Free(wndMapAndEvtMux);
					XCloseDisplay(dpy);
					return false;
				}
				ZKMux_UnLock(wndCountMux);
		}
	}
	return true;
}

bool pwre_step(void) {
	XEvent event;
	while (XPending(dpy)) {
		if (!handleXEvent(&event, true)) {
			return false;
		}
	}
	return true;
}

void pwre_run(void) {
	XEvent event;
	while (handleXEvent(&event, true));
}

static void fixPos(int *x, int *y, int width, int height) {
	if (*x == PWRE_POS_AUTO) {
		*x = (DisplayWidth(dpy, 0) - width) / 2;
	}
	if (*y == PWRE_POS_AUTO) {
		*y = (DisplayHeight(dpy, 0) - height) / 2;
	}
}

PrWnd _alloc_PrWnd(
	size_t memSize,
	uint64_t hints,
	int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
) {
	Window xWnd = XCreateWindow(
		dpy,
		root,
		0,
		0,
		150,
		150,
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

	ZKMux_Lock(wndCountMux); wndCount++; ZKMux_UnLock(wndCountMux);

	PrWnd wnd = calloc(1, memSize);
	wnd->xWnd = xWnd;

 	ZKMux_Lock(wndMapAndEvtMux);
	ZKMap_Set(wndMap, xWnd, wnd);
	ZKMux_UnLock(wndMapAndEvtMux);
	return wnd;
}

PrWnd new_PrWnd(uint64_t hints) {
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
	return _alloc_PrWnd(
		sizeof(struct PrWnd),
		hints,
		XDefaultDepth(dpy, 0), XDefaultVisual(dpy, 0), CWEventMask, &swa
	);
}

void PrWnd_Close(PrWnd wnd) {
	if (eventHandler && eventHandler(wnd, PWRE_EVENT_CLOSE, NULL)) {
		XDestroyWindow(dpy, wnd->xWnd);
	}
}

void PrWnd_Destroy(PrWnd wnd) {
	XDestroyWindow(dpy, wnd->xWnd);
}

const char *PrWnd_GetTitle(PrWnd wnd) {
	ZKMux_Lock(wnd->dataMux);
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
	ZKMux_UnLock(wnd->dataMux);
	return (const char *)wnd->titleBuf;
}

void PrWnd_SetTitle(PrWnd wnd, const char *title) {
	XChangeProperty(dpy, wnd->xWnd, netWmName, utf8str, 8, PropModeReplace, (const unsigned char *)title, strlen(title));
}

void PrWnd_Move(PrWnd wnd, int x, int y) {
	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, wnd->xWnd, &attrs);
	fixPos(&x, &y, attrs.width, attrs.height);

	ZKMux_Lock(wndMapAndEvtMux);
	int err = XMoveWindow(dpy, wnd->xWnd, x, y);
	if (err != BadValue && err != BadWindow && err != BadMatch) {
		xSync(
			wnd->xWnd,
			ConfigureNotify,
			&& (
				((XConfigureEvent *)&event)->x != 0 ||
				((XConfigureEvent *)&event)->y != 0
			)
		)
	}
	ZKMux_UnLock(wndMapAndEvtMux);
}

void PrWnd_Size(PrWnd wnd, int *width, int *height) {
	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, wnd->xWnd, &attrs);
	if (width) {
		*width = attrs.width;
	}
	if (height) {
		*height = attrs.height;
	}
}

void PrWnd_ReSize(PrWnd wnd, int width, int height) {
	ZKMux_Lock(wndMapAndEvtMux);
	int err = XResizeWindow(dpy, wnd->xWnd, width, height);
	if (err != BadValue && err != BadWindow) {
		xSync(
			wnd->xWnd,
			ConfigureNotify,
			&& ((XConfigureEvent *)&event)->width == width
			&& ((XConfigureEvent *)&event)->height == height
		)
	}
	ZKMux_UnLock(wndMapAndEvtMux);
}

static void visible(PrWnd wnd) {
	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, wnd->xWnd, &attrs);
	ZKMux_Lock(wndMapAndEvtMux);
	if (attrs.map_state != IsViewable && XMapWindow(dpy, wnd->xWnd) != BadWindow && attrs.map_state == IsUnmapped) {
		xSync(
			wnd->xWnd,
			MapNotify,
		)
	}
	ZKMux_UnLock(wndMapAndEvtMux);
}

void PrWnd_View(PrWnd wnd, PWRE_VIEW type) {
	XEvent event;
	switch (type) {
		case PWRE_VIEW_VISIBLE:
			visible(wnd);
			break;
		case PWRE_VIEW_MINIMIZE:
			visible(wnd);
			XIconifyWindow(dpy, wnd->xWnd, 0);
			break;
		case PWRE_VIEW_MAXIMIZE:
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
		case PWRE_VIEW_FULLSCREEN:
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

void PrWnd_UnView(PrWnd wnd, PWRE_VIEW type) {
	XEvent event;
	switch (type) {
		case PWRE_VIEW_VISIBLE:
			visible(wnd);
			break;
		case PWRE_VIEW_MINIMIZE:
			visible(wnd);
			break;
		case PWRE_VIEW_MAXIMIZE:
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
		case PWRE_VIEW_FULLSCREEN:
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

static const unsigned MWM_HINTS_DECORATIONS = (1 << 1);
static const int PROP_MOTIF_WM_HINTS_ELEMENTS = 5;

typedef struct {
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long inputMode;
	unsigned long status;
} PropMotifWmHints;

void PrWnd_Less(PrWnd wnd, bool less) {
	PropMotifWmHints motif_hints;
	motif_hints.flags = MWM_HINTS_DECORATIONS;
	motif_hints.decorations = 0;
	XChangeProperty(dpy, wnd->xWnd, motifWmHints, motifWmHints, 32, PropModeReplace, (unsigned char *) &motif_hints, PROP_MOTIF_WM_HINTS_ELEMENTS);
}

#endif // PWRE_X11
