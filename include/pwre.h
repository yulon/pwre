#ifndef _PWRE_H
#define _PWRE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int x, y;
} pwre_point_t;

typedef struct {
	int width, height;
} pwre_size_t;

typedef struct {
	int left, top, right, bottom;
} pwre_bounds_t;

typedef struct pwre_wnd *pwre_wnd_t;

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

typedef bool (*pwre_event_handler_t)(pwre_wnd_t, PWRE_EVENT, void *data);

bool pwre_init(pwre_event_handler_t);
bool pwre_step(void);
void pwre_run(void);

#define PWRE_HINT_ALPHA 0x00000001 // support for Windows Vista+ (suggest less), X11 (only GL), macOS.
#define PWRE_HINT_BLUR 0x00000010 // support for Windows Vista/7 (only Aero Glass), Windows 10 (unpublished API, not perfect, suggest less), macOS.
#define PWRE_HINT_WMBACKGROUND 0x00000002 // support for Windows Vista+ (Aero Glass will automatically blur, but when less only shadow), macOS.

pwre_wnd_t pwre_new_wnd(uint64_t hints);
void pwre_wnd_close(pwre_wnd_t);
void pwre_wnd_destroy(pwre_wnd_t);

void *pwre_wnd_native_id(pwre_wnd_t);

const char *pwre_wnd_title(pwre_wnd_t);
void pwre_wnd_retitle(pwre_wnd_t, const char *);

#define PWRE_MOVE_AUTO -10101

void pwre_wnd_pos(pwre_wnd_t, int *x, int *y);
void pwre_wnd_move(pwre_wnd_t, int x, int y);

void pwre_wnd_size(pwre_wnd_t, int *width, int *height);
void pwre_wnd_resize(pwre_wnd_t, int width, int height);

typedef enum {
	PWRE_STATE_VISIBLE = 1,
	PWRE_STATE_MINIMIZE = 2,
	PWRE_STATE_MAXIMIZE = 3,
	PWRE_STATE_FULLSCREEN = 4
} PWRE_STATE;

void pwre_wnd_state_add(pwre_wnd_t, PWRE_STATE);
void pwre_wnd_state_rm(pwre_wnd_t, PWRE_STATE);
bool pwre_wnd_state_has(pwre_wnd_t, PWRE_STATE);

void pwre_wnd_less(pwre_wnd_t, bool);

typedef struct {
	pwre_bounds_t outer;
	pwre_bounds_t border;
	pwre_bounds_t control;
} pwre_action_area_t;

bool pwre_wnd_custom_action_area(pwre_wnd_t, pwre_action_area_t *);

#define PWRE_HINT_GL_V3 0x3000000000

pwre_wnd_t pwre_new_wnd_with_gl(uint64_t hints);
void pwre_gl_make_current(pwre_wnd_t);
void pwre_gl_swap_buffers(pwre_wnd_t);

#ifdef __cplusplus
}
#endif

#endif // !_PWRE_H
