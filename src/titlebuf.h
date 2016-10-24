// need {plat}.h

static void wnd_title_buf_clear(pwre_wnd_t wnd, size_t length) {
	if (length) {
		length++;
		if (wnd->title_buf) {
			if (wnd->title_buf_len < length) {
				free(wnd->title_buf);
				wnd->title_buf_len = length;
				wnd->title_buf = malloc(wnd->title_buf_len);
			}
		} else {
			wnd->title_buf_len = length;
			wnd->title_buf = malloc(wnd->title_buf_len);
		}
	} else if (!wnd->title_buf) {
		wnd->title_buf_len = 128;
		wnd->title_buf = malloc(wnd->title_buf_len);
	}
	memset(wnd->title_buf, 0, wnd->title_buf_len);
}
