#include <stdlib.h>
#include <stdio.h>
#include <pwre.h>

bool eventHandler(PrWnd wnd, PrEvent event, void *data) {
	printf("event: %d\n", event);
	return false;
}

int main(void) {
	pwreInit(eventHandler);

	PrWnd wnd = new_PrWnd();
	PrWnd_setTitle(wnd, "我只是一个空白窗口_(:з」∠)_");
	PrWnd_resize(wnd, 500, 500);
	PrWnd_view(wnd, PrView_visible);
	PrWnd_moveToScreenCenter(wnd);

	printf("title: %s\n", PrWnd_getTitle(wnd));

	pwreRun();
	return 0;
}
