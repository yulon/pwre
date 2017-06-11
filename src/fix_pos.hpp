static inline void fix_pos(int &x, int &y, int sx, int sy, int width, int height) {
	switch (x) {
		case PWRE_MOVE_CENTER:
			x = (_SCREEN_W - width) / 2;
			break;
		case PWRE_NULL:
			x = sx;
	}
	switch (y) {
		case PWRE_MOVE_CENTER:
			y = (_SCREEN_H - height) / 2;
			break;
		case PWRE_NULL:
			y = sy;
	}
}

#undef _SCREEN_W
#undef _SCREEN_H
