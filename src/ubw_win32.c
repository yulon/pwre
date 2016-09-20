#include "ubw.h"

#ifdef UBWINDOW_WIN32

#include <windows.h>

static HMODULE mainModule;
static WNDCLASSEXW wndClass;
static int wndExtraLen;

static LRESULT CALLBACK wmHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_DESTROY) {
		wndCount--;
		if (!wndCount) {
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static DWORD baseExStyle;
static DWORD baseStyle;

int ubwInit(void) {
	mainModule = GetModuleHandleW(NULL);

	wndClass.style = CS_DBLCLKS;
	wndClass.hInstance = mainModule;
	wndClass.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
	wndClass.hIconSm = LoadIconW(NULL, (LPCWSTR)IDI_WINLOGO);
	wndClass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.lpszClassName = L"UBWindow";
	wndExtraLen = 10;
	wndClass.cbWndExtra = sizeof(void *) * wndExtraLen;
	wndClass.lpfnWndProc = wmHandler;
	wndClass.cbSize = sizeof(wndClass);

	ATOM ok = RegisterClassExW(&wndClass);
	if (!ok) {
		puts("UBWindow: Win32.RegisterClassExW error!");
		return 0;
	}

	baseExStyle = WS_EX_DLGMODALFRAME;
	baseStyle = WS_CAPTION | WS_SYSMENU | WS_OVERLAPPED | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

	return 1;
}

static MSG msg;

int ubwHandleEvent(void) {
	if (!GetMessageW(&msg, NULL, 0, 0))
	{
		return 0;
	}
	TranslateMessage(&msg);
	DispatchMessageW(&msg);
	return 1;
}

UBW ubwCreate(void) {
	_UBWPVT *wnd = calloc(1, sizeof(_UBWPVT));

	wnd->ntvPtr = (void *)CreateWindowExW(
		baseExStyle,
		wndClass.lpszClassName,
		NULL,
		baseStyle,
		0, 0, 0, 0, NULL, NULL,
		mainModule,
		NULL
	);
	if (!wnd->ntvPtr) {
		puts("UBWindow: Win32.CreateWindowExW error!");
		return NULL;
	}

	RECT rect = { 500, 500, 1000, 1000 };
	BOOL ok = AdjustWindowRectEx(&rect, baseStyle, FALSE, baseExStyle);
	if (!ok) {
		puts("UBWindow: Win32.AdjustWindowRectEx error!");
		return 0;
	}
	wnd->ncWidth = (500 - rect.left) + (rect.right - 1000);
	wnd->ncHeight = (500 - rect.top) + (rect.bottom - 1000);

	wndCount++;
	SetWindowLongPtr((HWND)wnd->ntvPtr, wndExtraLen - 1, (LONG_PTR)wnd);
	return (UBW)wnd;
}

#define _HWND (HWND)((_UBWPVT *)wnd)->ntvPtr

int ubwGetTitle(UBW wnd, char *str8) {
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

void ubwSetTitle(UBW wnd, const char *str8) {
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
	return;
}

void ubwMoveToScreenCenter(UBW wnd) {
	RECT rect;
	GetWindowRect(_HWND, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	MoveWindow(
		_HWND,
		(GetSystemMetrics(SM_CXSCREEN) - width) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - height) / 2,
		width - rect.left + ((_UBWPVT *)wnd)->ncWidth,
		height - rect.top + ((_UBWPVT *)wnd)->ncHeight,
		TRUE
	);
}

void ubwSize(UBW wnd, int *width, int *height) {
	RECT rect;
	GetWindowRect(_HWND, &rect);
	if (width) {
		*width = rect.right - rect.left - ((_UBWPVT *)wnd)->ncWidth;
	}
	if (height) {
		*height = rect.bottom - rect.top - ((_UBWPVT *)wnd)->ncHeight;
	}
}

void ubwResize(UBW wnd, int width, int height) {
	RECT rect;
	GetWindowRect(_HWND, &rect);
	MoveWindow(
		_HWND, rect.left, rect.top,
		width + ((_UBWPVT *)wnd)->ncWidth,
		height + ((_UBWPVT *)wnd)->ncHeight,
		TRUE
	);
}

void ubwActive(UBW wnd) {
	SetActiveWindow(_HWND);
}

void ubwShow(UBW wnd) {
	ShowWindow(_HWND, SW_SHOW);
}

void ubwHide(UBW wnd) {
	ShowWindow(_HWND, SW_HIDE);
}

int ubwIsVisible(UBW wnd) {
	LONG style = GetWindowLongW(_HWND, GWL_STYLE);
	return style & WS_VISIBLE;
}

void ubwSetView(UBW wnd, int flag) {
	switch (flag)
	{
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

int ubwGetView(UBW wnd) {
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
