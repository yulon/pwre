#ifndef _UBWINDOW_PRIVATE_H
#define _UBWINDOW_PRIVATE_H

#include "ubwindow.h"
#include <stdio.h>
#include <stdlib.h>

static int wndCount;
static UbwEventHandler dftEvtHdr;

typedef struct _UbwPvt {
	void *ntvPtr;
	int x, y, width, height;
	int ncWidth, ncHeight;
	UbwEventHandler *evtHdr;
} _UbwPvt;

#define _EVT_VARS(_wnd) \
	_UbwPvt *wnd = _wnd; \
	int processed; \
	UbwSize size;

#define _EVT_POST(_hdr_existent_code, _event, _data) \
	if (wnd->evtHdr) { \
		_hdr_existent_code \
		(*wnd->evtHdr)(wnd, _event, _data); \
	}

#define _EVT_SEND(_hdr_existent_code, _event, _data, _hdr_processed_code) \
	if (wnd->evtHdr) { \
		_hdr_existent_code \
		processed = (*wnd->evtHdr)(wnd, _event, _data); \
	} else { \
		processed = 0; \
	} \
	if (processed) { \
		_hdr_processed_code \
	}

#if !defined(UBWINDOW_WIN32) && !defined(UBWINDOW_X11) && !defined(UBWINDOW_COCOA)
	#if defined(_WIN32)
		#define UBWINDOW_WIN32
	#elif defined(__APPLE__) && defined(__MACH__)
		#define UBWINDOW_COCOA
	#elif defined(__unix__)
		#define UBWINDOW_X11
	#endif
#endif

#endif // !_UBWINDOW_PRIVATE_H
