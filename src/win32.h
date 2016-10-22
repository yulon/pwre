#include "pwre.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <dwmapi.h>
#include <zk/mux.h>

struct pwre_wnd {
	HWND hWnd;
	int nc_width, nc_height;
	char *title_buf;
	size_t title_buf_len;
	ZKMux data_mux;
	bool less;
	void (*on_free)(pwre_wnd_t);
};

pwre_wnd_t alloc_wnd(size_t struct_size, uint64_t hints);

#define _SCREEN_W GetSystemMetrics(SM_CXSCREEN)
#define _SCREEN_H GetSystemMetrics(SM_CYSCREEN)
