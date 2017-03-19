#include "plat.h"

#ifdef PWRE_PLAT_X11

#include "x11.hpp"
#include <X11/Xutil.h>

#include <unordered_map>
#include "zk_rwlock.hpp"
#include <mutex>
#include <atomic>

#include <climits>
#include <cstring>

namespace Pwre {
	Atom netWmName;
	Atom utf8Str;
	Atom wmDelWnd;
	Atom wmProtocols;

	Atom netWmState;
	#define netWmStateRemove 0
	#define netWmStateAdd 1
	#define netWmStateToggle 2
	Atom netWmStateHide;
	Atom netWmStateMaxVert;
	Atom netWmStateMaxHorz;
	Atom netWmStateFullscr;
	Atom motifWmHints;

	namespace System {
		std::unordered_map<XWindow, Window *> wndMap;
		_PWRE_SHARED_MUTEX_NS::shared_mutex wndMapLock;
		std::mutex xEventMux;
		std::atomic<int> wndCount;

		Display *dpy;
		XWindow root;

		bool Init() {
			XInitThreads();
			dpy = XOpenDisplay(NULL);
			if (!dpy) {
				std::cout << "Pwre: X11.XOpenDisplay error!" << std::endl;
				return false;
			}
			root = XRootWindow(dpy, 0);
			netWmName = XInternAtom(dpy, "_NET_WM_NAME", False);
			utf8Str = XInternAtom(dpy, "UTF8_STRING", False);
			wmDelWnd = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
			wmProtocols = XInternAtom(dpy, "WM_PROTOCOLS", False);

			netWmState =  XInternAtom(dpy, "_NET_WM_STATE", False);
			netWmStateHide =  XInternAtom(dpy, "_NET_WM_STATE_HIDDEN", False);
			netWmStateMaxVert = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
			netWmStateMaxHorz = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
			netWmStateFullscr = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

			motifWmHints = XInternAtom(dpy, "_MOTIF_WM_HINTS", True);

			wndCount = 0;
			return true;
		}

		#define _XEVENT_SYNC(_wnd, _event, _conds, _bingo) { \
			XEvent event; \
			while (System::XEventRecv(&event, false)) { \
				if ( \
					event.xany.window == _wnd && \
					event.xany.type == _event \
					_conds \
				) { \
					_bingo \
					break; \
				} \
			} \
		}

		bool XEventRecv(XEvent *event, bool mux) {
			if (mux) {
				System::xEventMux.lock();
				XNextEvent(dpy, event);
				System::xEventMux.unlock();
			} else {
				XNextEvent(dpy, event);
			}

			System::wndMapLock.lock_shared();
			auto wnd = System::wndMap[event->xany.window];
			System::wndMapLock.unlock_shared();

			if (wnd) {
				switch (event->xany.type) {
					case ConfigureNotify:
						wnd->OnSize.Receive(event->xconfigure.width, event->xconfigure.height);
						break;
					case Expose:
						wnd->OnPaint.Receive();
						break;
					case ClientMessage:
						if (event->xclient.message_type == wmProtocols && (Atom)event->xclient.data.l[0] == wmDelWnd) {
							if (!wnd->OnClose.Accept()) {
								return true;
							}

							bool ret = false;

							System::xEventMux.lock();
							XDestroyWindow(dpy, event->xany.window);
							_XEVENT_SYNC(
								wnd->_m->xWnd,
								DestroyNotify,
								,
								ret = true;
							)
							System::xEventMux.unlock();

							return ret;
						}
						break;
					case DestroyNotify:
						wnd->OnDestroy.Receive();

						System::wndMapLock.lock();
						System::wndMap.erase(wnd->_m->xWnd);
						System::wndMapLock.unlock();

						if (!--wndCount) {
							System::wndMap.clear();
							XCloseDisplay(dpy);
							return false;
						}
				}
			}
			return true;
		}

		bool Step() {
			XEvent event;
			while (XPending(dpy)) {
				if (!XEventRecv(&event, true)) {
					return false;
				}
			}
			return true;
		}

		void Run() {
			XEvent event;
			while (XEventRecv(&event, true));
		}
	} /* System */

	bool WindowCoreConstructor(
		Window *wnd,
		uint64_t hints,
		int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
	) {
		wnd->_m->xWnd = XCreateWindow(
			System::dpy,
			System::root,
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
		if (!wnd->_m->xWnd) {
			std::cout << "Pwre: X11.XCreateSimpleWindow error!" << std::endl;
			return false;
		}

		XSetWMProtocols(System::dpy, wnd->_m->xWnd, &wmDelWnd, 1);

		System::wndCount++;

		System::wndMapLock.lock();
		System::wndMap[wnd->_m->xWnd] = wnd;
		System::wndMapLock.unlock();
		return true;
	}

	Window::Window(uint64_t hints) {
		_m = new _BlackBox;
		if (hints != (uint64_t)-1) {
			XSetWindowAttributes swa;
			swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
			WindowCoreConstructor(
				this,
				hints,
				XDefaultDepth(System::dpy, 0), XDefaultVisual(System::dpy, 0), CWEventMask, &swa
			);
		}
	}

	Window::~Window() {
		delete _m;
	}

	uintptr_t Window::NativeObj() {
		return (uintptr_t)_m->xWnd;
	}

	void Window::Close() {
		if (OnClose.Accept()) {
			Destroy();
		}
	}

	void Window::Destroy() {
		XDestroyWindow(System::dpy, _m->xWnd);
	}

	std::string Window::Title() {
		std::string title;

		Atom type;
		int format;
		unsigned long nitems, after;
		unsigned char *data;
		if (Success == XGetWindowProperty(System::dpy, _m->xWnd, netWmName, 0, LONG_MAX, False, utf8Str, &type, &format, &nitems, &after, &data) && data) {
			title = (const char *)data;
			XFree(data);
		}

		return title;
	}

	void Window::Retitle(const std::string &title) {
		XChangeProperty(System::dpy, _m->xWnd, netWmName, utf8Str, 8, PropModeReplace, (const unsigned char*)title.c_str(), title.size());
	}

	#include "fixpos.hpp"

	void Window::Move(int x, int y) {
		XWindowAttributes wa;
		XGetWindowAttributes(System::dpy, _m->xWnd, &wa);
		FixPos(x, y, wa.width, wa.height);

		System::xEventMux.lock();
		int err = XMoveWindow(System::dpy, _m->xWnd, x, y);
		if (err != BadValue && err != BadWindow && err != BadMatch) {
			_XEVENT_SYNC(
				_m->xWnd,
				ConfigureNotify,
				&& (
					event.xconfigure.x != 0 ||
					event.xconfigure.y != 0
				),
			)
		}
		System::xEventMux.unlock();
	}

	void Window::Size(int &width, int &height) {
		XWindowAttributes wa;
		XGetWindowAttributes(System::dpy, _m->xWnd, &wa);
		if (width) {
			width = wa.width;
		}
		if (height) {
			height = wa.height;
		}
	}

	void Window::Resize(int width, int height) {
		System::xEventMux.lock();
		int err = XResizeWindow(System::dpy, _m->xWnd, width, height);
		if (err != BadValue && err != BadWindow) {
			_XEVENT_SYNC(
				_m->xWnd,
				ConfigureNotify,
				&& event.xconfigure.width == width
				&& event.xconfigure.height == height,
			)
		}
		System::xEventMux.unlock();
	}

	static void Visible(Window *wnd) {
		XWindowAttributes wa;
		XGetWindowAttributes(System::dpy, wnd->_m->xWnd, &wa);
		System::xEventMux.lock();
		if (wa.map_state != IsViewable && XMapRaised(System::dpy, wnd->_m->xWnd) != BadWindow && wa.map_state == IsUnmapped) {
			_XEVENT_SYNC(
				wnd->_m->xWnd,
				MapNotify,
				,
			)
		}
		System::xEventMux.unlock();
	}

	void Window::AddStates(uint32_t type) {
		XEvent event;
		switch (type) {
			case PWRE_STATE_VISIBLE:
				Visible(this);
				break;
			case PWRE_STATE_MINIMIZE:
				Visible(this);
				XIconifyWindow(System::dpy, _m->xWnd, 0);
				break;
			case PWRE_STATE_MAXIMIZE:
				Visible(this);
				memset(&event, 0, sizeof(event));
				event.type = ClientMessage;
				event.xclient.window = _m->xWnd;
				event.xclient.message_type = netWmState;
				event.xclient.format = 32;
				event.xclient.data.l[0] = netWmStateAdd;
				event.xclient.data.l[1] = netWmStateMaxVert;
				event.xclient.data.l[2] = netWmStateMaxHorz;
				XSendEvent(System::dpy, System::root, False, StructureNotifyMask, &event);
				break;
			case PWRE_STATE_FULLSCREEN:
				Visible(this);
				memset(&event, 0, sizeof(event));
				event.type = ClientMessage;
				event.xclient.window = _m->xWnd;
				event.xclient.message_type = netWmState;
				event.xclient.format = 32;
				event.xclient.data.l[0] = netWmStateAdd;
				event.xclient.data.l[1] = netWmStateFullscr;
				XSendEvent(System::dpy, System::root, False, StructureNotifyMask, &event);
		}
		return;
	}

	void Window::RmStates(uint32_t type) {
		XEvent event;
		switch (type) {
			case PWRE_STATE_VISIBLE:
				Visible(this);
				break;
			case PWRE_STATE_MINIMIZE:
				Visible(this);
				break;
			case PWRE_STATE_MAXIMIZE:
				Visible(this);
				memset(&event, 0, sizeof(event));
				event.type = ClientMessage;
				event.xclient.window = _m->xWnd;
				event.xclient.message_type = netWmState;
				event.xclient.format = 32;
				event.xclient.data.l[0] = netWmStateRemove;
				event.xclient.data.l[1] = netWmStateMaxVert;
				event.xclient.data.l[2] = netWmStateMaxHorz;
				XSendEvent(System::dpy, System::root, False, StructureNotifyMask, &event);
				break;
			case PWRE_STATE_FULLSCREEN:
				Visible(this);
				memset(&event, 0, sizeof(event));
				event.type = ClientMessage;
				event.xclient.window = _m->xWnd;
				event.xclient.message_type = netWmState;
				event.xclient.format = 32;
				event.xclient.data.l[0] = netWmStateRemove;
				event.xclient.data.l[1] = netWmStateFullscr;
				XSendEvent(System::dpy, System::root, False, StructureNotifyMask, &event);
		}
	}

	bool Window::HasStates(uint32_t type) {
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

	void Window::Less(bool less) {
		PropMotifWmHints motifHints;
		motifHints.flags = MWM_HINTS_DECORATIONS;
		motifHints.decorations = 0;
		XChangeProperty(System::dpy, _m->xWnd, motifWmHints, motifWmHints, 32, PropModeReplace, (unsigned char *) &motifHints, PROP_MOTIF_WM_HINTS_ELEMENTS);
	}
} /* Pwre */

#endif // PWRE_PLAT_X11
