#ifndef _UBWINDOW_PRIVATE_H
#define _UBWINDOW_PRIVATE_H

#include "ubwindow.h"
#include <stdio.h>
#include <stdlib.h>

static int ubwSum;

typedef struct _Ubw {
	void* pNtv;
	UbwRect rect;
	UbwBounds borders;
	UbwBounds paddings;
	UbwSize szNonCont;
} _Ubw;

#if !defined(UBWINDOW_WIN32) && !defined(UBWINDOW_X11) && !defined(UBWINDOW_COCOA)
	#if defined(_WIN32)
		#define UBWINDOW_WIN32
	#elif defined(__APPLE__) && defined(__MACH__)
		#define UBWINDOW_COCOA
	#elif defined(__unix__)
		#define UBWINDOW_XCB
	#endif
#endif

#endif // !_UBWINDOW_PRIVATE_H
