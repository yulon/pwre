#include <stdlib.h>
#include <stdio.h>
#include <pwre.h>

bool eventHandler(PrWnd wnd, PWRE_EVENT event, void *data) {
	printf("event: %d\n", event);
	return true;
}

int main(void) {
	pwre_init(eventHandler);

	PrWnd wnd = new_PrWnd(0);
	PrWnd_SetTitle(wnd, "我只是一个空白窗口_(:з」∠)_");
	PrWnd_ReSize(wnd, 500, 500);
	PrWnd_View(wnd, PWRE_VIEW_VISIBLE);
	PrWnd_Move(wnd, PWRE_POS_AUTO, PWRE_POS_AUTO);

	printf("title: %s\n", PrWnd_GetTitle(wnd));

	pwre_run();
	return 0;
}
