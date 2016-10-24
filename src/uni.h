// need {plat}.h

static int wnd_count;

#include <zk/mutex.h>

static zk_mutex_t wnd_count_mux;

static pwre_event_handler_t event_handler;

#define _EVENT_POST(_hdr_existent_code, _event, _data) \
	if (event_handler) { \
		_hdr_existent_code \
		event_handler(wnd, _event, _data); \
	}

#define _EVENT_SEND(_hdr_existent_code, _event, _data, _hdr_return_false_code) \
	if (event_handler) { \
		_hdr_existent_code \
		if (!event_handler(wnd, _event, _data)) { \
			_hdr_return_false_code \
		} \
	}

static inline void fix_pos(int *x, int *y, int width, int height) {
	if (*x == PWRE_MOVE_AUTO) {
		*x = (_SCREEN_W - width) / 2;
	}
	if (*y == PWRE_MOVE_AUTO) {
		*y = (_SCREEN_H - height) / 2;
	}
}
