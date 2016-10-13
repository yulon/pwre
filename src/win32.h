#include "pwre.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <zk/mux.h>

struct PrWnd {
	HWND hWnd;
	PrEventHandler evtHdr;
	int ncWidth, ncHeight;
	char *titleBuf;
	size_t titleBufLen;
	ZKMux dataMux;
	void (*onFree)(PrWnd);
};

PrWnd _alloc_PrWnd(size_t size, int x, int y, int width, int height);
