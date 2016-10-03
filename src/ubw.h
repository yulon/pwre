#ifndef _UBWINDOW_PRIVATE_H
#define _UBWINDOW_PRIVATE_H

#include "ubwindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int wndCount;
static UbwEventHandler dftEvtHdr;

typedef struct _UbwPvt {
	void *ntvPtr;
	UbwEventHandler evtHdr;
	int ncWidth, ncHeight;
} *_UbwPvt;

#define _EVT_VARS(_wnd) \
	_UbwPvt wnd = _wnd; \
	bool processed; \
	UbwSize size;

#define _EVT_POST(_hdr_existent_code, _event, _data) \
	if (wnd->evtHdr) { \
		_hdr_existent_code \
		(*wnd->evtHdr)((Ubw)wnd, _event, _data); \
	}

#define _EVT_SEND(_hdr_existent_code, _event, _data, _hdr_processed_code) \
	if (wnd->evtHdr) { \
		_hdr_existent_code \
		processed = (*wnd->evtHdr)((Ubw)wnd, _event, _data); \
	} else { \
		processed = false; \
	} \
	if (processed) { \
		_hdr_processed_code \
	}

#endif // !_UBWINDOW_PRIVATE_H
