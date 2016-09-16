#include "ubwindow.h"

int main(void) {
	ubwInit();

	Ubw wnd = ubwCreate();
	ubwSetTitle(wnd, "Hello Window!");
	UbwSize contSize = { 500, 500 };
	ubwResizeFromContentSize(wnd, contSize);
	ubwMoveToScreenCenter(wnd);
	ubwShow(wnd);

	ubwHandleEventsAndStuck();
	return 0;
}
