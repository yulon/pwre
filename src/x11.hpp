#include <X11/Xlib.h>
namespace Pwre {
	typedef Window XWindow;
} /* Pwre */

#include "pwre.hpp"
#include <iostream>
#include <mutex>

namespace Pwre {
	namespace System {
		extern Display *dpy;
		extern XWindow root;
	} /* System */

	struct Window::_BlackBox {
		XWindow xWnd;
		std::string titCache;
		std::mutex mux;
	};

	bool WindowCoreConstructor(
		Window *wnd,
		uint64_t hints,
		int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
	);
} /* Pwre */

#define _SCREEN_W DisplayWidth(System::dpy, 0)
#define _SCREEN_H DisplayHeight(System::dpy, 0)
