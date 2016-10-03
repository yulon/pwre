#ifndef _UBWINDOW_H
#define _UBWINDOW_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UbwSize {
	int width, height;
} UbwSize;

typedef struct UbwPoint {
	int x, y;
} UbwPoint;

typedef void *Ubw;

typedef bool (*UbwEventHandler)(Ubw, int event, void *data);
#define UBW_EVENT_CLOSE -1
#define UBW_EVENT_DESTROY -2
#define UBW_EVENT_SIZE -10
#define UBW_EVENT_MOVE -20
#define UBW_EVENT_PAINT -30
#define UBW_EVENT_MOUSE_MOVE 10
#define UBW_EVENT_MOUSE_DOWN 11
#define UBW_EVENT_MOUSE_UP 12
#define UBW_EVENT_KEY_DOWN 20
#define UBW_EVENT_KEY_UP 21

bool ubwInit(UbwEventHandler);
bool ubwStep(void);
void ubwRun(void);

Ubw ubwCreate(void);
void ubwClose(Ubw);
void ubwDestroy(Ubw);

void *ubwNativePointer(Ubw);

int ubwGetTitle(Ubw, char *);
void ubwSetTitle(Ubw, const char *);

void ubwMove(Ubw, int x, int y);
void ubwMoveToScreenCenter(Ubw);

void ubwSize(Ubw, int *width, int *height);
void ubwResize(Ubw, int width, int height);

int ubwView(Ubw, int);
#define UBW_VIEW_HIDDEN 1
#define UBW_VIEW_VISIBLE 2
#define UBW_VIEW_ADJUSTABLE 3
#define UBW_VIEW_MINIMIZATION 4
#define UBW_VIEW_MAXIMIZATION 5
#define UBW_VIEW_FULLSCREEN 6

#ifdef __cplusplus
}
#endif

#endif // !_UBWINDOW_H
