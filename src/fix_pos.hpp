static inline void fix_pos(int &x, int &y, int width, int height) {
	if (x == PWRE_MOVE_AUTO) {
		x = (_SCREEN_W - width) / 2;
	}
	if (y == PWRE_MOVE_AUTO) {
		y = (_SCREEN_H - height) / 2;
	}
}
#undef _SCREEN_W
#undef _SCREEN_H
