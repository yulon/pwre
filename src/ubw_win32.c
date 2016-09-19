#include "ubw.h"

#ifdef UBWINDOW_WIN32

#include <windows.h>

static HMODULE hProcess;
static WNDCLASSEXW wndCls;
static int lenWndExtra;

static LRESULT CALLBACK ubwProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_DESTROY) {
		ubwSum--;
		if (!ubwSum) {
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static DWORD dwExStyle;
static DWORD dwStyle;

int ubwInit() {
	hProcess = GetModuleHandleW(NULL);
	HICON hiApp = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
	HICON hiLogo = LoadIconW(NULL, (LPCWSTR)IDI_WINLOGO);
	HCURSOR hcArrow = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);

	wndCls.style = CS_DBLCLKS;
	wndCls.hInstance = hProcess;
	wndCls.hIcon = hiApp;
	wndCls.hIconSm = hiLogo;
	wndCls.hCursor = hcArrow;
	wndCls.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndCls.lpszClassName = L"UBWindow";
	lenWndExtra = 10;
	wndCls.cbWndExtra = sizeof(void*) * lenWndExtra;
	wndCls.lpfnWndProc = ubwProc;
	wndCls.cbSize = sizeof(wndCls);

	ATOM ok = RegisterClassExW(&wndCls);
	if (!ok) {
		puts("UBWindow: Win32.RegisterClassExW error!");
		return 0;
	}

	dwExStyle = WS_EX_DLGMODALFRAME;
	dwStyle = WS_CAPTION | WS_SYSMENU | WS_OVERLAPPED | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

	return 1;
}

static MSG msg;

int ubwHandleEvent() {
	if (!GetMessageW(&msg, NULL, 0, 0))
	{
		return 0;
	}
	TranslateMessage(&msg);
	DispatchMessageW(&msg);
	return 1;
}

Ubw ubwCreate() {
	_Ubw* wnd = calloc(1, sizeof(_Ubw));

	wnd->pNtv = (void*)CreateWindowExW(
		dwExStyle,
		wndCls.lpszClassName,
		NULL,
		dwStyle,
		0, 0, 0, 0, NULL, NULL,
		hProcess,
		NULL
	);
	if (!wnd->pNtv) {
		puts("UBWindow: Win32.CreateWindowExW error!");
		return NULL;
	}

	RECT bdNonCont = { 500, 500, 1000, 1000 };
	BOOL ok = AdjustWindowRectEx(&bdNonCont, dwStyle, FALSE, dwExStyle);
	if (!ok) {
		puts("UBWindow: Win32.AdjustWindowRectEx error!");
		return 0;
	}
	wnd->ncWidth = (500 - bdNonCont.left) + (bdNonCont.right - 1000);
	wnd->ncHeight = (500 - bdNonCont.top) + (bdNonCont.bottom - 1000);

	ubwSum++;
	SetWindowLongPtr((HWND)wnd->pNtv, lenWndExtra - 1, (LONG_PTR)wnd);
	return (Ubw)wnd;
}

#define _UBW_HWND (HWND)((_Ubw*)wnd)->pNtv

int ubwGetTitle(Ubw wnd, char* str8) {
	int lenStr16 = GetWindowTextLengthW(_UBW_HWND);
	if (!lenStr16) {
		return 0;
	}
	WCHAR* str16 = calloc(lenStr16 + 1, sizeof(WCHAR));
	GetWindowTextW(_UBW_HWND, str16, lenStr16);

	int lenStr8 = WideCharToMultiByte(CP_UTF8, 0, str16, -1, NULL, 0, NULL, NULL);
	if (lenStr8 && str8) {
		WideCharToMultiByte(CP_UTF8, 0, str16, -1, str8, lenStr8, NULL, NULL);
	}

	free(str16);
	return lenStr8;
}

int ubwSetTitle(Ubw wnd, char* str8) {
	if (!str8) {
		return 0;
	}
	int lenStr16 = MultiByteToWideChar(CP_UTF8, 0, str8, -1, NULL, 0);
	if (!lenStr16) {
		return lenStr16;
	}
	WCHAR* str16 = calloc(lenStr16 + 1, sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, str8, -1, str16, lenStr16);

	BOOL ok = SetWindowTextW(_UBW_HWND, str16);

	free(str16);
	return ok;
}

void ubwMoveToScreenCenter(Ubw wnd) {
	RECT rect;
	GetWindowRect(_UBW_HWND, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	MoveWindow(
		_UBW_HWND,
		(GetSystemMetrics(SM_CXSCREEN) - width) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - height) / 2,
		width - rect.left + ((_Ubw*)wnd)->ncWidth,
		height - rect.top + ((_Ubw*)wnd)->ncHeight,
		TRUE
	);
}

void ubwSize(Ubw wnd, int* width, int* height) {
	RECT rect;
	GetWindowRect(_UBW_HWND, &rect);
	if (width) {
		*width = rect.right - rect.left - ((_Ubw*)wnd)->ncWidth;
	}
	if (height) {
		*height = rect.bottom - rect.top - ((_Ubw*)wnd)->ncHeight;
	}
}

void ubwResize(Ubw wnd, int width, int height) {
	RECT rect;
	GetWindowRect(_UBW_HWND, &rect);
	MoveWindow(
		_UBW_HWND, rect.left, rect.top,
		width + ((_Ubw*)wnd)->ncWidth,
		height + ((_Ubw*)wnd)->ncHeight,
		TRUE
	);
}

void ubwActive(Ubw wnd) {
	SetActiveWindow(_UBW_HWND);
}

void ubwShow(Ubw wnd) {
	ShowWindow(_UBW_HWND, SW_SHOW);
}

void ubwHide(Ubw wnd) {
	ShowWindow(_UBW_HWND, SW_HIDE);
}

int ubwIsVisible(Ubw wnd) {
	LONG dwStyle = GetWindowLongW(_UBW_HWND, GWL_STYLE);
	return dwStyle & WS_VISIBLE;
}

void ubwSetView(Ubw wnd, int flag) {
	int nCmdShow = 0;
	switch (flag)
	{
	case UBW_VIEW_MAXIMIZE:
		nCmdShow = SW_MAXIMIZE;
		break;
	case UBW_VIEW_MINIMIZE:
		nCmdShow = SW_MINIMIZE;
		break;
	case UBW_VIEW_RESTORE:
		nCmdShow = SW_RESTORE;
		break;
	case UBW_VIEW_NORMAL:
		nCmdShow = SW_SHOWNORMAL;
	}
	if (nCmdShow) {
		ShowWindow(_UBW_HWND, nCmdShow);
	}
}

int ubwGetView(Ubw wnd) {
	LONG dwStyle = GetWindowLongW(_UBW_HWND, GWL_STYLE);
	if (dwStyle & WS_MAXIMIZE) {
		return UBW_VIEW_MAXIMIZE;
	}
	else if (dwStyle & WS_MAXIMIZE) {
		return UBW_VIEW_MINIMIZE;
	}
	return UBW_VIEW_NORMAL;
}

#undef _UBW_HWND
#endif // UBWINDOW_WIN32
