#ifndef _PWRE_HPP
#define _PWRE_HPP

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <chrono>

#if (defined(_WIN32) && defined(_MSC_VER) && !defined(_USING_V110_SDK71)) || __cplusplus > 201402L
	#include <shared_mutex>
#else
	#include <mutex>
	#include <atomic>
	#include <condition_variable>
#endif

namespace Pwre {
	// Blocking current GUI work, wait GUI environment quit, but doesn't stop other GUI works.
	// GUI thread use only.
	void WaitQuit();

	// Blocking current GUI work, wait flag is true, but doesn't stop other GUI works.
	// GUI thread use only.
	void Wait(const std::atomic<bool> &flag);

	// Wake up sleeping GUI thread (if is).
	// All threads can use, but GUI thread can't wake up self.
	void WakeUp();

	// Add a task to GUI thread, and return a function for delete this task.
	// Argument "count" is 0 for unlimited, "interval" in milliseconds.
	// All threads can use.
	std::function<void()> AddTask(
		const std::function<void()> &,
		size_t count = 0,
		size_t interval = 0
	);

	struct Point {
		int x, y;
	};

	struct Size {
		int width, height;
	};

	struct Bounds {
		int left, top, right, bottom;
	};

	struct ActionArea {
		Bounds outer;
		Bounds border;
		Bounds control;
	};

	#if (defined(_WIN32) && defined(_MSC_VER) && !defined(_USING_V110_SDK71)) || __cplusplus > 201402L
		typedef std::shared_mutex _shared_mutex;
	#else
		class _shared_mutex {
			public:
				void lock() {
					std::unique_lock<std::mutex> ul(_atomMux);

					if (_exclusive || _shared) {
						_cond.wait(ul);
					}
					_exclusive = true;
				}

				void unlock() {
					std::unique_lock<std::mutex> ul(_atomMux);

					_exclusive = false;
					_cond.notify_all();
				}

				void lock_shared() {
					std::unique_lock<std::mutex> ul(_atomMux);

					if (_exclusive) {
						_cond.wait(ul);
					}
					_shared++;
				}

				void unlock_shared() {
					std::unique_lock<std::mutex> ul(_atomMux);

					if (--_shared == 0) {
						_cond.notify_all();
					}
				}

			private:
				std::mutex _atomMux;
				bool _exclusive = false;
				size_t _shared = 0;
				std::condition_variable _cond;
		};
	#endif

	template <typename Ret, typename... Args>
	class EventHandler {
		public:
			std::function<void()> Add(const std::function<Ret(Args...)> &handler) {
				_sm.lock();
				_funcs.push_back(handler);
				auto it = --_funcs.end();
				_sm.unlock();
				return [this, it](){
					_sm.lock();
					this->_funcs.erase(it);
					_sm.unlock();
				};
			}
		protected:
			std::vector<std::function<Ret(Args...)>> _funcs;
			_shared_mutex _sm;
	};

	template <typename... Args>
	class EventAcceptor : public EventHandler<bool, Args...> {
		public:
			bool Accept(Args... a) {
				this->_sm.lock_shared();
				for (auto rit = this->_funcs.rbegin(); rit != this->_funcs.rend(); rit++) {
					if (!(*rit)(a...)) {
						this->_sm.unlock_shared();
						return false;
					}
				}
				this->_sm.unlock_shared();
				return true;
			}
	};

	template <typename... Args>
	class EventReceiver : public EventHandler<void, Args...> {
		public:
			void Receive(Args... a) {
				this->_sm.lock_shared();
				for (auto rit = this->_funcs.rbegin(); rit != this->_funcs.rend(); rit++) {
					(*rit)(a...);
				}
				this->_sm.unlock_shared();
			}
	};

	class Window {
		public:
			#define PWRE_HINT_ALPHA 0x00000001 // Support for Windows Vista+ (suggest less), X11 (only GL), macOS.
			#define PWRE_HINT_BLUR 0x00000010 // Support for Windows Vista/7 (only Aero Glass), Windows 10 (unpublished API, not perfect, suggest less), macOS.
			#define PWRE_HINT_WMBACKGROUND 0x00000002 // Support for Windows Vista+ (Aero Glass will automatically blur, but when less only shadow), macOS.

			Window(uint64_t hints = 0);
			~Window();

			void Close();
			void Destroy();

			uintptr_t NativeObj();

			std::string Title();
			void Retitle(const std::string &);

			#define PWRE_MOVE_AUTO -10101

			void Pos(int &x, int &y);
			void Move(int x = PWRE_MOVE_AUTO, int y = PWRE_MOVE_AUTO);

			void Size(int &width, int &height);
			void Resize(int width, int height);

			#define PWRE_STATE_VISIBLE 0x00000001
			#define PWRE_STATE_MINIMIZE 0x00000010
			#define PWRE_STATE_MAXIMIZE 0x00000100
			#define PWRE_STATE_FULLSCREEN 0x00001000

			void AddStates(uint32_t);
			void RmStates(uint32_t);
			bool HasStates(uint32_t);

			void Less(bool);
			bool CustomActionArea(const ActionArea &);

			EventAcceptor<> OnClose;
			EventReceiver<> OnDestroy;
			EventReceiver<int/*width*/, int/*height*/> OnSize;
			EventReceiver<int/*x*/, int/*y*/> OnMove;
			EventReceiver<> OnPaint;
			EventReceiver<int/*x*/, int/*y*/> OnMouseMove;
			EventReceiver<int/*x*/, int/*y*/> OnMouseDown;
			EventReceiver<int/*x*/, int/*y*/> OnMouseUp;
			EventReceiver<int/*keyCode*/> OnKeyDown;
			EventReceiver<int/*keyCode*/> OnKeyUp;

			///////////////////////////////////////////////

			struct _BlackBox;
			_BlackBox *_m;
	};

	namespace GL {
		class Window : public Pwre::Window {
			public:
				#define PWRE_HINT_GL_V3 0x3000000000

				Window(uint64_t hints = 0);
				~Window();

				uintptr_t NativeGLCtx();
				void MakeCurrent();
				void SwapBuffers();

				///////////////////////////////////////////////

				struct _BlackBox;
				_BlackBox *_glm;
		};

		inline uintptr_t NativeGLCtx(Pwre::Window *wnd) {
			return static_cast<Pwre::GL::Window *>(wnd)->NativeGLCtx();
		}

		inline void MakeCurrent(Pwre::Window *wnd) {
			static_cast<Pwre::GL::Window *>(wnd)->MakeCurrent();
		}

		inline void SwapBuffers(Pwre::Window *wnd) {
			static_cast<Pwre::GL::Window *>(wnd)->SwapBuffers();
		}
	} /* GL */
} /* Pwre */

#endif // !_PWRE_HPP
