#include "pdbe.h"

#ifdef UBWINDOW_WIN32

#include "ubw.h"
#include <windows.h>

#ifndef UBWINDOW_WIN32_WNDEXTRA
#define UBWINDOW_WIN32_WNDEXTRA 10
#endif

#ifndef UBWINDOW_WIN32_WNDEXTRA_I
#define UBWINDOW_WIN32_WNDEXTRA_I 6
#endif

static LRESULT CALLBACK wndMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	_EVT_VARS((_UbwPvt)GetWindowLongPtrW(hWnd, UBWINDOW_WIN32_WNDEXTRA_I));
	if (wnd) {
		switch (uMsg) {
			case WM_PAINT:
				_EVT_SEND(, UBW_EVENT_PAINT, NULL,
					ValidateRect(hWnd, NULL);
					return 0;
				)
				break;
			case WM_CLOSE:
				_EVT_SEND(, UBW_EVENT_CLOSE, NULL,
					return 0;
				)
				break;
			case WM_DESTROY:
				_EVT_POST(, UBW_EVENT_DESTROY, NULL);
				wndCount--;
				free(wnd);
				if (!wndCount) {
					PostQuitMessage(0);
					return 0;
				}
		}
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static HMODULE mainModule;
static WNDCLASSEXW wndClass;

bool ubwInit(UbwEventHandler evtHdr) {
	mainModule = GetModuleHandleW(NULL);

	wndClass.style = CS_DBLCLKS | CS_OWNDC;
	wndClass.hInstance = mainModule;
	wndClass.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
	wndClass.hIconSm = LoadIconW(NULL, (LPCWSTR)IDI_WINLOGO);
	wndClass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.lpszClassName = L"UBWindow";
	wndClass.cbWndExtra = sizeof(void *) * UBWINDOW_WIN32_WNDEXTRA;
	wndClass.lpfnWndProc = wndMsgHandler;
	wndClass.cbSize = sizeof(wndClass);

	ATOM ok = RegisterClassExW(&wndClass);
	if (!ok) {
		puts("UBWindow: Win32.RegisterClassExW error!");
		return false;
	}

	dftEvtHdr = evtHdr;
	return true;
}

bool ubwStep(void) {
	MSG msg;
	if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
		return msg.message != WM_QUIT;
	}
	return true;
}

void ubwRun(void) {
	MSG msg = { .message = 0 };
	while (msg.message != WM_QUIT) {
		if (GetMessageW(&msg, NULL, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
}

Ubw ubwCreate(void) {
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
		puts("UBWindow: Win32.CreateWindowExW error!");
		return NULL;
	}

	RECT rect = { 500, 500, 1000, 1000 };
	BOOL ok = AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
	if (!ok) {
		puts("UBWindow: Win32.AdjustWindowRectEx error!");
		return 0;
	}

	wndCount++;
	_UbwPvt wnd = calloc(1, sizeof(struct _UbwPvt));
	wnd->ntvPtr = (void *)hWnd;
	wnd->evtHdr = dftEvtHdr;

	wnd->ncWidth = (500 - rect.left) + (rect.right - 1000);
	wnd->ncHeight = (500 - rect.top) + (rect.bottom - 1000);

	SetWindowLongPtrW(hWnd, UBWINDOW_WIN32_WNDEXTRA_I, (LONG_PTR)wnd);
	return (Ubw)wnd;
}

#define _HWND (HWND)((_UbwPvt)wnd)->ntvPtr

void ubwClose(Ubw wnd) {
	CloseWindow(_HWND);
}

void ubwDestroy(Ubw wnd) {
	DestroyWindow(_HWND);
}

int ubwGetTitle(Ubw wnd, char *str8) {
	int str16Len = GetWindowTextLengthW(_HWND);
	if (!str16Len) {
		return 0;
	}
	WCHAR *str16 = calloc(str16Len + 1, sizeof(WCHAR));
	GetWindowTextW(_HWND, str16, str16Len);

	int str8Len = WideCharToMultiByte(CP_UTF8, 0, str16, -1, NULL, 0, NULL, NULL);
	if (str8Len && str8) {
		WideCharToMultiByte(CP_UTF8, 0, str16, -1, str8, str8Len, NULL, NULL);
	}

	free(str16);
	return str8Len;
}

void ubwSetTitle(Ubw wnd, const char *str8) {
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

void ubwMoveToScreenCenter(Ubw wnd) {
	RECT rect;
	GetWindowRect(_HWND, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	MoveWindow(
		_HWND,
		(GetSystemMetrics(SM_CXSCREEN) - width) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - height) / 2,
		width - rect.left + ((_UbwPvt)wnd)->ncWidth,
		height - rect.top + ((_UbwPvt)wnd)->ncHeight,
		TRUE
	);
}

void ubwSize(Ubw wnd, int *width, int *height) {
	RECT rect;
	GetClientRect(_HWND, &rect);
	if (width) {
		*width = rect.right;
	}
	if (height) {
		*height = rect.bottom;
	}
}

void ubwResize(Ubw wnd, int width, int height) {
	RECT rect;
	GetWindowRect(_HWND, &rect);
	MoveWindow(
		_HWND, rect.left, rect.top,
		width + ((_UbwPvt)wnd)->ncWidth,
		height + ((_UbwPvt)wnd)->ncHeight,
		TRUE
	);
}

void ubwActive(Ubw wnd) {
	SetActiveWindow(_HWND);
}

void ubwShow(Ubw wnd) {
	ShowWindow(_HWND, SW_SHOW);
}

void ubwHide(Ubw wnd) {
	ShowWindow(_HWND, SW_HIDE);
}

int ubwIsVisible(Ubw wnd) {
	LONG style = GetWindowLongW(_HWND, GWL_STYLE);
	return style & WS_VISIBLE;
}

void ubwSetView(Ubw wnd, int flag) {
	switch (flag) {
		case UBW_VIEW_MAXIMIZE:
			ShowWindow(_HWND, SW_MAXIMIZE);
			break;
		case UBW_VIEW_MINIMIZE:
			ShowWindow(_HWND, SW_MINIMIZE);
			break;
		case UBW_VIEW_RESTORE:
			ShowWindow(_HWND, SW_RESTORE);
			break;
		case UBW_VIEW_NORMAL:
			ShowWindow(_HWND, SW_SHOWNORMAL);
	}
}

int ubwGetView(Ubw wnd) {
	LONG style = GetWindowLongW(_HWND, GWL_STYLE);
	if (style & WS_MAXIMIZE) {
		return UBW_VIEW_MAXIMIZE;
	}
	else if (style & WS_MAXIMIZE) {
		return UBW_VIEW_MINIMIZE;
	}
	return UBW_VIEW_NORMAL;
}

#undef _HWND
#endif // UBWINDOW_WIN32
