#ifndef _PWRE_PDBE_H
#define _PWRE_PDBE_H

#if !defined(PWRE_BE_WIN32) && !defined(PWRE_BE_X11) && !defined(PWRE_BE_COCOA)
	#if defined(_WIN32)
		#define PWRE_BE_WIN32
	#elif defined(__APPLE__) && defined(__MACH__)
		#define PWRE_BE_COCOA
	#elif defined(__unix__)
		#define PWRE_BE_X11
	#endif
#endif

#endif // !_PWRE_PDBE_H
