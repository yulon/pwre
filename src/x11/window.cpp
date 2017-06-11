#include <pwre.hpp>

#ifdef PWRE_PLAT_X11

#include "window.hpp"

#include <X11/Xutil.h>

#include <unordered_map>
#include <mutex>

#include <climits>
#include <cstring>

#include <cassert>
#include "../uassert.h"

namespace pwre {
	#define _EVENT_MASK (ExposureMask | KeyPressMask | StructureNotifyMask)

	Atom _NET_WM_NAME;
	Atom UTF8_STRING;
	Atom WM_DELETE_WINDOW;
	Atom WM_PROTOCOLS;

	Atom _NET_WM_STATE;
	#define _NET_WM_STATE_REMOVE 0
	#define _NET_WM_STATE_ADD 1
	#define _NET_WM_STATE_TOGGLE 2
	Atom _NET_WM_STATE_HIDDEN;
	Atom _NET_WM_STATE_MAXIMIZED_VERT;
	Atom _NET_WM_STATE_MAXIMIZED_HORZ;
	Atom _NET_WM_STATE_FULLSCREEN;
	Atom _MOTIF_WM_HINTS;

	std::unordered_map<Window, window *> map;
	bool life = true;

	Display *dpy;
	Window root;

	void init_x11() {
		dpy = XOpenDisplay(NULL);
		uassert(dpy, "Pwre", "XOpenDisplay");

		root = XRootWindow(dpy, 0);
		_NET_WM_NAME = XInternAtom(dpy, "_NET_WM_NAME", False);
		UTF8_STRING = XInternAtom(dpy, "UTF8_STRING", False);
		WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
		WM_PROTOCOLS = XInternAtom(dpy, "WM_PROTOCOLS", False);

		_NET_WM_STATE =  XInternAtom(dpy, "_NET_WM_STATE", False);
		_NET_WM_STATE_HIDDEN =  XInternAtom(dpy, "_NET_WM_STATE_HIDDEN", False);
		_NET_WM_STATE_MAXIMIZED_VERT = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		_NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		_NET_WM_STATE_FULLSCREEN = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

		_MOTIF_WM_HINTS = XInternAtom(dpy, "_MOTIF_WM_HINTS", True);
	}

	void sync_event_to(window &wnd, long event_mask, int type) {
		XEvent event;
		while (life && wnd.available()) {
			XWindowEvent(dpy, wnd.native_handle(), event_mask, &event);
			handle_event(wnd, event);
			if (event.type == type) {
				return;
			}
		}
		return;
	}

	#define _SCREEN_W (DisplayWidth(dpy, 0))
	#define _SCREEN_H (DisplayHeight(dpy, 0))
	#include "../fix_pos.hpp"

	void handle_event(window &wnd, XEvent &event) {
		switch (event.type) {
			case Expose:
				wnd.on_paint.calls();
				return;
			case ConfigureNotify:
				wnd.on_size.calls();
				return;
			case MapNotify:
				if (wnd._move_buf.x != PWRE_NULL || wnd._move_buf.y != PWRE_NULL) {
					XMoveWindow(dpy, wnd._nwnd, wnd._move_buf.x, wnd._move_buf.y);
					wnd._move_buf = {PWRE_NULL, PWRE_NULL};

					sync_event_to(wnd, StructureNotifyMask, ConfigureNotify);
				}
				return;
			case ClientMessage:
				if (event.xclient.message_type == WM_PROTOCOLS && (Atom)event.xclient.data.l[0] == WM_DELETE_WINDOW) {
					if (!wnd.on_close.calls()) {
						return;
					}
					wnd.destroy();
				}
				return;
			case DestroyNotify:
				map.erase(wnd._nwnd);
				wnd.on_destroy.calls();
				wnd._nwnd = (Window)0;
				if (!map.size()) {
					map.clear();
					XCloseDisplay(dpy);
					life = false;
				}
		}
	}

	void next_event() {
		XEvent event;
		XNextEvent(dpy, &event);

		auto wnd = map[event.xany.window];
		if (wnd) {
			handle_event(*wnd, event);
		}
	}

	bool checkout_events() {
		while (life && XPending(dpy)) {
			next_event();
		}
		return life;
	}

	bool recv_event() {
		if (life) {
			next_event();
			return life;
		}
		return false;
	}

	void ctor(
		window &wnd,
		uint64_t hints,
		int depth,
		Visual *visual,
		unsigned long valuemask,
		XSetWindowAttributes *swa
	) {
		wnd._nwnd = XCreateWindow(
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
		if (!wnd._nwnd) {
			return;
		}

		XSetWMProtocols(dpy, wnd._nwnd, &WM_DELETE_WINDOW, 1);

		map[wnd._nwnd] = &wnd;
	}

	window::window(uint64_t hints) {
		if (hints == (uint64_t)-1) {
			return;
		}

		XSetWindowAttributes swa;
		swa.event_mask = _EVENT_MASK;

		ctor(*this, hints, XDefaultDepth(dpy, 0), XDefaultVisual(dpy, 0), CWEventMask, &swa);
	}

	void window::close() {
		if (on_close.calls()) {
			destroy();
		}
	}

	void window::destroy() {
		XDestroyWindow(dpy, _nwnd);

		sync_event_to(*this, StructureNotifyMask, DestroyNotify);
	}

	std::string window::title() {
		std::string title;

		Atom type;
		int format;
		unsigned long nitems, after;
		unsigned char *data;
		if (Success == XGetWindowProperty(dpy, _nwnd, _NET_WM_NAME, 0, LONG_MAX, False, UTF8_STRING, &type, &format, &nitems, &after, &data) && data) {
			title = (const char *)data;
			XFree(data);
		}

		return title;
	}

	void window::retitle(const std::string &title) {
		XChangeProperty(dpy, _nwnd, _NET_WM_NAME, UTF8_STRING, 8, PropModeReplace, (const unsigned char*)title.c_str(), title.size());
	}

	window::pos_type window::pos() {
		XWindowAttributes wa;
		XGetWindowAttributes(dpy, _nwnd, &wa);

		if (_move_buf.x != PWRE_NULL || _move_buf.y != PWRE_NULL) {
			return _move_buf;
		}

		return {wa.x, wa.y};
	}

	void window::move(pos_type pos) {
		XWindowAttributes wa;
		XGetWindowAttributes(dpy, _nwnd, &wa);

		fix_pos(pos.x, pos.y, wa.x, wa.y, wa.width, wa.height);

		if (wa.map_state != IsViewable) {
			_move_buf = pos;
			return;
		}

		XMoveWindow(dpy, _nwnd, pos.x, pos.y);

		sync_event_to(*this, StructureNotifyMask, ConfigureNotify);
	}

	window::size_type window::size() {
		XWindowAttributes wa;
		XGetWindowAttributes(dpy, _nwnd, &wa);
		return {wa.width, wa.height};
	}

	void window::resize(window::size_type sz) {
		XResizeWindow(dpy, _nwnd, sz.width, sz.height);

		sync_event_to(*this, StructureNotifyMask, ConfigureNotify);
	}

	void visible(window &wnd) {
		XWindowAttributes wa;
		XGetWindowAttributes(dpy, wnd.native_handle(), &wa);
		if (wa.map_state != IsViewable) {
			XMapRaised(dpy, wnd.native_handle());
		}

		sync_event_to(wnd, StructureNotifyMask, MapNotify);
	}

	void window::add_states(uint32_t type) {
		XEvent event;
		switch (type) {
			case PWRE_STATE_VISIBLE:
				visible(*this);
				break;
			case PWRE_STATE_MINIMIZE:
				visible(*this);
				XIconifyWindow(dpy, _nwnd, 0);
				break;
			case PWRE_STATE_MAXIMIZE:
				visible(*this);
				memset(&event, 0, sizeof(XClientMessageEvent));
				event.type = ClientMessage;
				event.xclient.window = _nwnd;
				event.xclient.message_type = _NET_WM_STATE;
				event.xclient.format = 32;
				event.xclient.data.l[0] = _NET_WM_STATE_ADD;
				event.xclient.data.l[1] = _NET_WM_STATE_MAXIMIZED_VERT;
				event.xclient.data.l[2] = _NET_WM_STATE_MAXIMIZED_HORZ;
				XSendEvent(dpy, root, False, StructureNotifyMask, &event);
				break;
			case PWRE_STATE_FULLSCREEN:
				visible(*this);
				memset(&event, 0, sizeof(XClientMessageEvent));
				event.type = ClientMessage;
				event.xclient.window = _nwnd;
				event.xclient.message_type = _NET_WM_STATE;
				event.xclient.format = 32;
				event.xclient.data.l[0] = _NET_WM_STATE_ADD;
				event.xclient.data.l[1] = _NET_WM_STATE_FULLSCREEN;
				XSendEvent(dpy, root, False, StructureNotifyMask, &event);
		}
	}

	void window::rm_states(uint32_t type) {
		XEvent event;
		switch (type) {
			case PWRE_STATE_VISIBLE:
				visible(*this);
				break;
			case PWRE_STATE_MINIMIZE:
				visible(*this);
				break;
			case PWRE_STATE_MAXIMIZE:
				visible(*this);
				memset(&event, 0, sizeof(XClientMessageEvent));
				event.type = ClientMessage;
				event.xclient.window = _nwnd;
				event.xclient.message_type = _NET_WM_STATE;
				event.xclient.format = 32;
				event.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
				event.xclient.data.l[1] = _NET_WM_STATE_MAXIMIZED_VERT;
				event.xclient.data.l[2] = _NET_WM_STATE_MAXIMIZED_HORZ;
				XSendEvent(dpy, root, False, StructureNotifyMask, &event);
				break;
			case PWRE_STATE_FULLSCREEN:
				visible(*this);
				memset(&event, 0, sizeof(XClientMessageEvent));
				event.type = ClientMessage;
				event.xclient.window = _nwnd;
				event.xclient.message_type = _NET_WM_STATE;
				event.xclient.format = 32;
				event.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
				event.xclient.data.l[1] = _NET_WM_STATE_FULLSCREEN;
				XSendEvent(dpy, root, False, StructureNotifyMask, &event);
		}
	}

	bool window::has_states(uint32_t type) {
		return false;
	}

	const unsigned MWM_HINTS_DECORATIONS = (1 << 1);
	const int PROP_MOTIF_WM_HINTS_ELEMENTS = 5;

	struct PropMotifWmHints {
		unsigned long flags;
		unsigned long functions;
		unsigned long decorations;
		long inputMode;
		unsigned long status;
	};

	void window::less(bool lessed) {
		PropMotifWmHints motif_hints;
		motif_hints.flags = MWM_HINTS_DECORATIONS;
		motif_hints.decorations = 0;
		XChangeProperty(dpy, _nwnd, _MOTIF_WM_HINTS, _MOTIF_WM_HINTS, 32, PropModeReplace, (unsigned char *) &motif_hints, PROP_MOTIF_WM_HINTS_ELEMENTS);
	}
} /* pwre */

#endif // PWRE_PLAT_X11
