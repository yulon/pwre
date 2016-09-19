#ifndef _UBWINDOW_H
#define _UBWINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

int ubwInit();
int ubwHandleEvent();
void ubwHandleEventsAndStuck();

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

void ubwMove(Ubw, int x, int y);
void ubwMoveToScreenCenter(Ubw);

void ubwSize(Ubw, int* width, int* height);
void ubwResize(Ubw, int width, int height);

void ubwSetView(Ubw, int);
int ubwGetView(Ubw);
#define UBW_VIEW_MAXIMIZE 0
#define UBW_VIEW_MINIMIZE 1
#define UBW_VIEW_RESTORE 2
#define UBW_VIEW_NORMAL 3

#ifdef __cplusplus
}
#endif

#endif // !_UBWINDOW_H
