#ifndef _PWRE_HPP
#define _PWRE_HPP

#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <tuple>

namespace pwre {
	struct point {
		int x, y;
	};

	struct size {
		int width, height;
	};

	struct bounds {
		int left, top, right, bottom;
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

	class gl_context {
		public:
			virtual uintptr_t native_handle() = 0;
			virtual void make_current() = 0;
			virtual void swap_buffers() = 0;

		protected:
			virtual ~gl_context() {}
	};

	class window {
		public:
			virtual void close() = 0;
			virtual void destroy() = 0;

			virtual uintptr_t native_handle() = 0;

			#define _PWRE_NOT_IMPL(_mn, _a) std::cout << "pwre::window::"#_mn"(" << _a << "): Not implemented!" << std::endl

			virtual std::string title()                                         { _PWRE_NOT_IMPL(title, ""); return NULL; }
			virtual void retitle(const std::string &title)                      { _PWRE_NOT_IMPL(retitle, "\"" << title << "\""); }

			virtual point pos()                                                 { _PWRE_NOT_IMPL(pos, ""); return {}; }

			#define PWRE_MOVE_AUTO -10101

			virtual void move(point pos = {PWRE_MOVE_AUTO, PWRE_MOVE_AUTO})     { _PWRE_NOT_IMPL(move, pos.x << ", " << pos.y); }

			virtual pwre::size size() = 0;
			virtual void resize(pwre::size sz)                                  { _PWRE_NOT_IMPL(resize, sz.width << ", " << sz.height); }

			#define PWRE_STATE_VISIBLE 0x00000001
			#define PWRE_STATE_MINIMIZE 0x00000010
			#define PWRE_STATE_MAXIMIZE 0x00000100
			#define PWRE_STATE_FULLSCREEN 0x00001000

			virtual void add_states(uint32_t states) = 0;
			virtual void rm_states(uint32_t states) = 0;
			virtual bool has_states(uint32_t states) = 0;

			virtual void less(bool lessed)                                      { _PWRE_NOT_IMPL(less, lessed); }
			virtual bool rearea(const action_area &)                            { _PWRE_NOT_IMPL(rearea, "action_area"); return false; }

			#undef _PWRE_NOT_IMPL

			////////////////////////////////////////////////////////////////////

			interceptor<> on_close;
			listener<> on_destroy;
			listener<> on_size;
			listener<point> on_move;
			listener<> on_paint;
			listener<int, point> on_mouse_move;
			listener<int, point> on_mouse_down;
			listener<int, point> on_mouse_up;
			listener<int> on_key_down;
			listener<int> on_key_up;

		protected:
			virtual ~window() {}
	};

	void init();
	bool recv_event();
	bool recv_event(window *);
	bool checkout_events();
	bool checkout_events(window *);

	#define PWRE_HINT_ALPHA 0x00000001 // Support for Windows Vista+ (suggest less), X11 (only GL), macOS.
	#define PWRE_HINT_BLUR 0x00000010 // Support for Windows Vista/7 (only Aero Glass), Windows 10 (unpublished API, not perfect, suggest less), macOS.
	#define PWRE_HINT_WMBACKGROUND 0x00000002 // Support for Windows Vista+ (Aero Glass will automatically blur, but when less only shadow), macOS.

	window *create(uint64_t hints = 0);
	window *create(gl_context *&, uint64_t hints = 0);
} /* pwre */

#endif // !_PWRE_HPP
