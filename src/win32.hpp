#include "pwre.hpp"
#include <iostream>
#include <windows.h>
#include <atomic>

namespace Pwre {
	struct Window::_BlackBox {
		HWND hWnd;
		int ncWidth, ncHeight;
		std::atomic<bool> less;
	};
} /* Pwre */

#define _SCREEN_W GetSystemMetrics(SM_CXSCREEN)
#define _SCREEN_H GetSystemMetrics(SM_CYSCREEN)
