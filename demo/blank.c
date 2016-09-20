#include <stdlib.h>
#include <stdio.h>
#include <ubwindow.h>

int main(void) {
	ubwInit();

	UBW wnd = ubwCreate();
	ubwSetTitle(wnd, "我只是一个空白窗口_(:з」∠)_");
	ubwResize(wnd, 500, 500);
	ubwShow(wnd);
	ubwMoveToScreenCenter(wnd);

	int titleLen = ubwGetTitle(wnd, NULL);
	char *title = calloc(titleLen + 1, sizeof(char));
	ubwGetTitle(wnd, title);
	printf("%d %s\n", titleLen, title);

	ubwHandleEventsAndStuck();
	return 0;
}
