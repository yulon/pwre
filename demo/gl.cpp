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
	pwre::init();

	pwre::gl_context *glc;
	auto wnd = pwre::create(glc);

	glc->make_current();
	wnd->retitle((const char *)glGetString(GL_VERSION));
	glClearColor(1, 1, 1, 1);

	wnd->resize({600, 500});
	wnd->add_states(PWRE_STATE_VISIBLE);
	wnd->move();

	glViewport(0, 0, 600, 500);

	while (pwre::checkout_events()) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBegin(GL_TRIANGLES);

		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 1.0, 0.0);

		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(-1.0, -1.0, 0.0);

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(1.0, -1.0, 0.0);

		glEnd();
		glc->swap_buffers();
	}
	return 0;
}
