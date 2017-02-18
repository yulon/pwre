#include <pwre.hpp>
#include <iostream>

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

class MyGLWindow : public Pwre::GLWindow {
	public:
		MyGLWindow():GLWindow(PWRE_HINT_ALPHA) {}

		virtual void OnPaint() {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBegin(GL_TRIANGLES);

			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(0.0, 1.0, 0.0);

			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(-1.0, -1.0, 0.0);

			glColor3f(0.0, 0.0, 1.0);
			glVertex3f(1.0, -1.0, 0.0);

			glEnd();
			SwapBuffers();
		}
};

int main() {
	Pwre::System::Init();

	MyGLWindow wnd;
	wnd.Less(true);
	wnd.Resize(600, 500);
	wnd.MakeCurrent();
	wnd.Retitle((const char *)glGetString(GL_VERSION));
	glClearColor(0, 0, 0, 0.4);
	wnd.AddStates(PWRE_STATE_VISIBLE);
	wnd.Move();

	Pwre::System::Run();
	return 0;
}
