// need {plat}.h

static void _PrWnd_clearTitleBuf(PrWnd wnd, size_t length) {
	if (length) {
		length++;
		if (wnd->titleBuf) {
			if (wnd->titleBufLen < length) {
				free(wnd->titleBuf);
				wnd->titleBufLen = length;
				wnd->titleBuf = malloc(wnd->titleBufLen);
			}
		} else {
			wnd->titleBufLen = length;
			wnd->titleBuf = malloc(wnd->titleBufLen);
		}
	} else if (!wnd->titleBuf) {
		wnd->titleBufLen = 128;
		wnd->titleBuf = malloc(wnd->titleBufLen);
	}
	memset(wnd->titleBuf, 0, wnd->titleBufLen);
}

static void _PrWnd_flushTitleBuf(PrWnd wnd, const char *newTitle) {
	if (!newTitle) {
		_PrWnd_clearTitleBuf(wnd, 0);
		return;
	}
	size_t newTitleLen = strlen(newTitle);
	if (!newTitleLen) {
		_PrWnd_clearTitleBuf(wnd, 0);
		return;
	}
	_PrWnd_clearTitleBuf(wnd, newTitleLen);
	strcpy(wnd->titleBuf, newTitle);
}
