#ifndef _UBWINDOW_PDBE_H
#define _UBWINDOW_PDBE_H

#if !defined(UBWINDOW_WIN32) && !defined(UBWINDOW_X11) && !defined(UBWINDOW_COCOA)
	#if defined(_WIN32)
		#define UBWINDOW_WIN32
	#elif defined(__APPLE__) && defined(__MACH__)
		#define UBWINDOW_COCOA
	#elif defined(__unix__)
		#define UBWINDOW_X11
	#endif
#endif

#endif // !_UBWINDOW_PDBE_H
