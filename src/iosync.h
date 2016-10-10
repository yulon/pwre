#ifndef _IOSYNC_H
#define _IOSYNC_H

#if defined(_WIN32)
	#include <windows.h>
#elif defined(__unix__)
	#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IosyMtx *IosyMtx;

IosyMtx new_IosyMtx(void) {
	#if defined(_WIN32)
		return (IosyMtx)CreateMutexW(NULL, FALSE, NULL);
	#elif defined(__unix__)
		return (IosyMtx)&(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	#else
		return NULL;
	#endif
}

void IosyMtx_lock(IosyMtx mtx) {
	if (mtx) {
		#if defined(_WIN32)
			WaitForSingleObject((HANDLE)mtx, INFINITE);
		#elif defined(__unix__)
			pthread_mutex_lock((pthread_mutex_t *)mtx);
		#endif
	}
}

void IosyMtx_unlock(IosyMtx mtx) {
	if (mtx) {
		#if defined(_WIN32)
			ReleaseMutex((HANDLE)mtx);
		#elif defined(__unix__)
			pthread_mutex_unlock((pthread_mutex_t *)mtx);
		#endif
	}
}

void IosyMtx_free(IosyMtx mtx) {
	if (mtx) {
		#if defined(_WIN32)
			CloseHandle((HANDLE)mtx);
		#elif defined(__unix__)
			pthread_mutex_destroy((pthread_mutex_t *)mtx);
		#endif
	}
}

#ifdef __cplusplus
}
#endif

#endif // !_IOSYNC_H
