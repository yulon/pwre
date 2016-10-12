#if !defined(PWRE_WIN32) && !defined(PWRE_X11) && !defined(PWRE_COCOA)
	#if defined(_WIN32)
		#define PWRE_WIN32
	#elif defined(__APPLE__) && defined(__MACH__)
		#define PWRE_COCOA
	#elif defined(__unix__)
		#define PWRE_X11
	#endif
#endif
