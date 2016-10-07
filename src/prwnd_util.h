#include "pwre.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PrWndPvt {
	void *ntvPtr;
	PrEventHandler evtHdr;
	int ncWidth, ncHeight;
	char *titleBuf;
	size_t titleBufLen;
} *PrWndPvt;

static PrWndPvt new_PrWndPvt(void) {
	return calloc(1, sizeof(struct PrWndPvt));
}

static void PrWndPvt_free(PrWndPvt wndPvt) {
	if (wndPvt->titleBuf) {
		free(wndPvt->titleBuf);
	}
	free(wndPvt);
}

static void PrWndPvt_blankTitle(PrWndPvt wndPvt, size_t length) {
	if (length) {
		length++;
		if (wndPvt->titleBuf) {
			if (wndPvt->titleBufLen < length) {
				free(wndPvt->titleBuf);
				wndPvt->titleBufLen = length;
				wndPvt->titleBuf = malloc(wndPvt->titleBufLen);
			}
		} else {
			wndPvt->titleBufLen = length;
			wndPvt->titleBuf = malloc(wndPvt->titleBufLen);
		}
	} else if (!wndPvt->titleBuf) {
		wndPvt->titleBufLen = 128;
		wndPvt->titleBuf = malloc(wndPvt->titleBufLen);
	}
	memset(wndPvt->titleBuf, 0, wndPvt->titleBufLen);
}

static void PrWndPvt_copyTitle(PrWndPvt wndPvt, const char *newTitle) {
	if (!newTitle) {
		PrWndPvt_blankTitle(wndPvt, 0);
		return;
	}
	size_t newTitleLen = strlen(newTitle);
	if (!newTitleLen) {
		PrWndPvt_blankTitle(wndPvt, 0);
		return;
	}
	PrWndPvt_blankTitle(wndPvt, newTitleLen);
	strcpy(wndPvt->titleBuf, newTitle);
}

static int wndCount;
static PrEventHandler dftEvtHdr;

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
