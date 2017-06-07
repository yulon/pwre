#include <X11/Xlib.h>
#include <pwre.hpp>
#include <iostream>
#include "../uassert.h"

namespace pwre {
	extern Display *dpy;
	extern Window root;

	class _window : public window {
		public:
			size_t ref;
			Window x;

			////////////////////////////////////////////////////////////////////

			_window(
				uint64_t hints,
				int depth, Visual *visual, unsigned long valuemask, XSetWindowAttributes *swa
			);

			virtual void close();
			virtual void destroy();

			virtual uintptr_t native_handle();

			virtual std::string title();
			virtual void retitle(const std::string &);

			virtual point pos();
			virtual void move(point);

			virtual pwre::size size();
			virtual void resize(pwre::size);

			virtual void add_states(uint32_t);
			virtual void rm_states(uint32_t);
			virtual bool has_states(uint32_t);

			virtual void less(bool);
			//virtual bool rearea(const action_area &);
	};
} /* pwre */
