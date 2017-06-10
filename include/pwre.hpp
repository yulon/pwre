#ifndef _PWRE_HPP
#define _PWRE_HPP

#if !defined(PWRE_PLAT_WIN32) && !defined(PWRE_PLAT_X11) && !defined(PWRE_PLAT_COCOA)
	#if defined(_WIN32)
		#define PWRE_PLAT_WIN32
	#elif defined(__APPLE__) && defined(__MACH__)
		#define PWRE_PLAT_COCOA
	#elif defined(__unix__)
		#define PWRE_PLAT_X11
	#endif
#endif

#ifdef PWRE_PLAT_WIN32
	#include <windows.h>
#elif defined(PWRE_PLAT_COCOA) && defined(__OBJC__)
	#import <Cocoa/Cocoa.h>
#elif defined(PWRE_PLAT_X11)
	#include <X11/Xlib.h>
	#include <GL/glx.h>
#endif

#include <string>
#include <vector>
#include <functional>
#include <iostream>

namespace pwre {
	class window {
		public:
			typedef
				#ifdef PWRE_PLAT_WIN32
					HWND
				#elif defined(PWRE_PLAT_COCOA)
					#ifdef __OBJC__
						NSWindow *
					#else
						uintptr_t
					#endif
				#elif defined(PWRE_PLAT_X11)
					Window
				#endif
				native_handle_type;

			struct pos_type {
				int x, y;
			};

			struct size_type {
				int width, height;
			};

			struct bounds {
				int left, top, right, bottom;
			};

			struct view_area {
				bounds chrome;
			};

			struct action_area {
				bounds outer;
				bounds border;
				bounds control;
			};

			template <typename Res, typename... Args>
			class funcontainer {
				public:
					std::function<void()> add(const std::function<Res(Args...)> &func) {
						_funcs.push_back(func);
						auto it = --_funcs.end();
						return [this, it](){
							this->_funcs.erase(it);
						};
					}
				protected:
					std::vector<std::function<Res(Args...)>> _funcs;
			};

			template <typename... Args>
			class interceptor : public funcontainer<bool, Args...> {
				public:
					bool calls(Args... a) {
						for (auto rit = this->_funcs.rbegin(); rit != this->_funcs.rend(); rit++) {
							if (!(*rit)(a...)) {
								return false;
							}
						}
						return true;
					}
			};

			template <typename... Args>
			class listener : public funcontainer<void, Args...> {
				public:
					void calls(Args... a) {
						for (auto rit = this->_funcs.rbegin(); rit != this->_funcs.rend(); rit++) {
							(*rit)(a...);
						}
					}
			};

			////////////////////////////////////////////////////////////////////

			#define PWRE_HINT_ALPHA 0x00000001 // Support for Windows Vista+ (suggest less), X11 (only GL), macOS.
			#define PWRE_HINT_BLUR 0x00000010 // Support for Windows Vista/7 (only Aero Glass), Windows 10 (unpublished API, not perfect, suggest less), macOS.
			#define PWRE_HINT_WMBACKGROUND 0x00000002 // Support for Windows Vista+ (Aero Glass will automatically blur, but when less only shadow), macOS.

			window(uint64_t hints = 0);

			window(const window &) = delete;
			window(window &&);

			virtual ~window() { if (available()) destroy(); }

			////////////////////////////////////////////////////////////////////

			bool available() { return _nwnd; }
			native_handle_type native_handle() { return _nwnd; }

			void close();
			void destroy();

			std::string title();
			void retitle(const std::string &);

			pos_type pos();

			#define PWRE_MOVE_AUTO -10101

			void move(pos_type pos = {PWRE_MOVE_AUTO, PWRE_MOVE_AUTO});

			size_type size();
			void resize(size_type);

			#define PWRE_STATE_VISIBLE 0x00000001
			#define PWRE_STATE_MINIMIZE 0x00000010
			#define PWRE_STATE_MAXIMIZE 0x00000100
			#define PWRE_STATE_FULLSCREEN 0x00001000

			void add_states(uint32_t);
			void rm_states(uint32_t);
			bool has_states(uint32_t);

			void less(bool);
			bool rearea(const action_area &);

			////////////////////////////////////////////////////////////////////

			interceptor<> on_close;
			listener<> on_destroy;
			listener<> on_size;
			listener<pos_type> on_move;
			listener<> on_paint;
			listener<int, pos_type> on_mouse_move;
			listener<int, pos_type> on_mouse_down;
			listener<int, pos_type> on_mouse_up;
			listener<int> on_key_down;
			listener<int> on_key_up;

		private:
			native_handle_type _nwnd;

			#ifdef PWRE_PLAT_WIN32
				int _nc_width, _nc_height;
				bool _lessed;
				friend LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			#elif defined(PWRE_PLAT_X11)
				friend void ctor(
					window &wnd,
					uint64_t hints,
					int depth,
					Visual *visual,
					unsigned long valuemask,
					XSetWindowAttributes *swa
				);
			#endif
	};

	class gl_window : public window {
		public:
			class render_context_type {
				public:
					typedef
						#ifdef PWRE_PLAT_WIN32
							HGLRC
						#elif defined(PWRE_PLAT_COCOA)
							#ifdef __OBJC__
								NSOpenGLContext *
							#else
								uintptr_t
							#endif
						#elif defined(PWRE_PLAT_X11)
							GLXContext
						#endif
						native_handle_type;

					////////////////////////////////////////////////////////////

					native_handle_type native_handle() { return _nrc; }

					void make_current();
					void swap_buffers();

					////////////////////////////////////////////////////////////

					render_context_type &operator=(const render_context_type &) = delete;
					render_context_type &operator=(render_context_type &&) = delete;

				private:
					render_context_type() {}

					native_handle_type _nrc;

					#ifdef PWRE_PLAT_WIN32
						HDC _dc;
					#elif defined(PWRE_PLAT_X11)
						window::native_handle_type _nwnd;
					#endif

					friend gl_window;
			};

			////////////////////////////////////////////////////////////////////

			render_context_type render_context;

			////////////////////////////////////////////////////////////////////

			gl_window(uint64_t hints = 0);
	};

	inline void init() {
		#ifdef PWRE_PLAT_WIN32
			#define _PWRE_INIT init_win32
		#elif defined(PWRE_PLAT_COCOA)
			#define _PWRE_INIT init_cocoa
		#elif defined(PWRE_PLAT_X11)
			#define _PWRE_INIT init_x11
		#endif
		void _PWRE_INIT();
		_PWRE_INIT();
		#undef _PWRE_INIT
	}

	bool recv_event();
	bool checkout_events();
} /* pwre */

#endif // !_PWRE_HPP
