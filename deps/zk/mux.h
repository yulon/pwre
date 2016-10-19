#ifndef _ZK_MUX_H
#define _ZK_MUX_H

typedef struct ZKMux *ZKMux;

#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	#include <pthread.h>
#else
	#pragma message("ZKMux: not support this OS!")
#endif

static inline ZKMux new_ZKMux(void) {
	#if defined(_WIN32)
		HANDLE ntvMux = CreateMutexW(NULL, FALSE, NULL);
		if (!ntvMux) {
			puts("ZK: Win32.CreateMutexW error!");
			exit(1);
		}
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_mutex_t *ntvMux = calloc(1, sizeof(pthread_mutex_t));
		pthread_mutex_init(ntvMux, NULL);
		if (!ntvMux) {
			puts("ZK: pthread_mutex_init error!");
			exit(1);
		}
	#endif
	return (ZKMux)ntvMux;
}

static inline void ZKMux_Lock(ZKMux mux) {
	#if defined(_WIN32)
		WaitForSingleObject((HANDLE)mux, INFINITE);
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_mutex_lock((pthread_mutex_t *)mux);
	#endif
}

static inline void ZKMux_UnLock(ZKMux mux) {
	#if defined(_WIN32)
		ReleaseMutex((HANDLE)mux);
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_mutex_unlock((pthread_mutex_t *)mux);
	#endif
}

static inline void ZKMux_Free(ZKMux mux) {
	#if defined(_WIN32)
		CloseHandle((HANDLE)mux);
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_mutex_destroy((pthread_mutex_t *)mux);
		free(mux);
	#endif
}

#endif // !_ZK_MUX_H
