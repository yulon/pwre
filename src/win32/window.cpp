#include "../plat.h"

#ifdef PWRE_PLAT_WIN32

#include "window.hpp"
#include <dwmapi.h>

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

	LRESULT CALLBACK proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		auto wnd = (_window *)GetWindowLongPtrW(hWnd, PWRE_PLAT_WIN32_WNDEXTRA_I);
		if (wnd) {
			switch (uMsg) {
				case WM_NCCALCSIZE:
					if (wnd->lessed) {
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
					delete wnd;
					if (!--count) {
						life = false;
						return 0;
					}
			}
		}
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	void init() {
		host = GetModuleHandleW(NULL);

		cls.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		cls.hInstance = host;
		cls.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
		cls.hIconSm = LoadIconW(NULL, (LPCWSTR)IDI_WINLOGO);
		cls.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
		cls.lpszClassName = L"pwre::window";
		cls.cbWndExtra = sizeof(void *) * PWRE_PLAT_WIN32_WNDEXTRA;
		cls.lpfnWndProc = proc;
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

	_window::_window(uint64_t hints) {
		lessed = false;

		RECT rect {500, 500, 1000, 1000};
		BOOL ok = AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
		uassert(ok, "Pwre", "AdjustWindowRectEx");

		nc_width = (500 - rect.left) + (rect.right - 1000);
		nc_height = (500 - rect.top) + (rect.bottom - 1000);

		h = CreateWindowExW(
			0,
			cls.lpszClassName,
			NULL,
			WS_OVERLAPPEDWINDOW,
			0, 0, 150 + nc_width, 150 + nc_height, NULL, NULL,
			host,
			(LPVOID)this
		);
		uassert(h, "Pwre", "CreateWindowExW");

		count++;

		SetWindowLongPtrW(h, PWRE_PLAT_WIN32_WNDEXTRA_I, (LONG_PTR)(this));

		if (dwm) {
			if ((hints & PWRE_HINT_ALPHA) == PWRE_HINT_ALPHA && gdi) {
				DWM_BLURBEHIND bb;
				bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
				bb.hRgnBlur = CreateRectRgn_p(0, 0, -1, -1);
				bb.fEnable = TRUE;
				bb.fTransitionOnMaximized = 1;
				DwmEnableBlurBehindWindow_p(h, &bb);
			} else
			if (((hints & PWRE_HINT_WMBACKGROUND) == PWRE_HINT_WMBACKGROUND)) {
				MARGINS mar_inset;
				mar_inset.cxLeftWidth = -1;
				DwmExtendFrameIntoClientArea_p(h, &mar_inset);
			}
		}
	}

	window *create(uint64_t hints) {
		return static_cast<window *>(new _window(hints));
	}

	void _window::close() {
		CloseWindow(h);
	}

	void _window::destroy() {
		DestroyWindow(h);
	}

	uintptr_t _window::native_handle() {
		return (uintptr_t)h;
	}

	std::string _window::title() {
		std::string val;

		int wchars_len = GetWindowTextLengthW(h);
		if (wchars_len) {
			wchars_len++;
			WCHAR *wchars = new WCHAR[wchars_len];
			GetWindowTextW(h, wchars, wchars_len);

			int chars_len = WideCharToMultiByte(CP_UTF8, 0, wchars, -1, NULL, 0, NULL, NULL);
			if (chars_len) {
				char *chars = new char[chars_len + 1];
				chars[chars_len] = '\0';
				WideCharToMultiByte(CP_UTF8, 0, wchars, -1, chars, chars_len, NULL, NULL);
				val = chars;
				delete[] chars;
			}

			delete[] wchars;
		}

		return val;
	}

	void _window::retitle(const std::string &title) {
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

		SetWindowTextW(h, wchars);

		delete[] wchars;
	}

	point _window::pos() {
		RECT rect;
		GetWindowRect(h, &rect);
		return {rect.left, rect.top};
	}

	#define _SCREEN_W (GetSystemMetrics(SM_CXSCREEN))
	#define _SCREEN_H (GetSystemMetrics(SM_CYSCREEN))
	#include "../fix_pos.hpp"

	void _window::move(point pos) {
		RECT rect;
		GetWindowRect(h, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		fix_pos(pos.x, pos.y, width, height);
		MoveWindow(
			h, pos.x, pos.y,
			width,
			height,
			FALSE
		);
	}

	pwre::size _window::size() {
		RECT rect;
		if (lessed) {
			GetWindowRect(h, &rect);
			rect.right -= rect.left;
			rect.bottom -= rect.top;
		} else {
			GetClientRect(h, &rect);
		}
		return {rect.right, rect.bottom};
	}

	void _window::resize(pwre::size sz) {
		RECT rect;
		GetWindowRect(h, &rect);
		if (!lessed) {
			sz.width += nc_width;
			sz.height += nc_height;
		}
		MoveWindow(
			h, rect.left, rect.top,
			sz.width,
			sz.height,
			TRUE
		);
	}

	#define _STYLE_HAS(_style) (GetWindowLongW(h, GWL_STYLE) & _style) == _style

	void _window::add_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				ShowWindow(h, SW_SHOW);
				break;
			case PWRE_STATE_MINIMIZE:
				ShowWindow(h, SW_MINIMIZE);
				break;
			case PWRE_STATE_MAXIMIZE:
				ShowWindow(h, SW_MAXIMIZE);
		}
	}

	void _window::rm_states(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				ShowWindow(h, SW_HIDE);
				break;
			case PWRE_STATE_MINIMIZE:
				if (_STYLE_HAS(WS_MINIMIZE)) {
					ShowWindow(h, SW_RESTORE);
				}
				break;
			case PWRE_STATE_MAXIMIZE:
				ShowWindow(h, SW_SHOWNORMAL);
		}
	}

	bool _window::has_states(uint32_t type) {
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

	void _window::less(bool lessed) {
		RECT rect;
		GetClientRect(h, &rect);
		this->lessed = lessed;
		resize({rect.right, rect.bottom});
	}
}

#endif // PWRE_PLAT_WIN32
