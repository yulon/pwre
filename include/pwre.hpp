#ifndef _PWRE_H
#define _PWRE_H

#include <string>
#include <functional>

namespace Pwre {
	namespace System {
		bool Init();
		uintptr_t NativeObj();
		bool Step();
		void Run();
	} /* System */

	typedef struct {
		int X, Y;
	} Point;

	typedef struct {
		int Width, Height;
	} Size;

	typedef struct {
		int Left, Top, Right, Bottom;
	} Bounds;

	typedef struct {
		Bounds Outer;
		Bounds Border;
		Bounds Control;
	} ActionArea;

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

			const std::string &Title();
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

			virtual bool OnClose() { return true; }
			virtual void OnDestroy() {}
			virtual void OnSize(int width, int height) {}
			virtual void OnMove() {}
			virtual void OnPaint() {}
			virtual void OnMouseMove() {}
			virtual void OnMouseDown() {}
			virtual void OnMouseUp() {}
			virtual void OnKeyDown() {}
			virtual void OnKeyUp() {}

			///////////////////////////////////////////////

			struct _BlackBox;
			_BlackBox *_M;
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
			_BlackBox *_GLM;
	};
} /* Pwre */

#endif // !_PWRE_H
