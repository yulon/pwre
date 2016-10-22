#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwre.h>

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

bool event_handler(pwre_wnd_t wnd, PWRE_EVENT event, void *data) {
	if (event == PWRE_EVENT_PAINT) {
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
	return true;
}

int main(void) {
	pwre_init(event_handler);

	pwre_wnd_t wnd = pwre_new_wnd_with_gl(PWRE_HINT_ALPHA);
	pwre_wnd_less(wnd, true);
	pwre_wnd_resize(wnd, 600, 500);
	pwre_gl_make_current(wnd);
	pwre_wnd_retitle(wnd, (const char *)glGetString(GL_VERSION));
	glClearColor(0, 0, 0, 0.4);

	pwre_wnd_state_add(wnd, PWRE_STATE_VISIBLE);
	pwre_wnd_move(wnd, PWRE_MOVE_AUTO, PWRE_MOVE_AUTO);

	pwre_run();
	return 0;
}
