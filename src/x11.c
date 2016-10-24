#include "plat.h"

#ifdef PWRE_PLAT_X11

#include "x11.h"
#include "uni.h"
#include "titlebuf_ex.h"

#include <limits.h>
#include <X11/Xutil.h>

#include <zk/map.h>
#include <zk/rwlock.h>

static zk_map_t wnd_map;
static zk_rwlock_t wnd_map_rwlock;
static zk_mutex_t evt_mux;

Display *_pwre_x11_dpy;
Window _pwre_x11_root;

static Atom net_wm_name;
static Atom utf8_str;
static Atom wm_del_wnd;
static Atom wm_protocols;

static Atom net_wm_state;
#define net_wm_state_remove 0
#define net_wm_state_add 1
#define net_wm_state_toggle 2
static Atom net_wm_state_hide;
static Atom net_wm_state_maxvert;
static Atom net_wm_state_maxhorz;
static Atom net_wm_state_fullscr;
static Atom motif_wm_hints;

bool pwre_init(pwre_event_handler_t evt_hdr) {
	XInitThreads();
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		puts("Pwre: X11.XOpenDisplay error!");
		return false;
	}
	root = XRootWindow(dpy, 0);
	net_wm_name = XInternAtom(dpy, "_NET_WM_NAME", False);
	utf8_str = XInternAtom(dpy, "UTF8_STRING", False);
	wm_del_wnd = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);

	net_wm_state =  XInternAtom(dpy, "_NET_WM_STATE", False);
	net_wm_state_hide =  XInternAtom(dpy, "_NET_WM_STATE_HIDDEN", False);
	net_wm_state_maxvert = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	net_wm_state_maxhorz = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	net_wm_state_fullscr = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

	motif_wm_hints = XInternAtom(dpy, "_MOTIF_WM_HINTS", True);

	wnd_count_mux = zk_new_mutex();
	event_handler = evt_hdr;
	wnd_map = zk_new_map(256);
	wnd_map_rwlock = zk_new_rwlock();
	evt_mux = zk_new_mutex();
	return true;
}

static void wnd_free(pwre_wnd_t wnd) {
	if (wnd->on_free) {
		wnd->on_free(wnd);
	}
	zk_mutex_lock(wnd->data_mux);
	if (wnd->title_buf) {
		free(wnd->title_buf);
	}
	zk_mutex_unlock(wnd->data_mux);
	zk_mutex_free(wnd->data_mux);
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

static bool handleXEvent(XEvent *event, bool mux) {
	if (mux) {
		zk_mutex_lock(evt_mux);
		XNextEvent(dpy, event);
		zk_mutex_unlock(evt_mux);
	} else {
		XNextEvent(dpy, event);
	}

	zk_rwlock_reading(wnd_map_rwlock);
	pwre_wnd_t wnd = (pwre_wnd_t)zk_map_get(wnd_map, ((XAnyEvent *)event)->window);
	zk_rwlock_red(wnd_map_rwlock);

	if (wnd) {
		switch (((XAnyEvent *)event)->type) {
			case ConfigureNotify:
				_EVENT_POST(
					pwre_size_t size;
					size.width = ((XConfigureEvent *)event)->width;
					size.height = ((XConfigureEvent *)event)->height;
					,
					PWRE_EVENT_SIZE, (void *)&size)
				break;
			case Expose:
				_EVENT_POST(, PWRE_EVENT_PAINT, NULL)
				break;
			case ClientMessage:
				if (((XClientMessageEvent *)event)->message_type == wm_protocols && (Atom)((XClientMessageEvent *)event)->data.l[0] == wm_del_wnd) {

					_EVENT_SEND(, PWRE_EVENT_CLOSE, NULL,
						return true;
					)

					zk_mutex_lock(evt_mux);
					XDestroyWindow(dpy, ((XAnyEvent *)event)->window);
					xSync(wnd->XWnd, DestroyNotify,)
					zk_mutex_unlock(evt_mux);

					return false;
				}
				break;
			case DestroyNotify:
				_EVENT_POST(, PWRE_EVENT_DESTROY, NULL)

				wnd_free(wnd);

				zk_rwlock_writing(wnd_map_rwlock);
				zk_map_delete(wnd_map, wnd->XWnd);
				zk_rwlock_written(wnd_map_rwlock);

				zk_mutex_lock(wnd_count_mux);
				wnd_count--;
				if (!wnd_count) {
					zk_mutex_unlock(wnd_count_mux);

					zk_mutex_free(wnd_count_mux);
					zk_map_free(wnd_map);
					zk_rwlock_free(wnd_map_rwlock);
					zk_mutex_free(evt_mux);

					XCloseDisplay(dpy);
					return false;
				}
				zk_mutex_unlock(wnd_count_mux);
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

pwre_wnd_t alloc_wnd(
	size_t struct_size,
	uint64_t hints,
	int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
) {
	Window XWnd = XCreateWindow(
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
	if (!XWnd) {
		puts("Pwre: X11.XCreateSimpleWindow error!");
		return NULL;
	}

	XSetWMProtocols(dpy, XWnd, &wm_del_wnd, 1);

	zk_mutex_lock(wnd_count_mux); wnd_count++; zk_mutex_unlock(wnd_count_mux);

	pwre_wnd_t wnd = calloc(1, struct_size);
	wnd->XWnd = XWnd;
	wnd->data_mux = zk_new_mutex();

	zk_rwlock_writing(wnd_map_rwlock);
	zk_map_set(wnd_map, XWnd, wnd);
	zk_rwlock_written(wnd_map_rwlock);
	return wnd;
}

pwre_wnd_t pwre_new_wnd(uint64_t hints) {
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
	return alloc_wnd(
		sizeof(struct pwre_wnd),
		hints,
		XDefaultDepth(dpy, 0), XDefaultVisual(dpy, 0), CWEventMask, &swa
	);
}

void pwre_wnd_close(pwre_wnd_t wnd) {
	if (event_handler && event_handler(wnd, PWRE_EVENT_CLOSE, NULL)) {
		XDestroyWindow(dpy, wnd->XWnd);
	}
}

void pwre_wnd_destroy(pwre_wnd_t wnd) {
	XDestroyWindow(dpy, wnd->XWnd);
}

const char *pwre_wnd_title(pwre_wnd_t wnd) {
	zk_mutex_lock(wnd->data_mux);
	Atom type;
	int format;
	unsigned long nitems, after;
	unsigned char *data;
	if (Success == XGetWindowProperty(dpy, wnd->XWnd, net_wm_name, 0, LONG_MAX, False, utf8_str, &type, &format, &nitems, &after, &data) && data) {
		wnd_title_buf_flush(wnd, (const char *)data);
		XFree(data);
	} else {
		wnd_title_buf_clear(wnd, 0);
	}
	zk_mutex_unlock(wnd->data_mux);
	return (const char *)wnd->title_buf;
}

void pwre_wnd_retitle(pwre_wnd_t wnd, const char *title) {
	XChangeProperty(dpy, wnd->XWnd, net_wm_name, utf8_str, 8, PropModeReplace, (const unsigned char *)title, strlen(title));
}

void pwre_wnd_move(pwre_wnd_t wnd, int x, int y) {
	XWindowAttributes wa;
	XGetWindowAttributes(dpy, wnd->XWnd, &wa);
	fix_pos(&x, &y, wa.width, wa.height);

	zk_mutex_lock(evt_mux);
	int err = XMoveWindow(dpy, wnd->XWnd, x, y);
	if (err != BadValue && err != BadWindow && err != BadMatch) {
		xSync(
			wnd->XWnd,
			ConfigureNotify,
			&& (
				((XConfigureEvent *)&event)->x != 0 ||
				((XConfigureEvent *)&event)->y != 0
			)
		)
	}
	zk_mutex_unlock(evt_mux);
}

void pwre_wnd_size(pwre_wnd_t wnd, int *width, int *height) {
	XWindowAttributes wa;
	XGetWindowAttributes(dpy, wnd->XWnd, &wa);
	if (width) {
		*width = wa.width;
	}
	if (height) {
		*height = wa.height;
	}
}

void pwre_wnd_resize(pwre_wnd_t wnd, int width, int height) {
	zk_mutex_lock(evt_mux);
	int err = XResizeWindow(dpy, wnd->XWnd, width, height);
	if (err != BadValue && err != BadWindow) {
		xSync(
			wnd->XWnd,
			ConfigureNotify,
			&& ((XConfigureEvent *)&event)->width == width
			&& ((XConfigureEvent *)&event)->height == height
		)
	}
	zk_mutex_unlock(evt_mux);
}

static void visible(pwre_wnd_t wnd) {
	XWindowAttributes wa;
	XGetWindowAttributes(dpy, wnd->XWnd, &wa);
	zk_mutex_lock(evt_mux);
	if (wa.map_state != IsViewable && XMapWindow(dpy, wnd->XWnd) != BadWindow && wa.map_state == IsUnmapped) {
		xSync(
			wnd->XWnd,
			MapNotify,
		)
	}
	zk_mutex_unlock(evt_mux);
}

void pwre_wnd_state_add(pwre_wnd_t wnd, PWRE_STATE type) {
	XEvent event;
	switch (type) {
		case PWRE_STATE_VISIBLE:
			visible(wnd);
			break;
		case PWRE_STATE_MINIMIZE:
			visible(wnd);
			XIconifyWindow(dpy, wnd->XWnd, 0);
			break;
		case PWRE_STATE_MAXIMIZE:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = wnd->XWnd;
			event.xclient.message_type = net_wm_state;
			event.xclient.format = 32;
			event.xclient.data.l[0] = net_wm_state_add;
			event.xclient.data.l[1] = net_wm_state_maxvert;
			event.xclient.data.l[2] = net_wm_state_maxhorz;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
			break;
		case PWRE_STATE_FULLSCREEN:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = wnd->XWnd;
			event.xclient.message_type = net_wm_state;
			event.xclient.format = 32;
			event.xclient.data.l[0] = net_wm_state_add;
			event.xclient.data.l[1] = net_wm_state_fullscr;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
	}
	return;
}

void pwre_wnd_state_rm(pwre_wnd_t wnd, PWRE_STATE type) {
	XEvent event;
	switch (type) {
		case PWRE_STATE_VISIBLE:
			visible(wnd);
			break;
		case PWRE_STATE_MINIMIZE:
			visible(wnd);
			break;
		case PWRE_STATE_MAXIMIZE:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = wnd->XWnd;
			event.xclient.message_type = net_wm_state;
			event.xclient.format = 32;
			event.xclient.data.l[0] = net_wm_state_remove;
			event.xclient.data.l[1] = net_wm_state_maxvert;
			event.xclient.data.l[2] = net_wm_state_maxhorz;
			XSendEvent(dpy, root, False, StructureNotifyMask, &event);
			break;
		case PWRE_STATE_FULLSCREEN:
			visible(wnd);
			memset(&event, 0, sizeof(event));
			event.type = ClientMessage;
			event.xclient.window = wnd->XWnd;
			event.xclient.message_type = net_wm_state;
			event.xclient.format = 32;
			event.xclient.data.l[0] = net_wm_state_remove;
			event.xclient.data.l[1] = net_wm_state_fullscr;
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

void pwre_wnd_less(pwre_wnd_t wnd, bool less) {
	PropMotifWmHints motif_hints;
	motif_hints.flags = MWM_HINTS_DECORATIONS;
	motif_hints.decorations = 0;
	XChangeProperty(dpy, wnd->XWnd, motif_wm_hints, motif_wm_hints, 32, PropModeReplace, (unsigned char *) &motif_hints, PROP_MOTIF_WM_HINTS_ELEMENTS);
}

#endif // PWRE_PLAT_X11
