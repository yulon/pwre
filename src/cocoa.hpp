#include "pwre.hpp"
#include <iostream>
#import <Cocoa/Cocoa.h>

namespace Pwre {
	struct Window::_BlackBox {
		NSWindow *nsWnd;
	};
} /* Pwre */

#define _SCREEN_W [[NSScreen mainScreen] frame].size.width
#define _SCREEN_H [[NSScreen mainScreen] frame].size.height
