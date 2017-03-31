#include <X11/Xlib.h>
namespace Pwre {
	typedef Window XWindow;
} /* Pwre */

#include "pwre.hpp"
#include <iostream>

namespace Pwre {
	extern Display *dpy;
	extern XWindow root;

	struct Window::_BlackBox {
		XWindow xWnd;
	};

	bool WindowCoreConstructor(
		Window *wnd,
		uint64_t hints,
		int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
	);
} /* Pwre */

#define _SCREEN_W DisplayWidth(dpy, 0)
#define _SCREEN_H DisplayHeight(dpy, 0)
