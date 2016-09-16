#ifndef _UBWINDOW_PRIVATE_H
#define _UBWINDOW_PRIVATE_H

#include "ubwindow.h"
#include <stdio.h>
#include <stdlib.h>

static int ubwSum;

typedef struct _Ubw {
	void* pNtv;
	UbwBounds borders;
	UbwBounds paddings;
	UbwSize szNonCont;
} _Ubw;

#endif // !_UBWINDOW_PRIVATE_H
