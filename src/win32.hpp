#include "pwre.hpp"
#include "gui_thrd.hpp"
#include <windows.h>

namespace Pwre {
	struct Window::_BlackBox {
		HWND hWnd;
		int ncWidth, ncHeight;
		bool less;
	};
} /* Pwre */

#define _SCREEN_W GetSystemMetrics(SM_CXSCREEN)
#define _SCREEN_H GetSystemMetrics(SM_CYSCREEN)
