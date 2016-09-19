#include "ubwindow.h"
#include "ubw.h"

void ubwHandleEventsAndStuck() {
	while (ubwHandleEvent());
}

void* ubwNativePointer(Ubw wnd) {
	return ((_Ubw*)wnd)->pNtv;
}
