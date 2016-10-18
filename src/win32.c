#include "plat.h"

#ifdef PWRE_WIN32

#define ZK_SCOPE pwre
#define ZK_IMPL

#include "win32.h"
#include "uni.h"
#include "titlebuf.h"

static HMODULE dwmapi;
typedef HRESULT (WINAPI *DwmEnableBlurBehindWindow_t)(HWND hWnd, const DWM_BLURBEHIND *pBlurBehind);
static DwmEnableBlurBehindWindow_t DwmEnableBlurBehindWindow_fp;
typedef HRESULT (WINAPI *DwmExtendFrameIntoClientArea_t)(HWND hWnd, const MARGINS *pMarInset);
static DwmExtendFrameIntoClientArea_t DwmExtendFrameIntoClientArea_fp;

static HMODULE gdi32;
typedef HRGN (WINAPI *CreateRectRgn_t)(int x1, int y1, int x2, int y2);
static CreateRectRgn_t CreateRectRgn_fp;

#define loadW32Api(m, f) f##_fp = ((f##_t)GetProcAddress(m, #f))

static void _PrWnd_free(PrWnd wnd) {
	if (wnd->onFree) {
		wnd->onFree(wnd);
	}
	ZKMux_Lock(wnd->dataMux);
	if (wnd->titleBuf) {
		free(wnd->titleBuf);
	}
	ZKMux_UnLock(wnd->dataMux);
	ZKMux_Free(wnd->dataMux);
	free(wnd);
}

#ifndef PWRE_WIN32_WNDEXTRA
#define PWRE_WIN32_WNDEXTRA 10
#endif

#ifndef PWRE_WIN32_WNDEXTRA_I
#define PWRE_WIN32_WNDEXTRA_I 6
#endif

static LRESULT CALLBACK wndMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	PrWnd wnd = (PrWnd)GetWindowLongPtrW(hWnd, PWRE_WIN32_WNDEXTRA_I);
	if (wnd) {
		switch (uMsg) {
			case WM_NCCALCSIZE:
				if (wnd->less) {
					if (wParam) {
						memcpy(&((LPNCCALCSIZE_PARAMS)lParam)->rgrc[2], &((LPNCCALCSIZE_PARAMS)lParam)->rgrc[1], sizeof(RECT));
						memcpy(&((LPNCCALCSIZE_PARAMS)lParam)->rgrc[1], &((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0], sizeof(RECT));
					}
					return 0;
				}
				break;
			case WM_PAINT:
				eventPost(, PWRE_EVENT_PAINT, NULL)
				ValidateRect(hWnd, NULL);
				return 0;
			case WM_CLOSE:
				eventSend(, PWRE_EVENT_CLOSE, NULL,
					return 0;
				)
				break;
			case WM_DESTROY:
				SetWindowLongPtrW(hWnd, PWRE_WIN32_WNDEXTRA_I, 0);

				eventPost(, PWRE_EVENT_DESTROY, NULL)

				ZKMux_Lock(wndCountMux);
				wndCount--;
				_PrWnd_free(wnd);
				if (!wndCount) {
					ZKMux_UnLock(wndCountMux);
					ZKMux_Free(wndCountMux);
					PostQuitMessage(0);
					return 0;
				}
				ZKMux_UnLock(wndCountMux);
		}
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static HMODULE mainModule;
static WNDCLASSEXW wndClass;

bool pwre_init(PrEventHandler evtHdr) {
	dwmapi = LoadLibraryW(L"dwmapi");
	if (dwmapi) {
		loadW32Api(dwmapi, DwmEnableBlurBehindWindow);
		loadW32Api(dwmapi, DwmExtendFrameIntoClientArea);

		gdi32 = LoadLibraryW(L"gdi32");
		if (gdi32) {
			loadW32Api(gdi32, CreateRectRgn);
		}
	}

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

	wndCountMux = new_ZKMux();
	eventHandler = evtHdr;
	return true;
}

bool pwre_step(void) {
	MSG msg;
	while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
		if (msg.message == WM_QUIT) {
			return false;
		}
	}
	return true;
}

void pwre_run(void) {
	MSG msg = { .message = 0 };
	while (msg.message != WM_QUIT && (GetMessageW(&msg, NULL, 0, 0) > 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

static void fixPos(int *x, int *y, int width, int height) {
	if (*x == PWRE_POS_AUTO) {
		*x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	}
	if (*y == PWRE_POS_AUTO) {
		*y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	}
}

PrWnd _alloc_PrWnd(size_t memSize, uint64_t hints) {
	RECT rect = { 500, 500, 1000, 1000 };
	BOOL ok = AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
	if (!ok) {
		puts("Pwre: Win32.AdjustWindowRectEx error!");
		return NULL;
	}

	PrWnd wnd = calloc(1, memSize);

	wnd->ncWidth = (500 - rect.left) + (rect.right - 1000);
	wnd->ncHeight = (500 - rect.top) + (rect.bottom - 1000);

	wnd->hWnd = CreateWindowExW(
		0,
		wndClass.lpszClassName,
		NULL,
		WS_OVERLAPPEDWINDOW,
		0, 0, 150 + wnd->ncWidth, 150 + wnd->ncHeight, NULL, NULL,
		mainModule,
		wnd
	);
	if (!wnd->hWnd) {
		puts("Pwre: Win32.CreateWindowExW error!");
		free(wnd);
		return NULL;
	}

	ZKMux_Lock(wndCountMux); wndCount++; ZKMux_UnLock(wndCountMux);

	wnd->dataMux = new_ZKMux();

	SetWindowLongPtrW(wnd->hWnd, PWRE_WIN32_WNDEXTRA_I, (LONG_PTR)(wnd));

	if (((hints & PWRE_HINT_ALPHA) == PWRE_HINT_ALPHA) && DwmEnableBlurBehindWindow_fp && CreateRectRgn_fp) {
		DWM_BLURBEHIND bb;
		bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
		bb.hRgnBlur = CreateRectRgn_fp(0, 0, -1, -1);
		bb.fEnable = TRUE;
		bb.fTransitionOnMaximized = 1;
		DwmEnableBlurBehindWindow_fp(wnd->hWnd, &bb);
	} else
	if (((hints & PWRE_HINT_WMBACKGROUND) == PWRE_HINT_WMBACKGROUND) && DwmExtendFrameIntoClientArea_fp) {
		MARGINS marInset = { -1 };
		DwmExtendFrameIntoClientArea_fp(wnd->hWnd, &marInset);
	}

	return wnd;
}

PrWnd new_PrWnd(uint64_t hints) {
	return _alloc_PrWnd(sizeof(struct PrWnd), hints);
}

void *PrWnd_NativePointer(PrWnd wnd) {
	return (void *)wnd->hWnd;
}

void PrWnd_Close(PrWnd wnd) {
	CloseWindow(wnd->hWnd);
}

void PrWnd_Destroy(PrWnd wnd) {
	DestroyWindow(wnd->hWnd);
}

const char *PrWnd_GetTitle(PrWnd wnd) {
	ZKMux_Lock(wnd->dataMux);
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
	ZKMux_UnLock(wnd->dataMux);
	return (const char *)wnd->titleBuf;
}

void PrWnd_SetTitle(PrWnd wnd, const char *str8) {
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

void PrWnd_Move(PrWnd wnd, int x, int y) {
	RECT rect;
	GetWindowRect(wnd->hWnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	fixPos(&x, &y, width, height);
	MoveWindow(
		wnd->hWnd, x, y,
		width,
		height,
		FALSE
	);
}

void PrWnd_Size(PrWnd wnd, int *width, int *height) {
	RECT rect;
	if (wnd->less) {
		GetWindowRect(wnd->hWnd, &rect);
		rect.right -= rect.left;
		rect.bottom -= rect.top;
	} else {
		GetClientRect(wnd->hWnd, &rect);
	}
	if (width) {
		*width = rect.right;
	}
	if (height) {
		*height = rect.bottom;
	}
}

void PrWnd_ReSize(PrWnd wnd, int width, int height) {
	RECT rect;
	GetWindowRect(wnd->hWnd, &rect);
	if (!wnd->less) {
		width += wnd->ncWidth;
		height += wnd->ncHeight;
	}
	MoveWindow(
		wnd->hWnd, rect.left, rect.top,
		width,
		height,
		TRUE
	);
}

#define styleHas(_style) (GetWindowLongW(wnd->hWnd, GWL_STYLE) & _style) == _style

void PrWnd_View(PrWnd wnd, PWRE_VIEW type) {
	switch (type) {
		case PWRE_VIEW_VISIBLE:
			ShowWindow(wnd->hWnd, SW_SHOW);
			break;
		case PWRE_VIEW_MINIMIZE:
			ShowWindow(wnd->hWnd, SW_MINIMIZE);
			break;
		case PWRE_VIEW_MAXIMIZE:
			ShowWindow(wnd->hWnd, SW_MAXIMIZE);
	}
}

void PrWnd_UnView(PrWnd wnd, PWRE_VIEW type) {
	switch (type) {
		case PWRE_VIEW_VISIBLE:
			ShowWindow(wnd->hWnd, SW_HIDE);
			break;
		case PWRE_VIEW_MINIMIZE:
			if (styleHas(WS_MINIMIZE)) {
				ShowWindow(wnd->hWnd, SW_RESTORE);
			}
			break;
		case PWRE_VIEW_MAXIMIZE:
			ShowWindow(wnd->hWnd, SW_SHOWNORMAL);
	}
}

bool PrWnd_Viewed(PrWnd wnd, PWRE_VIEW type) {
	switch (type) {
		case PWRE_VIEW_VISIBLE:
			return styleHas(WS_VISIBLE);
		case PWRE_VIEW_MINIMIZE:
			return styleHas(WS_MINIMIZE);
		case PWRE_VIEW_MAXIMIZE:
			return styleHas(WS_MAXIMIZE);
	}
	return 0;
}

#undef styleHas

void PrWnd_Less(PrWnd wnd, bool less) {
	RECT rect;
	GetClientRect(wnd->hWnd, &rect);
	wnd->less = less;
	PrWnd_ReSize(wnd, rect.right, rect.bottom);
}

#endif // PWRE_WIN32
