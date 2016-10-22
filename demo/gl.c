#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwre.h>

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

int main(void) {
	pwre_init(NULL);

	pwre_wnd_t wnd = pwre_new_wnd_with_gl(0);
	pwre_wnd_resize(wnd, 600, 500);
	pwre_gl_make_current(wnd);
	pwre_wnd_retitle(wnd, (const char *)glGetString(GL_VERSION));
	glClearColor(1, 1, 1, 1);
	pwre_wnd_state_add(wnd, PWRE_STATE_VISIBLE);
	pwre_wnd_move(wnd, PWRE_MOVE_AUTO, PWRE_MOVE_AUTO);

	while (pwre_step()) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBegin(GL_TRIANGLES);

		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 1.0, 0.0);

		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(-1.0, -1.0, 0.0);

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(1.0, -1.0, 0.0);

		glEnd();
		pwre_gl_swap_buffers(wnd);
	}
	return 0;
}
