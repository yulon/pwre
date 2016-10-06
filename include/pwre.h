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

typedef void *PrWnd;

typedef enum {
	PrEvent_close = -1,
	PrEvent_destroy = -2,
	PrEvent_size = -10,
	PrEvent_move = -20,
	PrEvent_paint = -30,
	PrEvent_mouseMove = 10,
	PrEvent_mouseDown = 11,
	PrEvent_mouseUp = 12,
	PrEvent_keyDown = 20,
	PrEvent_keyUp = 21
} PrEvent;
typedef bool (*PrEventHandler)(PrWnd, PrEvent, void *data);

bool pwreInit(PrEventHandler);
bool pwreStep(void);
void pwreRun(void);

PrWnd new_PrWnd(void);
void PrWnd_close(PrWnd);
void PrWnd_destroy(PrWnd);

void *PrWnd_nativePointer(PrWnd);

int PrWnd_getTitle(PrWnd, char *);
void PrWnd_setTitle(PrWnd, const char *);

void PrWnd_move(PrWnd, int x, int y);
void PrWnd_moveToScreenCenter(PrWnd);

void PrWnd_size(PrWnd, int *width, int *height);
void PrWnd_resize(PrWnd, int width, int height);

typedef enum {
	PrView_visible = 1,
	PrView_minimize = 2,
	PrView_maximize = 3,
	PrView_fullscreen = 4
} PrView;
void PrWnd_view(PrWnd, PrView);
void PrWnd_unview(PrWnd, PrView);
bool PrWnd_viewed(PrWnd, PrView);

#ifdef __cplusplus
}
#endif

#endif // !_PWRE_H
