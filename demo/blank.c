#include "ubwindow.h"

int main(void) {
	ubwInit();

	UBW wnd = ubwCreate();
	ubwSetTitle(wnd, "Hello Window!");
	ubwResize(wnd, 500, 500);
	ubwShow(wnd);
	ubwMoveToScreenCenter(wnd);

	ubwHandleEventsAndStuck();
	return 0;
}
