// need {plat}.h

static int wndCount;

#include <zk/mux.h>

static ZKMux wndCountMux;

static PrEventHandler eventHandler;

#define eventPost(_hdr_existent_code, _event, _data) \
	if (eventHandler) { \
		_hdr_existent_code \
		eventHandler(wnd, _event, _data); \
	}

#define eventSend(_hdr_existent_code, _event, _data, _hdr_return_false_code) { \
	bool evtHdrRst; \
	if (eventHandler) { \
		_hdr_existent_code \
		evtHdrRst = eventHandler(wnd, _event, _data); \
	} else { \
		evtHdrRst = true; \
	} \
	if (!evtHdrRst) { \
		_hdr_return_false_code \
	} \
}
