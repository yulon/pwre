#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwre.h>

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

bool eventHandler(PrWnd wnd, PWRE_EVENT event, void *data) {
	if (event == PWRE_EVENT_PAINT) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBegin(GL_TRIANGLES);

		glColor4f(1.0, 0.0, 0.0, 0.0);
		glVertex3f(0.0, 1.0, 0.0);

		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(-1.0, -1.0, 0.0);

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(1.0, -1.0, 0.0);

		glEnd();
		PrWnd_GL_swapBuffers(wnd);
	}
	return true;
}

int main(void) {
	pwreInit(eventHandler);

	PrWnd wnd = new_PrWnd_with_GL(PWRE_POS_AUTO, PWRE_POS_AUTO, 600, 500, PWRE_GL_ALPHA);
	PrWnd_GL_makeCurrent(wnd);
	PrWnd_setTitle(wnd, (const char *)glGetString(GL_VERSION));
	glClearColor(0, 0, 0, 0.4);
	PrWnd_view(wnd, PWRE_VIEW_VISIBLE);

	pwreRun();
	return 0;
}
