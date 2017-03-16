#ifndef _PWRE_H
#define _PWRE_H

#include <string>
#include <vector>
#include <functional>

namespace Pwre {
	namespace System {
		bool Init();
		uintptr_t NativeObj();
		bool Step();
		void Run();
	} /* System */

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

	template <typename... Args>
	class EventAcceptor {
		public:
			int Add(const std::function<bool(Args...)> &handler) {
				_cbs.push_back(handler);
				return _cbs.size() - 1;
			}
			bool Accept(Args... a) {
				for (int i = _cbs.size() - 1; i >= 0; i--) {
					if (!_cbs[i](a...)) {
						return false;
					}
				}
				return true;
			}
		private:
			std::vector<std::function<bool(Args...)>> _cbs;
	};

	template <typename... Args>
	class EventReceiver {
		public:
			int Add(const std::function<void(Args...)> &handler) {
				_cbs.push_back(handler);
				return _cbs.size() - 1;
			}
			void Receive(Args... a) {
				for (int i = _cbs.size() - 1; i >= 0; i--) {
					_cbs[i](a...);
				}
			}
		private:
			std::vector<std::function<void(Args...)>> _cbs;
	};

	class Window {
		public:
			#define PWRE_HINT_ALPHA 0x00000001 // support for Windows Vista+ (suggest less), X11 (only GL), macOS.
			#define PWRE_HINT_BLUR 0x00000010 // support for Windows Vista/7 (only Aero Glass), Windows 10 (unpublished API, not perfect, suggest less), macOS.
			#define PWRE_HINT_WMBACKGROUND 0x00000002 // support for Windows Vista+ (Aero Glass will automatically blur, but when less only shadow), macOS.

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

	class GLWindow : public Window {
		public:
			#define PWRE_HINT_GL_V3 0x3000000000

			GLWindow(uint64_t hints = 0);
			~GLWindow();

			uintptr_t NativeGLCtx();
			void MakeCurrent();
			void SwapBuffers();

			///////////////////////////////////////////////

			struct _BlackBox;
			_BlackBox *_glm;
	};
} /* Pwre */

#endif // !_PWRE_H
