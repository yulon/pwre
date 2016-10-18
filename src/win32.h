#include "pwre.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <dwmapi.h>
#include <zk/mux.h>

struct PrWnd {
	HWND hWnd;
	int ncWidth, ncHeight;
	char *titleBuf;
	size_t titleBufLen;
	ZKMux dataMux;
	bool less;
	void (*onFree)(PrWnd);
};

PrWnd _alloc_PrWnd(size_t memSize, uint64_t hints);
