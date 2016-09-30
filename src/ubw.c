#include "ubw.h"

void *ubwNativePointer(Ubw wnd) {
	return ((_UbwPvt)wnd)->ntvPtr;
}
