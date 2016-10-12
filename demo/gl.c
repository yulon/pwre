#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwre.h>
#include <GL/gl.h>

int main(void) {
	pwreInit(NULL);

	PrWnd wnd = new_PrWnd_with_GL(PrPos_ScreenCenter, PrPos_ScreenCenter, 600, 500);
	PrWnd_GL_makeCurrent(wnd);
	PrWnd_setTitle(wnd, (const char *)glGetString(GL_VERSION));
	glClearColor(1, 1, 1, 1);
	PrWnd_view(wnd, PrView_Visible);

	while (pwreStep()) {
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
	return 0;
}
