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

#define eventSend(_hdr_existent_code, _event, _data, _hdr_return_false_code) \
	if (eventHandler) { \
		_hdr_existent_code \
		if (!eventHandler(wnd, _event, _data)) { \
			_hdr_return_false_code \
		} \
	}

static inline void fixPos(int *x, int *y, int width, int height) {
	if (*x == PWRE_POS_AUTO) {
		*x = (screenWidth - width) / 2;
	}
	if (*y == PWRE_POS_AUTO) {
		*y = (screenHeight - height) / 2;
	}
}
