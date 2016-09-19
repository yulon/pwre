#include "ubwindow.h"
#include "ubw.h"

void ubwHandleEventsAndStuck(void) {
	while (ubwHandleEvent());
}

void *ubwNativePointer(UBW wnd) {
	return ((_UBWPVT *)wnd)->pNtv;
}
