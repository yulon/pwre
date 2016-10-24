// need {plat}.h

#include "titlebuf.h"

static void wnd_title_buf_flush(pwre_wnd_t wnd, const char *new_title) {
	if (!new_title) {
		wnd_title_buf_clear(wnd, 0);
		return;
	}
	size_t new_title_len = strlen(new_title);
	if (!new_title_len) {
		wnd_title_buf_clear(wnd, 0);
		return;
	}
	wnd_title_buf_clear(wnd, new_title_len);
	strcpy(wnd->title_buf, new_title);
}
