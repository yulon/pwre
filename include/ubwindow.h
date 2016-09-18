#ifndef _UBWINDOW_H
#define _UBWINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

int ubwInit();
int ubwHandleEvent();
void ubwHandleEventsAndStuck();

typedef struct UbwRect {
	int left, top, width, height;
} UbwRect;

typedef struct UbwSize {
	int width, height;
} UbwSize;

typedef struct UbwPoint {
	int left, top;
} UbwPoint;

typedef struct UbwBounds {
	int left, top, right, bottom;
} UbwBounds;

typedef void* Ubw;
Ubw ubwCreate();
void ubwClose(Ubw);

void* ubwNativePointer(Ubw);

void ubwShow(Ubw);
void ubwHide(Ubw);
int ubwIsVisible(Ubw);
void ubwActive(Ubw);

int ubwGetTitle(Ubw, char*);
int ubwSetTitle(Ubw, char*);

void ubwGetRect(Ubw, UbwRect*);
void ubwSetRect(Ubw, UbwRect);

void ubwMove(Ubw, UbwPoint);
void ubwMoveToScreenCenter(Ubw);

void ubwSize(Ubw, UbwSize*);
void ubwResize(Ubw, UbwSize);
void ubwContentSize(Ubw, UbwSize*);
void ubwResizeFromContentSize(Ubw, UbwSize);

void ubwSetView(Ubw, int);
int ubwGetView(Ubw);
#define UBW_VIEW_MAXIMIZE 0
#define UBW_VIEW_MINIMIZE 1
#define UBW_VIEW_RESTORE 2
#define UBW_VIEW_NORMAL 3

void ubwGetPaddings(Ubw, UbwBounds*);
void ubwSetPaddings(Ubw, UbwBounds);
void ubwGetBorders(Ubw, UbwBounds*);
void ubwSetBorders(Ubw, UbwBounds);

#ifdef __cplusplus
}
#endif

#endif // !_UBWINDOW_H
