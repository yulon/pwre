#include "plat.h"

#ifdef PWRE_WIN32

#ifndef PWRE_WIN32_WNDEXTRA
#define PWRE_WIN32_WNDEXTRA 10
#endif

#ifndef PWRE_WIN32_WNDEXTRA_I
#define PWRE_WIN32_WNDEXTRA_I 6
#endif

#include "win32.h"
#include "uni.h"
#include "titlebuf.h"

static void _PrWnd_free(PrWnd wnd) {
	if (wnd->onFree) {
		wnd->onFree(wnd);
	}
	Mutex_lock(wnd->dataMux);
	if (wnd->titleBuf) {
		free(wnd->titleBuf);
	}
	Mutex_unlock(wnd->dataMux);
	Mutex_free(wnd->dataMux);
	free(wnd);
}

static LRESULT CALLBACK wndMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	eventTarget((PrWnd)GetWindowLongPtrW(hWnd, PWRE_WIN32_WNDEXTRA_I))
	if (wnd) {
		switch (uMsg) {
			case WM_PAINT:
				eventPost(, PrEvent_Paint, NULL)
				ValidateRect(hWnd, NULL);
				return 0;
			case WM_CLOSE:
				eventSend(, PrEvent_Close, NULL,
					return 0;
				)
				break;
			case WM_DESTROY:
				SetWindowLongPtrW(hWnd, PWRE_WIN32_WNDEXTRA_I, 0);

				eventPost(, PrEvent_Destroy, NULL)

				Mutex_lock(wndCountMux);
				wndCount--;
				_PrWnd_free(wnd);
				if (!wndCount) {
					Mutex_unlock(wndCountMux);
					Mutex_free(wndCountMux);
					PostQuitMessage(0);
					return 0;
				}
				Mutex_unlock(wndCountMux);
		}
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static HMODULE mainModule;
static WNDCLASSEXW wndClass;

bool pwreInit(PrEventHandler evtHdr) {
	mainModule = GetModuleHandleW(NULL);

	wndClass.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndClass.hInstance = mainModule;
	wndClass.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
	wndClass.hIconSm = LoadIconW(NULL, (LPCWSTR)IDI_WINLOGO);
	wndClass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	wndClass.lpszClassName = L"PwreWindow";
	wndClass.cbWndExtra = sizeof(void *) * PWRE_WIN32_WNDEXTRA;
	wndClass.lpfnWndProc = wndMsgHandler;
	wndClass.cbSize = sizeof(wndClass);

	ATOM ok = RegisterClassExW(&wndClass);
	if (!ok) {
		puts("Pwre: Win32.RegisterClassExW error!");
		return false;
	}

	wndCountMux = new_Mutex();
	dftEvtHdr = evtHdr;
	return true;
}

bool pwreStep(void) {
	MSG msg;
	if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
		return msg.message != WM_QUIT;
	}
	return true;
}

void pwreRun(void) {
	MSG msg = { .message = 0 };
	while (msg.message != WM_QUIT) {
		if (GetMessageW(&msg, NULL, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
}

static void fixPos(int *x, int *y, int width, int height) {
	if (*x == PrPos_ScreenCenter) {
		*x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	}
	if (*y == PrPos_ScreenCenter) {
		*y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	}
}

PrWnd _alloc_PrWnd(size_t size, int x, int y, int width, int height) {
	fixPos(&x, &y, width, height);

	HWND hWnd = CreateWindowExW(
		0,
		wndClass.lpszClassName,
		NULL,
		WS_OVERLAPPEDWINDOW,
		x, y, width, height, NULL, NULL,
		mainModule,
		NULL
	);
	if (!hWnd) {
		puts("Pwre: Win32.CreateWindowExW error!");
		return NULL;
	}

	RECT rect = { 500, 500, 1000, 1000 };
	BOOL ok = AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
	if (!ok) {
		puts("Pwre: Win32.AdjustWindowRectEx error!");
		return 0;
	}

	Mutex_lock(wndCountMux); wndCount++; Mutex_unlock(wndCountMux);

	PrWnd wnd = calloc(1, size);
	wnd->dataMux = new_Mutex();

	wnd->hWnd = hWnd;
	wnd->evtHdr = dftEvtHdr;

	wnd->ncWidth = (500 - rect.left) + (rect.right - 1000);
	wnd->ncHeight = (500 - rect.top) + (rect.bottom - 1000);

	SetWindowLongPtrW(hWnd, PWRE_WIN32_WNDEXTRA_I, (LONG_PTR)wnd);
	return wnd;
}

PrWnd new_PrWnd(int x, int y, int width, int height) {
	return _alloc_PrWnd(sizeof(struct PrWnd), x, y, width, height);
}

void *PrWnd_nativePointer(PrWnd wnd) {
	return (void *)wnd->hWnd;
}

void PrWnd_close(PrWnd wnd) {
	CloseWindow(wnd->hWnd);
}

void PrWnd_destroy(PrWnd wnd) {
	DestroyWindow(wnd->hWnd);
}

const char *PrWnd_getTitle(PrWnd wnd) {
	Mutex_lock(wnd->dataMux);
	int str16Len = GetWindowTextLengthW(wnd->hWnd);
	if (str16Len) {
		str16Len++;
		WCHAR *str16 = calloc(str16Len, sizeof(WCHAR));
		GetWindowTextW(wnd->hWnd, str16, str16Len);

		int str8Len = WideCharToMultiByte(CP_UTF8, 0, str16, -1, NULL, 0, NULL, NULL);
		_PrWnd_clearTitleBuf(wnd, str8Len);
		if (str8Len) {
			WideCharToMultiByte(CP_UTF8, 0, str16, -1, wnd->titleBuf, str8Len, NULL, NULL);
		}
		free(str16);
	} else {
		_PrWnd_clearTitleBuf(wnd, 0);
	}
	Mutex_unlock(wnd->dataMux);
	return (const char *)wnd->titleBuf;
}

void PrWnd_setTitle(PrWnd wnd, const char *str8) {
	if (!str8) {
		return;
	}
	int str16Len = MultiByteToWideChar(CP_UTF8, 0, str8, -1, NULL, 0);
	if (!str16Len) {
		return;
	}
	WCHAR *str16 = calloc(str16Len + 1, sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, str8, -1, str16, str16Len);

	SetWindowTextW(wnd->hWnd, str16);

	free(str16);
}

void PrWnd_size(PrWnd wnd, int *width, int *height) {
	RECT rect;
	GetClientRect(wnd->hWnd, &rect);
	if (width) {
		*width = rect.right;
	}
	if (height) {
		*height = rect.bottom;
	}
}

void PrWnd_resize(PrWnd wnd, int width, int height) {
	RECT rect;
	GetWindowRect(wnd->hWnd, &rect);
	MoveWindow(
		wnd->hWnd, rect.left, rect.top,
		width + wnd->ncWidth,
		height + wnd->ncHeight,
		TRUE
	);
}

#define _STYLE_HAS(_style) GetWindowLongW(wnd->hWnd, GWL_STYLE) & _style

void PrWnd_view(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_Visible:
			ShowWindow(wnd->hWnd, SW_SHOW);
			break;
		case PrView_Minimize:
			ShowWindow(wnd->hWnd, SW_MINIMIZE);
			break;
		case PrView_Maximize:
			ShowWindow(wnd->hWnd, SW_MAXIMIZE);
	}
}

void PrWnd_unview(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_Visible:
			ShowWindow(wnd->hWnd, SW_HIDE);
			break;
		case PrView_Minimize:
			if (_STYLE_HAS(WS_MINIMIZE)) {
				ShowWindow(wnd->hWnd, SW_RESTORE);
			}
			break;
		case PrView_Maximize:
			ShowWindow(wnd->hWnd, SW_SHOWNORMAL);
	}
}

bool PrWnd_viewed(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_Visible:
			return _STYLE_HAS(WS_VISIBLE);
		case PrView_Minimize:
			return _STYLE_HAS(WS_MINIMIZE);
		case PrView_Maximize:
			return _STYLE_HAS(WS_MAXIMIZE);
	}
	return 0;
}

#undef _STYLE_HAS

#endif // PWRE_WIN32
