#include <pwre.hpp>
#include <iostream>

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>
#else
	#if defined(_WIN32) && defined(_MSC_VER)
		#include <windows.h>
	#endif

	#include <GL/gl.h>
#endif

int main() {
	Pwre::GL::Window wnd(PWRE_HINT_ALPHA);
	wnd.Less(true);
	wnd.Resize(600, 500);
	wnd.MakeCurrent();
	wnd.Retitle((const char *)glGetString(GL_VERSION));

	glClearColor(0, 0, 0, 0.4);

	wnd.OnPaint.Add([&wnd]() {
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
	});

	wnd.AddStates(PWRE_STATE_VISIBLE);
	wnd.Move();

	Pwre::WaitQuit();
	return 0;
}
