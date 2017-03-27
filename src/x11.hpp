#include <X11/Xlib.h>
namespace Pwre {
	typedef Window XWindow;
} /* Pwre */

#include "pwre.hpp"
#include <iostream>

namespace Pwre {
	extern class WindowSystem {
		public:
			Display *dpy;
			XWindow root;

			WindowSystem();
	} wndSys;

	struct Window::_BlackBox {
		XWindow xWnd;
	};

	bool WindowCoreConstructor(
		Window *wnd,
		uint64_t hints,
		int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
	);
} /* Pwre */

#define _SCREEN_W DisplayWidth(wndSys.dpy, 0)
#define _SCREEN_H DisplayHeight(wndSys.dpy, 0)
