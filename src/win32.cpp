#include "plat.h"

#ifdef PWRE_PLAT_WIN32

#include "win32.hpp"
#include <dwmapi.h>

namespace Pwre {
	namespace DLL {
		#define _LOAD_DLL(m, code) DLL::m::h = LoadLibraryW(L###m); if (DLL::m::h) { code }
		#define _LOAD_DLL_FUNC(m, f) DLL::m::f = (DLL::m::f##_t)GetProcAddress(DLL::m::h, #f)

		namespace dwmapi {
			HMODULE h;
			typedef HRESULT (WINAPI *DwmEnableBlurBehindWindow_t)(HWND hWnd, const DWM_BLURBEHIND *pBlurBehind);
			DwmEnableBlurBehindWindow_t DwmEnableBlurBehindWindow;
			typedef HRESULT (WINAPI *DwmExtendFrameIntoClientArea_t)(HWND hWnd, const MARGINS *pMarInset);
			DwmExtendFrameIntoClientArea_t DwmExtendFrameIntoClientArea;
		} /* dwmapi */

		namespace gdi32 {
			HMODULE h;
			typedef HRGN (WINAPI *CreateRectRgn_t)(int x1, int y1, int x2, int y2);
			CreateRectRgn_t CreateRectRgn;
		} /* gdi32 */
	} /* DLL */

	namespace System {
		#ifndef PWRE_PLAT_WIN32_WNDEXTRA
		#define PWRE_PLAT_WIN32_WNDEXTRA 10
		#endif

		#ifndef PWRE_PLAT_WIN32_WNDEXTRA_I
		#define PWRE_PLAT_WIN32_WNDEXTRA_I 6
		#endif

		std::atomic<int> wndCount;

		LRESULT CALLBACK WndMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
			auto wnd = (Window *)GetWindowLongPtrW(hWnd, PWRE_PLAT_WIN32_WNDEXTRA_I);
			if (wnd) {
				switch (uMsg) {
					case WM_NCCALCSIZE:
						if (wnd->_m->less) {
							if (wParam) {
								((LPNCCALCSIZE_PARAMS)lParam)->rgrc[2] = ((LPNCCALCSIZE_PARAMS)lParam)->rgrc[1];
								((LPNCCALCSIZE_PARAMS)lParam)->rgrc[1] = ((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0];
							}
							return 0;
						}
						break;
					case WM_PAINT:
						wnd->OnPaint.Receive();
						ValidateRect(hWnd, NULL);
						return 0;
					case WM_ERASEBKGND:
						return 0;
					case WM_CLOSE:
						if (!wnd->OnClose.Accept()) {
							return 0;
						}
						break;
					case WM_DESTROY:
						SetWindowLongPtrW(hWnd, PWRE_PLAT_WIN32_WNDEXTRA_I, 0);
						wnd->OnDestroy.Receive();
						if (!--wndCount) {
							PostQuitMessage(0);
							return 0;
						}
				}
			}
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}

		static HMODULE mainModule;
		static WNDCLASSEXW wndClass;

		bool Init() {
			_LOAD_DLL(dwmapi,
				_LOAD_DLL_FUNC(dwmapi, DwmEnableBlurBehindWindow);
				_LOAD_DLL_FUNC(dwmapi, DwmExtendFrameIntoClientArea);

				_LOAD_DLL(gdi32,
					_LOAD_DLL_FUNC(gdi32, CreateRectRgn);
				)
			)

			mainModule = GetModuleHandleW(NULL);

			wndClass.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
			wndClass.hInstance = mainModule;
			wndClass.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
			wndClass.hIconSm = LoadIconW(NULL, (LPCWSTR)IDI_WINLOGO);
			wndClass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
			wndClass.lpszClassName = L"Pwre::Window";
			wndClass.cbWndExtra = sizeof(void *) * PWRE_PLAT_WIN32_WNDEXTRA;
			wndClass.lpfnWndProc = WndMsgHandler;
			wndClass.cbSize = sizeof(wndClass);

			ATOM ok = RegisterClassExW(&wndClass);
			if (!ok) {
				std::cout << "Pwre: Win32.RegisterClassExW error!" << std::endl;
				return false;
			}

			wndCount = 0;
			return true;
		}

		uintptr_t NativeObj() {
			return (uintptr_t)mainModule;
		}

		bool Step() {
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

		void Run() {
			MSG msg;
			msg.message = 0;
			while (msg.message != WM_QUIT && (GetMessageW(&msg, NULL, 0, 0) > 0)) {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
	} /* System */

	Window::Window(uint64_t hints) {
		_m = new _BlackBox;
		_m->less = false;

		RECT rect{500, 500, 1000, 1000};
		BOOL ok = AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
		if (!ok) {
			std::cout << "Pwre: Win32.AdjustWindowRectEx error!" << std::endl;
			return;
		}

		_m->ncWidth = (500 - rect.left) + (rect.right - 1000);
		_m->ncHeight = (500 - rect.top) + (rect.bottom - 1000);

		_m->hWnd = CreateWindowExW(
			0,
			System::wndClass.lpszClassName,
			NULL,
			WS_OVERLAPPEDWINDOW,
			0, 0, 150 + _m->ncWidth, 150 + _m->ncHeight, NULL, NULL,
			System::mainModule,
			(LPVOID)this
		);
		if (!_m->hWnd) {
			std::cout << "Pwre: Win32.CreateWindowExW error!" << std::endl;
			return;
		}

		System::wndCount++;

		SetWindowLongPtrW(_m->hWnd, PWRE_PLAT_WIN32_WNDEXTRA_I, (LONG_PTR)(this));

		if (((hints & PWRE_HINT_ALPHA) == PWRE_HINT_ALPHA) && DLL::dwmapi::DwmEnableBlurBehindWindow && DLL::gdi32::CreateRectRgn) {
			DWM_BLURBEHIND bb;
			bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
			bb.hRgnBlur = DLL::gdi32::CreateRectRgn(0, 0, -1, -1);
			bb.fEnable = TRUE;
			bb.fTransitionOnMaximized = 1;
			DLL::dwmapi::DwmEnableBlurBehindWindow(_m->hWnd, &bb);
		} else
		if (((hints & PWRE_HINT_WMBACKGROUND) == PWRE_HINT_WMBACKGROUND) && DLL::dwmapi::DwmExtendFrameIntoClientArea) {
			MARGINS marInset;
			marInset.cxLeftWidth = -1;
			DLL::dwmapi::DwmExtendFrameIntoClientArea(_m->hWnd, &marInset);
		}
	}

	Window::~Window() {
		delete _m;
	}

	uintptr_t Window::NativeObj() {
		return (uintptr_t)_m->hWnd;
	}

	void Window::Close() {
		CloseWindow(_m->hWnd);
	}

	void Window::Destroy() {
		DestroyWindow(_m->hWnd);
	}

	std::string Window::Title() {
		std::string title;

		int wcharsLen = GetWindowTextLengthW(_m->hWnd);
		if (wcharsLen) {
			wcharsLen++;
			WCHAR *wchars = new WCHAR[wcharsLen];
			GetWindowTextW(_m->hWnd, wchars, wcharsLen);

			int charsLen = WideCharToMultiByte(CP_UTF8, 0, wchars, -1, NULL, 0, NULL, NULL);
			if (charsLen) {
				char *chars = new char[charsLen + 1];
				chars[charsLen] = '\0';
				WideCharToMultiByte(CP_UTF8, 0, wchars, -1, chars, charsLen, NULL, NULL);
				title = chars;
				delete[] chars;
			}

			delete[] wchars;
		}

		return title;
	}

	void Window::Retitle(const std::string &title) {
		if (title.size() == 0) {
			return;
		}
		const char *chars = title.c_str();

		int wcharsLen = MultiByteToWideChar(CP_UTF8, 0, chars, -1, NULL, 0);
		if (!wcharsLen) {
			return;
		}
		WCHAR *wchars = new WCHAR[wcharsLen + 1];
		MultiByteToWideChar(CP_UTF8, 0, chars, -1, wchars, wcharsLen);

		SetWindowTextW(_m->hWnd, wchars);

		delete[] wchars;
	}

	void Window::Pos(int &x, int &y) {
		RECT rect;
		GetWindowRect(_m->hWnd, &rect);
		x = rect.left;
		y = rect.top;
	}

	#include "fixpos.hpp"

	void Window::Move(int x, int y) {
		RECT rect;
		GetWindowRect(_m->hWnd, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		FixPos(x, y, width, height);
		MoveWindow(
			_m->hWnd, x, y,
			width,
			height,
			FALSE
		);
	}

	void Window::Size(int &width, int &height) {
		RECT rect;
		if (_m->less) {
			GetWindowRect(_m->hWnd, &rect);
			rect.right -= rect.left;
			rect.bottom -= rect.top;
		} else {
			GetClientRect(_m->hWnd, &rect);
		}
		if (width) {
			width = rect.right;
		}
		if (height) {
			height = rect.bottom;
		}
	}

	void Window::Resize(int width, int height) {
		RECT rect;
		GetWindowRect(_m->hWnd, &rect);
		if (!_m->less) {
			width += _m->ncWidth;
			height += _m->ncHeight;
		}
		MoveWindow(
			_m->hWnd, rect.left, rect.top,
			width,
			height,
			TRUE
		);
	}

	#define _STYLE_HAS(_style) (GetWindowLongW(_m->hWnd, GWL_STYLE) & _style) == _style

	void Window::AddStates(uint32_t type) {
		ShowWindow(_m->hWnd, SW_SHOW);
		return;
		switch (type) {
			case PWRE_STATE_VISIBLE:
				ShowWindow(_m->hWnd, SW_SHOW);
				break;
			case PWRE_STATE_MINIMIZE:
				ShowWindow(_m->hWnd, SW_MINIMIZE);
				break;
			case PWRE_STATE_MAXIMIZE:
				ShowWindow(_m->hWnd, SW_MAXIMIZE);
		}
	}

	void Window::RmStates(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				ShowWindow(_m->hWnd, SW_HIDE);
				break;
			case PWRE_STATE_MINIMIZE:
				if (_STYLE_HAS(WS_MINIMIZE)) {
					ShowWindow(_m->hWnd, SW_RESTORE);
				}
				break;
			case PWRE_STATE_MAXIMIZE:
				ShowWindow(_m->hWnd, SW_SHOWNORMAL);
		}
	}

	bool Window::HasStates(uint32_t type) {
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

	void Window::Less(bool less) {
		RECT rect;
		GetClientRect(_m->hWnd, &rect);
		_m->less = less;
		Window::Resize(rect.right, rect.bottom);
	}
} /* Pwre */

#endif // PWRE_PLAT_WIN32
