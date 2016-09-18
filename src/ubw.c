#include "ubwindow.h"
#include "ubw.h"

void ubwHandleEventsAndStuck() {
	while (ubwHandleEvent());
}

void* ubwNativePointer(Ubw wnd) {
	return ((_Ubw*)wnd)->pNtv;
}

void ubwContentSize(Ubw wnd, UbwSize* pSize) {
	if (pSize) {
		UbwRect rect;
		ubwGetRect(wnd, &rect);
		pSize->width = rect.width - ((_Ubw*)wnd)->szNonCont.width;
		pSize->height = rect.height - ((_Ubw*)wnd)->szNonCont.height;
	}
}

void ubwResizeFromContentSize(Ubw wnd, UbwSize size) {
	UbwRect rect;
	ubwGetRect(wnd, &rect);
	rect.width = size.width + ((_Ubw*)wnd)->szNonCont.width;
	rect.height = size.height + ((_Ubw*)wnd)->szNonCont.height;
	ubwSetRect(wnd, rect);
}

void ubwGetPaddings(Ubw wnd, UbwBounds* pPaddings) {
	if (pPaddings) {
		*pPaddings = ((_Ubw*)wnd)->paddings;
	}
}

void ubwGetBorders(Ubw wnd, UbwBounds* pBorders) {
	if (pBorders) {
		*pBorders = ((_Ubw*)wnd)->borders;
	}
}
