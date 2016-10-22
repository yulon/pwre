#include <stdlib.h>
#include <stdio.h>
#include <pwre.h>

bool event_handler(pwre_wnd_t wnd, PWRE_EVENT event, void *data) {
	printf("event: %d\n", event);
	return true;
}

int main(void) {
	pwre_init(event_handler);

	pwre_wnd_t wnd = pwre_new_wnd(0);
	pwre_wnd_retitle(wnd, "我只是一个空白窗口_(:з」∠)_");
	pwre_wnd_resize(wnd, 500, 500);
	pwre_wnd_state_add(wnd, PWRE_STATE_VISIBLE);
	pwre_wnd_move(wnd, PWRE_MOVE_AUTO, PWRE_MOVE_AUTO);

	printf("title: %s\n", pwre_wnd_title(wnd));

	pwre_run();
	return 0;
}
