#include <stdlib.h>
#include <stdio.h>
#include <ubwindow.h>

bool eventHandler(Ubw wnd, int event, void *data) {
	printf("event: %d\n", event);
	return false;
}

int main(void) {
	ubwInit(eventHandler);

	Ubw wnd = ubwCreate();
	ubwSetTitle(wnd, "我只是一个空白窗口_(:з」∠)_");
	ubwResize(wnd, 500, 500);
	ubwView(wnd, UBW_VIEW_VISIBLE);
	ubwMoveToScreenCenter(wnd);

	int titleLen = ubwGetTitle(wnd, NULL);
	char *title = calloc(titleLen + 1, sizeof(char));
	ubwGetTitle(wnd, title);
	printf("%d %s\n", titleLen, title);

	ubwRun();
	return 0;
}
