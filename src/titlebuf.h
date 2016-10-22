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
