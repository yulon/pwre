#include "pwre.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct PrWnd {
	void *ntvPtr;
	PrEventHandler evtHdr;
	int ncWidth, ncHeight;
	char *titleBuf;
	size_t titleBufLen;
};

static void PrWnd__free(PrWnd wnd) {
	if (wnd->titleBuf) {
		free(wnd->titleBuf);
	}
	free(wnd);
}

static void PrWnd__clearTitleBuf(PrWnd wnd, size_t length) {
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

static void PrWnd__updateTitleBuf(PrWnd wnd, const char *newTitle) {
	if (!newTitle) {
		PrWnd__clearTitleBuf(wnd, 0);
		return;
	}
	size_t newTitleLen = strlen(newTitle);
	if (!newTitleLen) {
		PrWnd__clearTitleBuf(wnd, 0);
		return;
	}
	PrWnd__clearTitleBuf(wnd, newTitleLen);
	strcpy(wnd->titleBuf, newTitle);
}

void *PrWnd_nativePointer(PrWnd wnd) {
	return wnd->ntvPtr;
}

PrEventHandler PrWnd_getEventHandler(PrWnd wnd) {
	return wnd->evtHdr;
}

void PrWnd_setEventHandler(PrWnd wnd, PrEventHandler evtHdr) {
	wnd->evtHdr = evtHdr;
}

static int wndCount;
static PrEventHandler dftEvtHdr;

#define _EVT_VARS(_wnd) \
	PrWnd wnd = _wnd; \
	bool processed; \
	PrSize size;

#define _EVT_POST(_hdr_existent_code, _event, _data) \
	if (wnd->evtHdr) { \
		_hdr_existent_code \
		wnd->evtHdr(wnd, _event, _data); \
	}

#define _EVT_SEND(_hdr_existent_code, _event, _data, _hdr_processed_code) \
	if (wnd->evtHdr) { \
		_hdr_existent_code \
		processed = wnd->evtHdr(wnd, _event, _data); \
	} else { \
		processed = false; \
	} \
	if (processed) { \
		_hdr_processed_code \
	}
