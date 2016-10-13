#ifndef _PWRE_H
#define _PWRE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int width, height;
} PrSize;

typedef struct {
	int x, y;
} PrPoint;

#define PrPos_ScreenCenter -10101

typedef struct PrWnd *PrWnd;

typedef enum {
	PrEvent_Close = -1,
	PrEvent_Destroy = -2,
	PrEvent_Size = -10,
	PrEvent_Move = -20,
	PrEvent_Paint = -30,
	PrEvent_MouseMove = 10,
	PrEvent_MouseDown = 11,
	PrEvent_MouseUp = 12,
	PrEvent_KeyDown = 20,
	PrEvent_KeyUp = 21
} PrEvent;
typedef bool (*PrEventHandler)(PrWnd, PrEvent, void *data);

bool pwreInit(PrEventHandler);
bool pwreStep(void);
void pwreRun(void);

PrWnd new_PrWnd(int x, int y, int width, int height);
void PrWnd_close(PrWnd);
void PrWnd_destroy(PrWnd);

void *PrWnd_nativePointer(PrWnd);

PrEventHandler PrWnd_getEventHandler(PrWnd);
void PrWnd_setEventHandler(PrWnd, PrEventHandler);

const char *PrWnd_getTitle(PrWnd);
void PrWnd_setTitle(PrWnd, const char *);

void PrWnd_move(PrWnd, int x, int y);

void PrWnd_size(PrWnd, int *width, int *height);
void PrWnd_resize(PrWnd, int width, int height);

typedef enum {
	PrView_Visible = 1,
	PrView_Minimize = 2,
	PrView_Maximize = 3,
	PrView_Fullscreen = 4
} PrView;
void PrWnd_view(PrWnd, PrView);
void PrWnd_unview(PrWnd, PrView);
bool PrWnd_viewed(PrWnd, PrView);

PrWnd new_PrWnd_with_GL(int x, int y, int width, int height);
void PrWnd_GL_makeCurrent(PrWnd);
void PrWnd_GL_swapBuffers(PrWnd);

#ifdef __cplusplus
}
#endif

#endif // !_PWRE_H
