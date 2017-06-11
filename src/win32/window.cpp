#include <pwre.hpp>

#ifdef PWRE_PLAT_WIN32

#include <dwmapi.h>

#include <cassert>
#include "../uassert.h"

namespace pwre {
	auto dwm = LoadLibraryW(L"dwmapi");
	auto DwmEnableBlurBehindWindow_p = (HRESULT (WINAPI *)(HWND hWnd, const DWM_BLURBEHIND *pBlurBehind))GetProcAddress(dwm, "DwmEnableBlurBehindWindow");
	auto DwmExtendFrameIntoClientArea_p = (HRESULT (WINAPI *)(HWND hWnd, const MARGINS *pMarInset))GetProcAddress(dwm, "DwmExtendFrameIntoClientArea");

	auto gdi = LoadLibraryW(L"gdi32");
	auto CreateRectRgn_p = (HRGN (WINAPI *)(int x1, int y1, int x2, int y2))GetProcAddress(gdi, "CreateRectRgn");

	size_t count = 0;
	bool life = true;

	HMODULE host;
	WNDCLASSEXW cls;

	#ifndef PWRE_PLAT_WIN32_WNDEXTRA
	#define PWRE_PLAT_WIN32_WNDEXTRA 10
	#endif

	#ifndef PWRE_PLAT_WIN32_WNDEXTRA_I
	#define PWRE_PLAT_WIN32_WNDEXTRA_I 6
	#endif

	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		auto wnd = (window *)GetWindowLongPtrW(hWnd, PWRE_PLAT_WIN32_WNDEXTRA_I);
		if (wnd) {
			switch (uMsg) {
				case WM_NCCALCSIZE:
					if (wnd->_lessed) {
						if (wParam) {
							((LPNCCALCSIZE_PARAMS)lParam)->rgrc[2] = ((LPNCCALCSIZE_PARAMS)lParam)->rgrc[1];
							((LPNCCALCSIZE_PARAMS)lParam)->rgrc[1] = ((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0];
						}
						return 0;
					}
					break;
				case WM_PAINT:
					wnd->on_paint.calls();
					ValidateRect(hWnd, NULL);
					return 0;
				case WM_ERASEBKGND:
					return 0;
				case WM_CLOSE:
					if (!wnd->on_close.calls()) {
						return 0;
					}
					break;
				case WM_DESTROY:
					SetWindowLongPtrW(hWnd, PWRE_PLAT_WIN32_WNDEXTRA_I, 0);
					wnd->on_destroy.calls();
					wnd->_nwnd = NULL;
					if (!--count) {
						life = false;
					}
			}
		}
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	void init_win32() {
		host = GetModuleHandleW(NULL);

		cls.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		cls.hInstance = host;
		cls.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
		cls.hIconSm = LoadIconW(NULL, (LPCWSTR)IDI_WINLOGO);
		cls.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
		cls.lpszClassName = L"pwre::window";
		cls.cbWndExtra = sizeof(void *) * PWRE_PLAT_WIN32_WNDEXTRA;
		cls.lpfnWndProc = WndProc;
		cls.cbSize = sizeof(cls);

		ATOM ok = RegisterClassExW(&cls);
		uassert(ok, "Pwre", "RegisterClassExW");
	}

	bool checkout_events() {
		MSG msg;
		while (life && PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return life;
	}

	bool recv_event() {
		MSG msg;
		if (life && GetMessageW(&msg, NULL, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return life;
	}

	#define _SCREEN_W (GetSystemMetrics(SM_CXSCREEN))
	#define _SCREEN_H (GetSystemMetrics(SM_CYSCREEN))
	#include "../fix_pos.hpp"

	window::window(uint64_t hints) {
		_lessed = false;

		RECT rect {500, 500, 1000, 1000};
		BOOL ok = AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
		if (!ok) {
			_nwnd = NULL;
			return;
		}

		_nc_width = (500 - rect.left) + (rect.right - 1000);
		_nc_height = (500 - rect.top) + (rect.bottom - 1000);

		_nwnd = CreateWindowExW(
			0,
			cls.lpszClassName,
			NULL,
			WS_OVERLAPPEDWINDOW,
			0, 0, 150 + _nc_width, 150 + _nc_height, NULL, NULL,
			host,
			(LPVOID)this
		);
		if (!_nwnd) {
			return;
		}

		++count;

		SetWindowLongPtrW(_nwnd, PWRE_PLAT_WIN32_WNDEXTRA_I, (LONG_PTR)(this));

		if (dwm) {
			if ((hints & PWRE_HINT_ALPHA) == PWRE_HINT_ALPHA && gdi) {
				DWM_BLURBEHIND bb;
				bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
				bb.hRgnBlur = CreateRectRgn_p(0, 0, -1, -1);
				bb.fEnable = TRUE;
				bb.fTransitionOnMaximized = 1;
				DwmEnableBlurBehindWindow_p(_nwnd, &bb);
			} else
			if (((hints & PWRE_HINT_WMBACKGROUND) == PWRE_HINT_WMBACKGROUND)) {
				MARGINS mar_inset;
				mar_inset.cxLeftWidth = -1;
				DwmExtendFrameIntoClientArea_p(_nwnd, &mar_inset);
			}
		}
	}

	void window::close() {
		CloseWindow(_nwnd);
	}

	void window::destroy() {
		DestroyWindow(_nwnd);
	}

	std::string window::title() {
		std::string val;

		int wchars_len = GetWindowTextLengthW(_nwnd);
		if (wchars_len) {
			wchars_len++;
			WCHAR *wchars = new WCHAR[wchars_len];
			GetWindowTextW(_nwnd, wchars, wchars_len);

			int chars_len = WideCharToMultiByte(CP_UTF8, 0, wchars, -1, NULL, 0, NULL, NULL);
			if (chars_len) {
				char *chars = new char[chars_len + 1];
				chars[chars_len] = 0;
				WideCharToMultiByte(CP_UTF8, 0, wchars, -1, chars, chars_len, NULL, NULL);
				val = chars;
				delete[] chars;
			}

			delete[] wchars;
		}

		return val;
	}

	void window::retitle(const std::string &title) {
		if (title.size() == 0) {
			return;
		}
		const char *chars = title.c_str();

		int wchars_len = MultiByteToWideChar(CP_UTF8, 0, chars, -1, NULL, 0);
		if (!wchars_len) {
			return;
		}
		WCHAR *wchars = new WCHAR[wchars_len + 1];
		MultiByteToWideChar(CP_UTF8, 0, chars, -1, wchars, wchars_len);

		SetWindowTextW(_nwnd, wchars);

		delete[] wchars;
	}

	window::pos_type window::pos() {
		RECT rect;
		GetWindowRect(_nwnd, &rect);
		return {rect.left, rect.top};
	}

	void window::move(window::pos_type pos) {
		RECT rect;
		GetWindowRect(_nwnd, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		fix_pos(pos.x, pos.y, rect.left, rect.top, width, height);
		MoveWindow(
			_nwnd, pos.x, pos.y,
			width,
			height,
			FALSE
		);
	}

	window::size_type window::size() {
		RECT rect;
		if (_lessed) {
			GetWindowRect(_nwnd, &rect);
			rect.right -= rect.left;
			rect.bottom -= rect.top;
		} else {
			GetClientRect(_nwnd, &rect);
		}
		return {rect.right, rect.bottom};
	}

	void window::resize(window::size_type sz) {
		RECT rect;
		GetWindowRect(_nwnd, &rect);
		if (!_lessed) {
			sz.width += _nc_width;
			sz.height += _nc_height;
		}

		MoveWindow(
			_nwnd, rect.left, rect.top,
			sz.width,
			sz.height,
			TRUE
		);
	}

	#define _STYLE_HAS(_style) (GetWindowLongW(_nwnd, GWL_STYLE) & _style) == _style

	void window::add_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				ShowWindow(_nwnd, SW_SHOW);
				break;
			case PWRE_STATE_MINIMIZE:
				ShowWindow(_nwnd, SW_MINIMIZE);
				break;
			case PWRE_STATE_MAXIMIZE:
				ShowWindow(_nwnd, SW_MAXIMIZE);
		}
	}

	void window::rm_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				ShowWindow(_nwnd, SW_HIDE);
				break;
			case PWRE_STATE_MINIMIZE:
				if (_STYLE_HAS(WS_MINIMIZE)) {
					ShowWindow(_nwnd, SW_RESTORE);
				}
				break;
			case PWRE_STATE_MAXIMIZE:
				ShowWindow(_nwnd, SW_SHOWNORMAL);
		}
	}

	bool window::has_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				return _STYLE_HAS(WS_VISIBLE);
			case PWRE_STATE_MINIMIZE:
				return _STYLE_HAS(WS_MINIMIZE);
			case PWRE_STATE_MAXIMIZE:
				return _STYLE_HAS(WS_MAXIMIZE);
		}
		return false;
	}

	#undef _STYLE_HAS

	void window::less(bool lessed) {
		RECT rect;
		GetClientRect(_nwnd, &rect);
		_lessed = lessed;
		resize({rect.right, rect.bottom});
	}
}

#endif // PWRE_PLAT_WIN32
