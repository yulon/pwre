#include "ubwindow.h"

int main(void) {
	ubwInit();

	Ubw wnd = ubwCreate();
	ubwSetTitle(wnd, "Hello Window!");
	UbwSize size = { 500, 500 };
	ubwResizeFromContentSize(wnd, size);
	ubwShow(wnd);
	ubwMoveToScreenCenter(wnd);

	ubwHandleEventsAndStuck();
	return 0;
}
