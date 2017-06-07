#include "../plat.h"

#ifdef PWRE_PLAT_X11

#include "window.hpp"
#include <X11/Xutil.h>

#include <unordered_map>
#include <mutex>

#include <climits>
#include <cstring>

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

	std::unordered_map<Window, _window *> map;
	bool life = true;

	Display *dpy;
	Window root;

	void init() {
		XInitThreads();

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

	void on_destroy(_window *_wnd) {
		_wnd->on_destroy.calls();
		map.erase(_wnd->xwnd);
		if (!_wnd->ref) {
			delete _wnd;
		} else {
			_wnd->xwnd = NULL;
		}
		if (!map.size()) {
			map.clear();
			XCloseDisplay(dpy);
			life = false;
		}
	}

	#define _HANDLE_EVENT_RESULT_NORMAL 0
	#define _HANDLE_EVENT_RESULT_DESTROY 1
	#define _HANDLE_EVENT_RESULT_QUIT 2

	size_t handle_event_level = 0;

	void handle_event(_window *_wnd, XEvent &event) {
		switch (event.type) {
			case ConfigureNotify:
				_wnd->on_size.calls();
				break;
			case Expose:
				_wnd->on_paint.calls();
				break;
			case ClientMessage:
				if (event.xclient.message_type == WM_PROTOCOLS && (Atom)event.xclient.data.l[0] == WM_DELETE_WINDOW) {
					if (!_wnd->on_close.calls()) {
						return;
					}
					_wnd->destroy();
				}
				break;
			case DestroyNotify:
				on_destroy(_wnd);
		}
	}

	void next_event() {
		XEvent event;
		XNextEvent(dpy, &event);

		auto _wnd = map[event.xany.window];
		if (_wnd) {
			handle_event(_wnd, event);
		}/* else if (event->xany.window == root && event->type == ClientMessage && event->xclient.format == 32) {
			OnNotify();
		}*/
	}

	bool checkout_events() {
		while (life && XPending(dpy)) {
			next_event();
		}
		return life;
	}

	bool checkout_events(_window *_wnd, long event_mask) {
		_wnd->ref++;
		XEvent event;
		while (life && _wnd->xwnd && XCheckWindowEvent(dpy, _wnd->xwnd, event_mask, &event)) {
			handle_event(_wnd, event);
		}
		if (!--_wnd->ref && !_wnd->xwnd) {
			delete _wnd;
		}
		return life;
	}

	bool checkout_events(window *wnd) {
		return checkout_events(static_cast<_window *>(wnd), _EVENT_MASK);
	}

	bool recv_event() {
		if (life) {
			next_event();
			return life;
		}
		return false;
	}

	bool recv_event(window *wnd) {
		auto _wnd = static_cast<_window *>(wnd);
		if (life && _wnd->xwnd) {
			XEvent event;
			XWindowEvent(dpy, _wnd->xwnd, _EVENT_MASK, &event);
			_wnd->ref++;
			handle_event(_wnd, event);
			_wnd->ref--;
			if (_wnd->xwnd) {
				return true;
			}
		}
		if (!_wnd->ref) {
			delete _wnd;
		}
		return false;
	}

	/*void Notify() {
		XEvent event;
		memset(&event, 0, sizeof(XClientMessageEvent));
		event.type = ClientMessage;
		event.xclient.window = root;
		event.xclient.format = 32;
		XSendEvent(dpy, root, False, StructureNotifyMask, &event);
	}*/

	_window::_window(
		uint64_t hints,
		int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
	) {
		xwnd = XCreateWindow(
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
		uassert(xwnd, "Pwre", "XCreateWindow");

		XSetWMProtocols(dpy, xwnd, &WM_DELETE_WINDOW, 1);

		map[xwnd] = this;
		ref = 0;
	}

	window *create(uint64_t hints) {
		XSetWindowAttributes swa;
		swa.event_mask = _EVENT_MASK;
		return static_cast<window *>(new _window(
			hints,
			XDefaultDepth(dpy, 0), XDefaultVisual(dpy, 0), CWEventMask, &swa
		));
	}

	uintptr_t _window::native_handle() {
		return (uintptr_t)xwnd;
	}

	void _window::close() {
		if (on_close.calls()) {
			destroy();
		}
	}

	void _window::destroy() {
		XDestroyWindow(dpy, xwnd);
		pwre::on_destroy(this);
	}

	std::string _window::title() {
		std::string title;

		Atom type;
		int format;
		unsigned long nitems, after;
		unsigned char *data;
		if (Success == XGetWindowProperty(dpy, xwnd, _NET_WM_NAME, 0, LONG_MAX, False, UTF8_STRING, &type, &format, &nitems, &after, &data) && data) {
			title = (const char *)data;
			XFree(data);
		}

		return title;
	}

	void _window::retitle(const std::string &title) {
		XChangeProperty(dpy, xwnd, _NET_WM_NAME, UTF8_STRING, 8, PropModeReplace, (const unsigned char*)title.c_str(), title.size());
	}

	point _window::pos() {
		checkout_events(this, ConfigureNotify);

		XWindowAttributes wa;
		XGetWindowAttributes(dpy, xwnd, &wa);
		return {wa.x, wa.y};
	}

	#define _SCREEN_W (DisplayWidth(dpy, 0))
	#define _SCREEN_H (DisplayHeight(dpy, 0))
	#include "../fix_pos.hpp"

	void _window::move(point pos) {
		auto sz = size();
		fix_pos(pos.x, pos.y, sz.width, sz.height);

		XMoveWindow(dpy, xwnd, pos.x, pos.y);
	}

	pwre::size _window::size() {
		checkout_events(this, StructureNotifyMask);

		XWindowAttributes wa;
		XGetWindowAttributes(dpy, xwnd, &wa);
		return {wa.width, wa.height};
	}

	void _window::resize(pwre::size sz) {
		XResizeWindow(dpy, xwnd, sz.width, sz.height);
	}

	static void visible(_window *_wnd) {
		checkout_events(_wnd, StructureNotifyMask);

		XWindowAttributes wa;
		XGetWindowAttributes(dpy, _wnd->xwnd, &wa);
		if (wa.map_state != IsViewable && XMapRaised(dpy, _wnd->xwnd) != BadWindow);
	}

	void _window::add_states(uint32_t type) {
		XEvent event;
		switch (type) {
			case PWRE_STATE_VISIBLE:
				visible(this);
				break;
			case PWRE_STATE_MINIMIZE:
				visible(this);
				XIconifyWindow(dpy, xwnd, 0);
				break;
			case PWRE_STATE_MAXIMIZE:
				visible(this);
				memset(&event, 0, sizeof(XClientMessageEvent));
				event.type = ClientMessage;
				event.xclient.window = xwnd;
				event.xclient.message_type = _NET_WM_STATE;
				event.xclient.format = 32;
				event.xclient.data.l[0] = _NET_WM_STATE_ADD;
				event.xclient.data.l[1] = _NET_WM_STATE_MAXIMIZED_VERT;
				event.xclient.data.l[2] = _NET_WM_STATE_MAXIMIZED_HORZ;
				XSendEvent(dpy, root, False, StructureNotifyMask, &event);
				break;
			case PWRE_STATE_FULLSCREEN:
				visible(this);
				memset(&event, 0, sizeof(XClientMessageEvent));
				event.type = ClientMessage;
				event.xclient.window = xwnd;
				event.xclient.message_type = _NET_WM_STATE;
				event.xclient.format = 32;
				event.xclient.data.l[0] = _NET_WM_STATE_ADD;
				event.xclient.data.l[1] = _NET_WM_STATE_FULLSCREEN;
				XSendEvent(dpy, root, False, StructureNotifyMask, &event);
		}
	}

	void _window::rm_states(uint32_t type) {
		XEvent event;
		switch (type) {
			case PWRE_STATE_VISIBLE:
				visible(this);
				break;
			case PWRE_STATE_MINIMIZE:
				visible(this);
				break;
			case PWRE_STATE_MAXIMIZE:
				visible(this);
				memset(&event, 0, sizeof(XClientMessageEvent));
				event.type = ClientMessage;
				event.xclient.window = xwnd;
				event.xclient.message_type = _NET_WM_STATE;
				event.xclient.format = 32;
				event.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
				event.xclient.data.l[1] = _NET_WM_STATE_MAXIMIZED_VERT;
				event.xclient.data.l[2] = _NET_WM_STATE_MAXIMIZED_HORZ;
				XSendEvent(dpy, root, False, StructureNotifyMask, &event);
				break;
			case PWRE_STATE_FULLSCREEN:
				visible(this);
				memset(&event, 0, sizeof(XClientMessageEvent));
				event.type = ClientMessage;
				event.xclient.window = xwnd;
				event.xclient.message_type = _NET_WM_STATE;
				event.xclient.format = 32;
				event.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
				event.xclient.data.l[1] = _NET_WM_STATE_FULLSCREEN;
				XSendEvent(dpy, root, False, StructureNotifyMask, &event);
		}
	}

	bool _window::has_states(uint32_t type) {
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

	void _window::less(bool less) {
		PropMotifWmHints motif_hints;
		motif_hints.flags = MWM_HINTS_DECORATIONS;
		motif_hints.decorations = 0;
		XChangeProperty(dpy, xwnd, _MOTIF_WM_HINTS, _MOTIF_WM_HINTS, 32, PropModeReplace, (unsigned char *) &motif_hints, PROP_MOTIF_WM_HINTS_ELEMENTS);
	}
} /* pwre */

#endif // PWRE_PLAT_X11
