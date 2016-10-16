#ifndef _PWRE_H
#define _PWRE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int width, height;
} PrSize;

typedef struct {
	int x, y;
} PrPoint;

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

bool pwre_init(PrEventHandler);
bool pwre_step(void);
void pwre_run(void);

#define PWRE_MASK_ALPHA 0x00000001 // support for Windows Vista+ (suggest less), X11 (only GL), macOS.
#define PWRE_MASK_BLUR 0x00000010 // support for Windows Vista/7 (only Aero Glass), Windows 10 (unpublished API, not perfect, suggest less), macOS.
#define PWRE_MASK_FULLWMBG 0x00000002 // support for Windows Vista+ (Aero Glass will automatically blur), macOS.

PrWnd new_PrWnd(uint64_t mask);
void PrWnd_Close(PrWnd);
void PrWnd_Destroy(PrWnd);

void *PrWnd_NativePointer(PrWnd);

PrEventHandler PrWnd_GetEventHandler(PrWnd);
void PrWnd_SetEventHandler(PrWnd, PrEventHandler);

const char *PrWnd_GetTitle(PrWnd);
void PrWnd_SetTitle(PrWnd, const char *);

#define PWRE_POS_AUTO -10101

void PrWnd_Move(PrWnd, int x, int y);

void PrWnd_Size(PrWnd, int *width, int *height);
void PrWnd_ReSize(PrWnd, int width, int height);

typedef enum {
	PWRE_VIEW_VISIBLE = 1,
	PWRE_VIEW_MINIMIZE = 2,
	PWRE_VIEW_MAXIMIZE = 3,
	PWRE_VIEW_FULLSCREEN = 4
} PWRE_VIEW;

void PrWnd_View(PrWnd, PWRE_VIEW);
void PrWnd_UnView(PrWnd, PWRE_VIEW);
bool PrWnd_Viewed(PrWnd, PWRE_VIEW);

void PrWnd_Less(PrWnd, bool);

typedef struct {
	int left, top, right, bottom;
} PrBounds;

typedef struct {
	PrBounds border;
	PrBounds padding;
} PrArea;

bool PrWnd_CustomArea(PrWnd, PrArea *);

#define PWRE_MASK_GL_V3 0x3000000000

PrWnd new_PrWnd_with_GL(uint64_t mask);
void PrWnd_GL_MakeCurrent(PrWnd);
void PrWnd_GL_SwapBuffers(PrWnd);

#ifdef __cplusplus
}
#endif

#endif // !_PWRE_H
