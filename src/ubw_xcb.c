#include "ubw.h"

#ifdef UBWINDOW_XCB

#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>

static Display* dpy;
xcb_connection_t* conn;
static int scrDft;
xcb_screen_t* scr = 0;

static _Ubw* _UbwList[256];

int ubwInit() {
	XInitThreads();

	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		puts("UBWindow: Xlib.XOpenDisplay error!");
		return NULL;
	}

	scrDft = DefaultScreen(dpy);

	conn = XGetXCBConnection(dpy);
	if (!conn)
	{
		XCloseDisplay(dpy);
		puts("UBWindow: Xlib_xcb.XGetXCBConnection error!");
		return -1;
	}

	XSetEventQueueOwner(dpy, XCBOwnsEventQueue);


	/*xcb_screen_iterator_t scr_iter = xcb_setup_roots_iterator(xcb_get_setup(conn));
	for (
		int scr_num = scrDft;
		scr_iter.rem && scr_num > 0;
		--scr_num, xcb_screen_next(&scr_iter)
	) {
		scr = scr_iter.data;
	}*/

	scr = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
}

static void _UbwListAdd(_Ubw* wnd) {
	for (int i = 0; i < 256; i++) {
		if (!_UbwList[i]) {
			_UbwList[i] = wnd;
			return;
		}
	}
	puts("UBWindow: Create the number exceeds the upper limit!");
}

static int _UbwListIx(xcb_window_t xWnd) {
	for (int i = 0; i < 256; i++) {
		if (!_UbwList[i]) {
			return -1;
		}
		if ((xcb_window_t)_UbwList[i]->pNtv == xWnd) {
			return i;
		}
	}
}

static xcb_generic_event_t* event;

int ubwHandleEvent() {
	if (event = xcb_wait_for_event(conn)) {
		switch (event->response_type & ~0x80) {
			UbwRect r = {};
			int ixWnd;

		case XCB_CONFIGURE_NOTIFY:
			printf( "XCB_CONFIGURE_NOTIFY %d %d %d %d\n", ((xcb_configure_notify_event_t*)event)->x, ((xcb_configure_notify_event_t*)event)->y, ((xcb_configure_notify_event_t*)event)->width, ((xcb_configure_notify_event_t*)event)->height);
			break;

		case XCB_EXPOSE:
			ixWnd = _UbwListIx(((xcb_expose_event_t*)event)->window);
			if (ixWnd != -1) {
				ubwGetRect(_UbwList[ixWnd], NULL);
			}
			break;

		case XCB_KEY_PRESS:
			ixWnd = _UbwListIx(((xcb_key_press_event_t*)event)->event);
			if (ixWnd != -1) {
				ubwSum--;
				if (!ubwSum) {
					XCloseDisplay(dpy);
					return 0;
				}
				_UbwList[ixWnd] = NULL;
			}
			break;

		}
		free(event);
		event = NULL;
		xcb_flush(conn);
		return 1;
	}
	return 0;
}

Ubw ubwCreate() {
	_Ubw* wnd = calloc(1, sizeof(_Ubw));

	wnd->pNtv = (void*)xcb_generate_id(conn);

	uint32_t mask = XCB_CW_EVENT_MASK | XCB_CW_BACK_PIXEL;
	uint32_t value[2];
	value[0] = scr->white_pixel;
	value[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

	xcb_create_window(
		conn,
		XCB_COPY_FROM_PARENT,
		(xcb_window_t)wnd->pNtv,
		scr->root,
		0, 0,
		10, 10,
		50,
		XCB_WINDOW_CLASS_INPUT_OUTPUT,
		scr->root_visual,
		mask,
		value
	);

	wnd->rect.width = 100;
	wnd->rect.height = 100;

	wnd->szNonCont.width = 10;
	wnd->szNonCont.height = 50;

	ubwSum++;
	_UbwListAdd(wnd);
	return (Ubw)wnd;
}

#define _UBW_XWND (xcb_window_t)((_Ubw*)wnd)->pNtv

int ubwGetTitle(Ubw wnd, char* str8) {

	return 0;
}

int ubwSetTitle(Ubw wnd, char* str8) {
	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, _UBW_XWND, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(str8), str8);
	return 1;
}

void ubwGetRect(Ubw wnd, UbwRect* pRect) {
	xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn, _UBW_XWND);
	xcb_get_geometry_reply_t* reply = xcb_get_geometry_reply(conn, cookie, NULL);
	printf("xcb_get_geometry %d %d %d %d\n", reply->x, reply->y, reply->width, reply->height);
}

void ubwSetRect(Ubw wnd, UbwRect rect) {
	uint32_t values[4] = { rect.left , rect.top, rect.width, rect.height };
	xcb_configure_window(conn, _UBW_XWND, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
}

void ubwMove(Ubw wnd, UbwPoint point) {

}

void ubwMoveToScreenCenter(Ubw wnd) {
	UbwRect rect = { 0, 0, 100, 100 };
	ubwGetRect(wnd, &rect);
	rect.left = (DisplayWidth(dpy, 0) - rect.width) / 2;
	rect.top = (DisplayHeight(dpy, 0) - rect.height) / 2;
	ubwSetRect(wnd, rect);
}

void ubwResize(Ubw wnd, UbwSize size) {

}

void ubwShow(Ubw wnd) {
	xcb_map_window(conn, _UBW_XWND);
	xcb_flush(conn);
}

void ubwHide(Ubw wnd) {
	xcb_unmap_window(conn, _UBW_XWND);
}

#undef _UBW_XWND
#endif // UBWINDOW_XCB
