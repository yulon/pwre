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
						if (wnd->_M->Lessed) {
							if (wParam) {
								((LPNCCALCSIZE_PARAMS)lParam)->rgrc[2] = ((LPNCCALCSIZE_PARAMS)lParam)->rgrc[1];
								((LPNCCALCSIZE_PARAMS)lParam)->rgrc[1] = ((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0];
							}
							return 0;
						}
						break;
					case WM_PAINT:
						wnd->OnPaint();
						ValidateRect(hWnd, NULL);
						return 0;
					case WM_CLOSE:
						if (!wnd->OnClose()) {
							return 0;
						}
						break;
					case WM_DESTROY:
						SetWindowLongPtrW(hWnd, PWRE_PLAT_WIN32_WNDEXTRA_I, 0);
						wnd->OnDestroy();
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
		_M = new _BlackBox;
		_M->Lessed = false;

		RECT rect = { 500, 500, 1000, 1000 };
		BOOL ok = AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
		if (!ok) {
			std::cout << "Pwre: Win32.AdjustWindowRectEx error!" << std::endl;
			return;
		}

		_M->NCWidth = (500 - rect.left) + (rect.right - 1000);
		_M->NCHeight = (500 - rect.top) + (rect.bottom - 1000);

		_M->HWnd = CreateWindowExW(
			0,
			System::wndClass.lpszClassName,
			NULL,
			WS_OVERLAPPEDWINDOW,
			0, 0, 150 + _M->NCWidth, 150 + _M->NCHeight, NULL, NULL,
			System::mainModule,
			(LPVOID)this
		);
		if (!_M->HWnd) {
			std::cout << "Pwre: Win32.CreateWindowExW error!" << std::endl;
			return;
		}

		System::wndCount++;

		SetWindowLongPtrW(_M->HWnd, PWRE_PLAT_WIN32_WNDEXTRA_I, (LONG_PTR)(this));

		if (((hints & PWRE_HINT_ALPHA) == PWRE_HINT_ALPHA) && DLL::dwmapi::DwmEnableBlurBehindWindow && DLL::gdi32::CreateRectRgn) {
			DWM_BLURBEHIND bb;
			bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
			bb.hRgnBlur = DLL::gdi32::CreateRectRgn(0, 0, -1, -1);
			bb.fEnable = TRUE;
			bb.fTransitionOnMaximized = 1;
			DLL::dwmapi::DwmEnableBlurBehindWindow(_M->HWnd, &bb);
		} else
		if (((hints & PWRE_HINT_WMBACKGROUND) == PWRE_HINT_WMBACKGROUND) && DLL::dwmapi::DwmExtendFrameIntoClientArea) {
			MARGINS marInset;
			marInset.cxLeftWidth = -1;
			DLL::dwmapi::DwmExtendFrameIntoClientArea(_M->HWnd, &marInset);
		}
	}

	Window::~Window() {
		delete _M;
	}

	uintptr_t Window::NativeObj() {
		return (uintptr_t)_M->HWnd;
	}

	void Window::Close() {
		CloseWindow(_M->HWnd);
	}

	void Window::Destroy() {
		DestroyWindow(_M->HWnd);
	}

	const std::string &Window::Title() {
		_M->Mux.lock();
		int str16_len = GetWindowTextLengthW(_M->HWnd);
		if (str16_len) {
			str16_len++;
			WCHAR *str16 = new WCHAR[str16_len];
			GetWindowTextW(_M->HWnd, str16, str16_len);

			int str8_len = WideCharToMultiByte(CP_UTF8, 0, str16, -1, NULL, 0, NULL, NULL);
			if (str8_len) {
				char *str8 = new char[str8_len + 1];
				str8[str8_len] = '\0';
				WideCharToMultiByte(CP_UTF8, 0, str16, -1, str8, str8_len, NULL, NULL);
				_M->TitleBuf = str8;
				delete[] str8;
			}

			delete[] str16;
		} else {
			_M->TitleBuf.resize(0);
		}
		_M->Mux.unlock();
		return _M->TitleBuf;
	}

	void Window::Retitle(const std::string &s) {
		if (s.size() == 0) {
			return;
		}
		const char *str8 = s.c_str();

		int str16_len = MultiByteToWideChar(CP_UTF8, 0, str8, -1, NULL, 0);
		if (!str16_len) {
			return;
		}
		WCHAR *str16 = new WCHAR[str16_len + 1];
		MultiByteToWideChar(CP_UTF8, 0, str8, -1, str16, str16_len);

		SetWindowTextW(_M->HWnd, str16);

		delete[] str16;
	}

	void Window::Pos(int &x, int &y) {
		RECT rect;
		GetWindowRect(_M->HWnd, &rect);
		x = rect.left;
		y = rect.top;
	}

	#include "fixpos.hpp"

	void Window::Move(int x, int y) {
		RECT rect;
		GetWindowRect(_M->HWnd, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		FixPos(x, y, width, height);
		MoveWindow(
			_M->HWnd, x, y,
			width,
			height,
			FALSE
		);
	}

	void Window::Size(int &width, int &height) {
		RECT rect;
		if (_M->Lessed) {
			GetWindowRect(_M->HWnd, &rect);
			rect.right -= rect.left;
			rect.bottom -= rect.top;
		} else {
			GetClientRect(_M->HWnd, &rect);
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
		GetWindowRect(_M->HWnd, &rect);
		if (!_M->Lessed) {
			width += _M->NCWidth;
			height += _M->NCHeight;
		}
		MoveWindow(
			_M->HWnd, rect.left, rect.top,
			width,
			height,
			TRUE
		);
	}

	#define _STYLE_HAS(_style) (GetWindowLongW(_M->HWnd, GWL_STYLE) & _style) == _style

	void Window::AddStates(uint32_t type) {
		ShowWindow(_M->HWnd, SW_SHOW);
		return;
		switch (type) {
			case PWRE_STATE_VISIBLE:
				ShowWindow(_M->HWnd, SW_SHOW);
				break;
			case PWRE_STATE_MINIMIZE:
				ShowWindow(_M->HWnd, SW_MINIMIZE);
				break;
			case PWRE_STATE_MAXIMIZE:
				ShowWindow(_M->HWnd, SW_MAXIMIZE);
		}
	}

	void Window::RmStates(uint32_t type) {
		switch (type) {
			case PWRE_STATE_VISIBLE:
				ShowWindow(_M->HWnd, SW_HIDE);
				break;
			case PWRE_STATE_MINIMIZE:
				if (_STYLE_HAS(WS_MINIMIZE)) {
					ShowWindow(_M->HWnd, SW_RESTORE);
				}
				break;
			case PWRE_STATE_MAXIMIZE:
				ShowWindow(_M->HWnd, SW_SHOWNORMAL);
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
		GetClientRect(_M->HWnd, &rect);
		_M->Lessed = less;
		Window::Resize(rect.right, rect.bottom);
	}
} /* Pwre */

#endif // PWRE_PLAT_WIN32
