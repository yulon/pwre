#include "plat.h"

#ifdef PWRE_PLAT_WIN32

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

#define _LOAD_W32_API(m, f) f##_fp = ((f##_t)GetProcAddress(m, #f))

static void pwre_free(pwre_wnd_t wnd) {
	if (wnd->on_free) {
		wnd->on_free(wnd);
	}
	ZKMux_Lock(wnd->data_mux);
	if (wnd->title_buf) {
		free(wnd->title_buf);
	}
	ZKMux_UnLock(wnd->data_mux);
	ZKMux_Free(wnd->data_mux);
	free(wnd);
}

#ifndef PWRE_PLAT_WIN32_WNDEXTRA
#define PWRE_PLAT_WIN32_WNDEXTRA 10
#endif

#ifndef PWRE_PLAT_WIN32_WNDEXTRA_I
#define PWRE_PLAT_WIN32_WNDEXTRA_I 6
#endif

static LRESULT CALLBACK wm_handler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	pwre_wnd_t wnd = (pwre_wnd_t)GetWindowLongPtrW(hWnd, PWRE_PLAT_WIN32_WNDEXTRA_I);
	if (wnd) {
		switch (uMsg) {
			case WM_NCCALCSIZE:
				if (wnd->less) {
					if (wParam) {
						((LPNCCALCSIZE_PARAMS)lParam)->rgrc[2] = ((LPNCCALCSIZE_PARAMS)lParam)->rgrc[1];
						((LPNCCALCSIZE_PARAMS)lParam)->rgrc[1] = ((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0];
					}
					return 0;
				}
				break;
			case WM_PAINT:
				_EVENT_POST(, PWRE_EVENT_PAINT, NULL)
				ValidateRect(hWnd, NULL);
				return 0;
			case WM_CLOSE:
				_EVENT_SEND(, PWRE_EVENT_CLOSE, NULL,
					return 0;
				)
				break;
			case WM_DESTROY:
				SetWindowLongPtrW(hWnd, PWRE_PLAT_WIN32_WNDEXTRA_I, 0);

				_EVENT_POST(, PWRE_EVENT_DESTROY, NULL)

				ZKMux_Lock(wnd_count_mux);
				wnd_count--;
				pwre_free(wnd);
				if (!wnd_count) {
					ZKMux_UnLock(wnd_count_mux);
					ZKMux_Free(wnd_count_mux);
					PostQuitMessage(0);
					return 0;
				}
				ZKMux_UnLock(wnd_count_mux);
		}
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static HMODULE main_module;
static WNDCLASSEXW wnd_class;

bool pwre_init(pwre_event_handler_t evt_hdr) {
	dwmapi = LoadLibraryW(L"dwmapi");
	if (dwmapi) {
		_LOAD_W32_API(dwmapi, DwmEnableBlurBehindWindow);
		_LOAD_W32_API(dwmapi, DwmExtendFrameIntoClientArea);

		gdi32 = LoadLibraryW(L"gdi32");
		if (gdi32) {
			_LOAD_W32_API(gdi32, CreateRectRgn);
		}
	}

	main_module = GetModuleHandleW(NULL);

	wnd_class.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wnd_class.hInstance = main_module;
	wnd_class.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
	wnd_class.hIconSm = LoadIconW(NULL, (LPCWSTR)IDI_WINLOGO);
	wnd_class.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	wnd_class.lpszClassName = L"PwreWindow";
	wnd_class.cbWndExtra = sizeof(void *) * PWRE_PLAT_WIN32_WNDEXTRA;
	wnd_class.lpfnWndProc = wm_handler;
	wnd_class.cbSize = sizeof(wnd_class);

	ATOM ok = RegisterClassExW(&wnd_class);
	if (!ok) {
		puts("Pwre: Win32.RegisterClassExW error!");
		return false;
	}

	wnd_count_mux = new_ZKMux();
	event_handler = evt_hdr;
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

pwre_wnd_t alloc_wnd(size_t struct_size, uint64_t hints) {
	RECT rect = { 500, 500, 1000, 1000 };
	BOOL ok = AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
	if (!ok) {
		puts("Pwre: Win32.AdjustWindowRectEx error!");
		return NULL;
	}

	pwre_wnd_t wnd = calloc(1, struct_size);

	wnd->nc_width = (500 - rect.left) + (rect.right - 1000);
	wnd->nc_height = (500 - rect.top) + (rect.bottom - 1000);

	wnd->hWnd = CreateWindowExW(
		0,
		wnd_class.lpszClassName,
		NULL,
		WS_OVERLAPPEDWINDOW,
		0, 0, 150 + wnd->nc_width, 150 + wnd->nc_height, NULL, NULL,
		main_module,
		wnd
	);
	if (!wnd->hWnd) {
		puts("Pwre: Win32.CreateWindowExW error!");
		free(wnd);
		return NULL;
	}

	ZKMux_Lock(wnd_count_mux); wnd_count++; ZKMux_UnLock(wnd_count_mux);

	wnd->data_mux = new_ZKMux();

	SetWindowLongPtrW(wnd->hWnd, PWRE_PLAT_WIN32_WNDEXTRA_I, (LONG_PTR)(wnd));

	if (((hints & PWRE_HINT_ALPHA) == PWRE_HINT_ALPHA) && DwmEnableBlurBehindWindow_fp && CreateRectRgn_fp) {
		DWM_BLURBEHIND bb;
		bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
		bb.hRgnBlur = CreateRectRgn_fp(0, 0, -1, -1);
		bb.fEnable = TRUE;
		bb.fTransitionOnMaximized = 1;
		DwmEnableBlurBehindWindow_fp(wnd->hWnd, &bb);
	} else
	if (((hints & PWRE_HINT_WMBACKGROUND) == PWRE_HINT_WMBACKGROUND) && DwmExtendFrameIntoClientArea_fp) {
		MARGINS mar_inset = { .cxLeftWidth = -1 };
		DwmExtendFrameIntoClientArea_fp(wnd->hWnd, &mar_inset);
	}

	return wnd;
}

pwre_wnd_t pwre_new_wnd(uint64_t hints) {
	return alloc_wnd(sizeof(struct pwre_wnd), hints);
}

void *pwre_wnd_native_id(pwre_wnd_t wnd) {
	return (void *)wnd->hWnd;
}

void pwre_wnd_close(pwre_wnd_t wnd) {
	CloseWindow(wnd->hWnd);
}

void pwre_wnd_destroy(pwre_wnd_t wnd) {
	DestroyWindow(wnd->hWnd);
}

const char *pwre_wnd_title(pwre_wnd_t wnd) {
	ZKMux_Lock(wnd->data_mux);
	int str16_len = GetWindowTextLengthW(wnd->hWnd);
	if (str16_len) {
		str16_len++;
		WCHAR *str16 = calloc(str16_len, sizeof(WCHAR));
		GetWindowTextW(wnd->hWnd, str16, str16_len);

		int str8_len = WideCharToMultiByte(CP_UTF8, 0, str16, -1, NULL, 0, NULL, NULL);
		wnd_title_buf_clear(wnd, str8_len);
		if (str8_len) {
			WideCharToMultiByte(CP_UTF8, 0, str16, -1, wnd->title_buf, str8_len, NULL, NULL);
		}
		free(str16);
	} else {
		wnd_title_buf_clear(wnd, 0);
	}
	ZKMux_UnLock(wnd->data_mux);
	return (const char *)wnd->title_buf;
}

void pwre_wnd_retitle(pwre_wnd_t wnd, const char *str8) {
	if (!str8) {
		return;
	}
	int str16_len = MultiByteToWideChar(CP_UTF8, 0, str8, -1, NULL, 0);
	if (!str16_len) {
		return;
	}
	WCHAR *str16 = calloc(str16_len + 1, sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, str8, -1, str16, str16_len);

	SetWindowTextW(wnd->hWnd, str16);

	free(str16);
}

void pwre_wnd_move(pwre_wnd_t wnd, int x, int y) {
	RECT rect;
	GetWindowRect(wnd->hWnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	fix_pos(&x, &y, width, height);
	MoveWindow(
		wnd->hWnd, x, y,
		width,
		height,
		FALSE
	);
}

void pwre_wnd_size(pwre_wnd_t wnd, int *width, int *height) {
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

void pwre_wnd_resize(pwre_wnd_t wnd, int width, int height) {
	RECT rect;
	GetWindowRect(wnd->hWnd, &rect);
	if (!wnd->less) {
		width += wnd->nc_width;
		height += wnd->nc_height;
	}
	MoveWindow(
		wnd->hWnd, rect.left, rect.top,
		width,
		height,
		TRUE
	);
}

#define _STYLE_HAS(_style) (GetWindowLongW(wnd->hWnd, GWL_STYLE) & _style) == _style

void pwre_wnd_state_add(pwre_wnd_t wnd, PWRE_STATE type) {
	switch (type) {
		case PWRE_STATE_VISIBLE:
			ShowWindow(wnd->hWnd, SW_SHOW);
			break;
		case PWRE_STATE_MINIMIZE:
			ShowWindow(wnd->hWnd, SW_MINIMIZE);
			break;
		case PWRE_STATE_MAXIMIZE:
			ShowWindow(wnd->hWnd, SW_MAXIMIZE);
	}
}

void pwre_wnd_state_rm(pwre_wnd_t wnd, PWRE_STATE type) {
	switch (type) {
		case PWRE_STATE_VISIBLE:
			ShowWindow(wnd->hWnd, SW_HIDE);
			break;
		case PWRE_STATE_MINIMIZE:
			if (_STYLE_HAS(WS_MINIMIZE)) {
				ShowWindow(wnd->hWnd, SW_RESTORE);
			}
			break;
		case PWRE_STATE_MAXIMIZE:
			ShowWindow(wnd->hWnd, SW_SHOWNORMAL);
	}
}

bool pwre_wnd_state_has(pwre_wnd_t wnd, PWRE_STATE type) {
	switch (type) {
		case PWRE_STATE_VISIBLE:
			return _STYLE_HAS(WS_VISIBLE);
		case PWRE_STATE_MINIMIZE:
			return _STYLE_HAS(WS_MINIMIZE);
		case PWRE_STATE_MAXIMIZE:
			return _STYLE_HAS(WS_MAXIMIZE);
	}
	return 0;
}

#undef _STYLE_HAS

void pwre_wnd_less(pwre_wnd_t wnd, bool less) {
	RECT rect;
	GetClientRect(wnd->hWnd, &rect);
	wnd->less = less;
	pwre_wnd_resize(wnd, rect.right, rect.bottom);
}

#endif // PWRE_PLAT_WIN32
