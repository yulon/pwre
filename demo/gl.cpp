#include <pwre.hpp>

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>
#else
	#if defined(_WIN32) && defined(_MSC_VER)
		#include <windows.h>
	#endif

	#include <GL/gl.h>
#endif

int main() {
	Pwre::System::Init();

	Pwre::GLWindow wnd;
	wnd.Resize(600, 500);
	wnd.MakeCurrent();
	wnd.Retitle((const char *)glGetString(GL_VERSION));
	glClearColor(1, 1, 1, 1);
	wnd.AddStates(PWRE_STATE_VISIBLE);
	wnd.Move();

	while (Pwre::System::Step()) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBegin(GL_TRIANGLES);

		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 1.0, 0.0);

		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(-1.0, -1.0, 0.0);

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(1.0, -1.0, 0.0);

		glEnd();
		wnd.SwapBuffers();
	}
	return 0;
}
