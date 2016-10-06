#include "pwre.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int wndCount;
static PrEventHandler dftEvtHdr;

typedef struct PrWndPvt {
	void *ntvPtr;
	PrEventHandler evtHdr;
	int ncWidth, ncHeight;
} *PrWndPvt;

void *PrWnd_nativePointer(PrWnd wnd) {
	return ((PrWndPvt)wnd)->ntvPtr;
}

#define _EVT_VARS(_wnd) \
	PrWndPvt wnd = _wnd; \
	bool processed; \
	PrSize size;

#define _EVT_POST(_hdr_existent_code, _event, _data) \
	if (wnd->evtHdr) { \
		_hdr_existent_code \
		(*wnd->evtHdr)((PrWnd)wnd, _event, _data); \
	}

#define _EVT_SEND(_hdr_existent_code, _event, _data, _hdr_processed_code) \
	if (wnd->evtHdr) { \
		_hdr_existent_code \
		processed = (*wnd->evtHdr)((PrWnd)wnd, _event, _data); \
	} else { \
		processed = false; \
	} \
	if (processed) { \
		_hdr_processed_code \
	}
