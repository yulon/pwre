#include <stdlib.h>
#include <stdio.h>
#include <pwre.h>

bool eventHandler(PrWnd wnd, PWRE_EVENT event, void *data) {
	printf("event: %d\n", event);
	return true;
}

int main(void) {
	pwreInit(eventHandler);

	PrWnd wnd = new_PrWnd(PWRE_POS_AUTO, PWRE_POS_AUTO, 500, 500);
	PrWnd_setTitle(wnd, "我只是一个空白窗口_(:з」∠)_");
	PrWnd_view(wnd, PWRE_VIEW_VISIBLE);

	printf("title: %s\n", PrWnd_getTitle(wnd));

	pwreRun();
	return 0;
}
