#include "mutex.h"
#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	#include <pthread.h>
#else
	#pragma message("Mutex: not support this OS!")
#endif

Mutex new_Mutex(void) {
	#if defined(_WIN32)
		return (Mutex)CreateMutexW(NULL, FALSE, NULL);
	#elif defined(__unix__)
		pthread_mutex_t *ptMux = calloc(1, sizeof(pthread_mutex_t));
		pthread_mutex_init(ptMux, NULL);
		return (Mutex)ptMux;
	#else
		return NULL;
	#endif
}

void Mutex_lock(Mutex mux) {
	if (mux) {
		#if defined(_WIN32)
			WaitForSingleObject((HANDLE)mux, INFINITE);
		#elif defined(__unix__)
			pthread_mutex_lock((pthread_mutex_t *)mux);
		#endif
	}
}

void Mutex_unlock(Mutex mux) {
	if (mux) {
		#if defined(_WIN32)
			ReleaseMutex((HANDLE)mux);
		#elif defined(__unix__)
			pthread_mutex_unlock((pthread_mutex_t *)mux);
		#endif
	}
}

void Mutex_free(Mutex mux) {
	if (mux) {
		#if defined(_WIN32)
			CloseHandle((HANDLE)mux);
		#elif defined(__unix__)
			pthread_mutex_destroy((pthread_mutex_t *)mux);
			free(mux);
		#endif
	}
}
