#include "pdbe.h"

#ifdef PWRE_BE_WIN32

#include "uni.h"
#include <windows.h>

#ifndef PWRE_BE_WIN32_WNDEXTRA
#define PWRE_BE_WIN32_WNDEXTRA 10
#endif

#ifndef PWRE_BE_WIN32_WNDEXTRA_I
#define PWRE_BE_WIN32_WNDEXTRA_I 6
#endif

static LRESULT CALLBACK wndMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	_EVT_VARS((PrWndPvt)GetWindowLongPtrW(hWnd, PWRE_BE_WIN32_WNDEXTRA_I));
	if (wnd) {
		switch (uMsg) {
			case WM_PAINT:
				_EVT_SEND(, PrEvent_paint, NULL,
					ValidateRect(hWnd, NULL);
					return 0;
				)
				break;
			case WM_CLOSE:
				_EVT_SEND(, PrEvent_close, NULL,
					return 0;
				)
				break;
			case WM_DESTROY:
				_EVT_POST(, PrEvent_destroy, NULL);
				wndCount--;
				if (!wndCount) {
					PostQuitMessage(0);
					return 0;
				}
				PrWndPvt_free(wnd);
		}
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static HMODULE mainModule;
static WNDCLASSEXW wndClass;

bool pwreInit(PrEventHandler evtHdr) {
	mainModule = GetModuleHandleW(NULL);

	wndClass.style = CS_DBLCLKS | CS_OWNDC;
	wndClass.hInstance = mainModule;
	wndClass.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
	wndClass.hIconSm = LoadIconW(NULL, (LPCWSTR)IDI_WINLOGO);
	wndClass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.lpszClassName = L"PwreWindow";
	wndClass.cbWndExtra = sizeof(void *) * PWRE_BE_WIN32_WNDEXTRA;
	wndClass.lpfnWndProc = wndMsgHandler;
	wndClass.cbSize = sizeof(wndClass);

	ATOM ok = RegisterClassExW(&wndClass);
	if (!ok) {
		puts("Pwre: Win32.RegisterClassExW error!");
		return false;
	}

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

PrWnd new_PrWnd(void) {
	HWND hWnd = CreateWindowExW(
		0,
		wndClass.lpszClassName,
		NULL,
		WS_OVERLAPPEDWINDOW,
		0, 0, 0, 0, NULL, NULL,
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

	wndCount++;
	PrWndPvt wnd = new_PrWndPvt();
	wnd->ntvPtr = (void *)hWnd;
	wnd->evtHdr = dftEvtHdr;

	wnd->ncWidth = (500 - rect.left) + (rect.right - 1000);
	wnd->ncHeight = (500 - rect.top) + (rect.bottom - 1000);

	SetWindowLongPtrW(hWnd, PWRE_BE_WIN32_WNDEXTRA_I, (LONG_PTR)wnd);
	return (PrWnd)wnd;
}

#define _HWND (HWND)((PrWndPvt)wnd)->ntvPtr

void PrWnd_close(PrWnd wnd) {
	CloseWindow(_HWND);
}

void PrWnd_destroy(PrWnd wnd) {
	DestroyWindow(_HWND);
}

const char *PrWnd_getTitle(PrWnd wnd) {
	int str16Len = GetWindowTextLengthW(_HWND);
	if (str16Len) {
		str16Len++;
		WCHAR *str16 = calloc(str16Len, sizeof(WCHAR));
		GetWindowTextW(_HWND, str16, str16Len);

		int str8Len = WideCharToMultiByte(CP_UTF8, 0, str16, -1, NULL, 0, NULL, NULL);
		PrWndPvt_blankTitle((PrWndPvt)wnd, str8Len);
		if (str8Len) {
			WideCharToMultiByte(CP_UTF8, 0, str16, -1, ((PrWndPvt)wnd)->titleBuf, str8Len, NULL, NULL);
		}
		free(str16);
	} else {
		PrWndPvt_blankTitle((PrWndPvt)wnd, 0);
	}
	return (const char *)((PrWndPvt)wnd)->titleBuf;
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

	SetWindowTextW(_HWND, str16);

	free(str16);
}

void PrWnd_moveToScreenCenter(PrWnd wnd) {
	RECT rect;
	GetWindowRect(_HWND, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	MoveWindow(
		_HWND,
		(GetSystemMetrics(SM_CXSCREEN) - width) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - height) / 2,
		width - rect.left + ((PrWndPvt)wnd)->ncWidth,
		height - rect.top + ((PrWndPvt)wnd)->ncHeight,
		TRUE
	);
}

void PrWnd_size(PrWnd wnd, int *width, int *height) {
	RECT rect;
	GetClientRect(_HWND, &rect);
	if (width) {
		*width = rect.right;
	}
	if (height) {
		*height = rect.bottom;
	}
}

void PrWnd_resize(PrWnd wnd, int width, int height) {
	RECT rect;
	GetWindowRect(_HWND, &rect);
	MoveWindow(
		_HWND, rect.left, rect.top,
		width + ((PrWndPvt)wnd)->ncWidth,
		height + ((PrWndPvt)wnd)->ncHeight,
		TRUE
	);
}

#define _STYLE_HAS(_style) GetWindowLongW(_HWND, GWL_STYLE) & _style

void PrWnd_view(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_visible:
			ShowWindow(_HWND, SW_SHOW);
			break;
		case PrView_minimize:
			ShowWindow(_HWND, SW_MINIMIZE);
			break;
		case PrView_maximize:
			ShowWindow(_HWND, SW_MAXIMIZE);
	}
}

void PrWnd_unview(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_visible:
			ShowWindow(_HWND, SW_HIDE);
			break;
		case PrView_minimize:
			if (_STYLE_HAS(WS_MINIMIZE)) {
				ShowWindow(_HWND, SW_RESTORE);
			}
			break;
		case PrView_maximize:
			ShowWindow(_HWND, SW_SHOWNORMAL);
	}
}

bool PrWnd_viewed(PrWnd wnd, PrView type) {
	switch (type) {
		case PrView_visible:
			return _STYLE_HAS(WS_VISIBLE);
		case PrView_minimize:
			return _STYLE_HAS(WS_MINIMIZE);
		case PrView_maximize:
			return _STYLE_HAS(WS_MAXIMIZE);
	}
	return 0;
}

#undef _STYLE_HAS

#undef _HWND
#endif // PWRE_BE_WIN32
