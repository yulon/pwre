#include "pwre.hpp"
#include <iostream>
#include <windows.h>
#include <mutex>
#include <atomic>

namespace Pwre {
	struct Window::_BlackBox {
		HWND HWnd;
		int NCWidth, NCHeight;
		std::string TitleBuf;
		std::mutex Mux;
		std::atomic<bool> Lessed;
	};
} /* Pwre */

#define _SCREEN_W GetSystemMetrics(SM_CXSCREEN)
#define _SCREEN_H GetSystemMetrics(SM_CYSCREEN)
