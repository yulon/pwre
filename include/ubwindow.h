#ifndef _UBWINDOW_H
#define _UBWINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

int ubwInit(void);
int ubwHandleEvent(void);
void ubwHandleEventsAndStuck(void);

typedef void *UBW;
UBW ubwCreate(void);
void ubwClose(UBW);

void *ubwNativePointer(UBW);

void ubwShow(UBW);
void ubwHide(UBW);
int ubwIsVisible(UBW);
void ubwActive(UBW);

int ubwGetTitle(UBW, char *);
void ubwSetTitle(UBW, const char *);

void ubwMove(UBW, int x, int y);
void ubwMoveToScreenCenter(UBW);

void ubwSize(UBW, int *width, int *height);
void ubwResize(UBW, int width, int height);

void ubwSetView(UBW, int);
int ubwGetView(UBW);
#define UBW_VIEW_MAXIMIZE 0
#define UBW_VIEW_MINIMIZE 1
#define UBW_VIEW_RESTORE 2
#define UBW_VIEW_NORMAL 3

#ifdef __cplusplus
}
#endif

#endif // !_UBWINDOW_H
