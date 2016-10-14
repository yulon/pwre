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

#define PWRE_POS_AUTO -10101

typedef struct PrWnd *PrWnd;

typedef enum {
	PWRE_EVENT_CLOSE = -1,
	PWRE_EVENT_DESTROY = -2,
	PWRE_EVENT_SIZE = -10,
	PWRE_EVENT_MOVE = -20,
	PWRE_EVENT_PAINT = -30,
	PWRE_EVENT_MOUSE_MOVE = 10,
	PWRE_EVENT_MOUSE_DOWN = 11,
	PWRE_EVENT_MOUSE_UP = 12,
	PWRE_EVENT_KEY_DOWN = 20,
	PWRE_EVENT_KEY_UP = 21
} PWRE_EVENT;

typedef bool (*PrEventHandler)(PrWnd, PWRE_EVENT, void *data);

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
	PWRE_VIEW_VISIBLE = 1,
	PWRE_VIEW_MINIMIZE = 2,
	PWRE_VIEW_MAXIMIZE = 3,
	PWRE_VIEW_FULLSCREEN = 4
} PWRE_VIEW;

void PrWnd_view(PrWnd, PWRE_VIEW);
void PrWnd_unview(PrWnd, PWRE_VIEW);
bool PrWnd_viewed(PrWnd, PWRE_VIEW);

#define PWRE_GL_V3 0x0003
#define PWRE_GL_ALPHA 0x0010

PrWnd new_PrWnd_with_GL(int x, int y, int width, int height, int flags);
void PrWnd_GL_makeCurrent(PrWnd);
void PrWnd_GL_swapBuffers(PrWnd);

#ifdef __cplusplus
}
#endif

#endif // !_PWRE_H
