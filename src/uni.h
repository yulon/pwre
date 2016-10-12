// need {plat}.h

PrEventHandler PrWnd_getEventHandler(PrWnd wnd) {
	return wnd->evtHdr;
}

void PrWnd_setEventHandler(PrWnd wnd, PrEventHandler evtHdr) {
	wnd->evtHdr = evtHdr;
}

static int wndCount;

#include "mutex.h"

static Mutex wndCountMux;

static PrEventHandler dftEvtHdr;

#define eventTarget(_wnd) \
	PrWnd wnd = _wnd; \
	bool evtHdrRst; \

#define eventPost(_hdr_existent_code, _event, _data) \
	if (wnd->evtHdr) { \
		_hdr_existent_code \
		wnd->evtHdr(wnd, _event, _data); \
	}

#define eventSend(_hdr_existent_code, _event, _data, _hdr_return_false_code) \
	if (wnd->evtHdr) { \
		_hdr_existent_code \
		evtHdrRst = wnd->evtHdr(wnd, _event, _data); \
	} else { \
		evtHdrRst = true; \
	} \
	if (!evtHdrRst) { \
		_hdr_return_false_code \
	}
