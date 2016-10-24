#ifndef _ZK_MUTEX_H
#define _ZK_MUTEX_H

typedef struct zk_mutex *zk_mutex_t;

#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	#include <pthread.h>
#else
	#pragma message("ZK.mutex: not support this OS!")
#endif

static inline zk_mutex_t zk_new_mutex(void) {
	#if defined(_WIN32)
		HANDLE ntv_mux = CreateMutexW(NULL, FALSE, NULL);
		if (!ntv_mux) {
			puts("ZK.mutex: Win32.CreateMutexW error!");
			exit(1);
		}
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_mutex_t *ntv_mux = calloc(1, sizeof(pthread_mutex_t));
		int result = pthread_mutex_init(ntv_mux, NULL);
		if (result) {
			puts("ZK.mutex: POSIX.pthread_mutex_init error!");
			exit(1);
		}
	#endif
	return (zk_mutex_t)ntv_mux;
}

static inline void zk_mutex_lock(zk_mutex_t mux) {
	#if defined(_WIN32)
		WaitForSingleObject((HANDLE)mux, INFINITE);
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_mutex_lock((pthread_mutex_t *)mux);
	#endif
}

static inline void zk_mutex_unlock(zk_mutex_t mux) {
	#if defined(_WIN32)
		ReleaseMutex((HANDLE)mux);
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_mutex_unlock((pthread_mutex_t *)mux);
	#endif
}

static inline void zk_mutex_free(zk_mutex_t mux) {
	#if defined(_WIN32)
		CloseHandle((HANDLE)mux);
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_mutex_destroy((pthread_mutex_t *)mux);
		free(mux);
	#endif
}

#endif // !_ZK_MUTEX_H
