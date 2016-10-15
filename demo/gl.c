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

	PrWnd wnd = new_PrWnd_with_GL(0);
	PrWnd_ReSize(wnd, 600, 500);
	PrWnd_GL_MakeCurrent(wnd);
	PrWnd_SetTitle(wnd, (const char *)glGetString(GL_VERSION));
	glClearColor(1, 1, 1, 1);
	PrWnd_View(wnd, PWRE_VIEW_VISIBLE);
	PrWnd_Move(wnd, PWRE_POS_AUTO, PWRE_POS_AUTO);

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
		PrWnd_GL_SwapBuffers(wnd);
	}
	return 0;
}
