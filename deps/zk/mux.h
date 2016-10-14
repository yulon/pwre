#ifndef _ZK_MUX_H
#define _ZK_MUX_H

#ifndef _ZK_LINK
#define _ZK_LINK(p, n) ___##p##___##n
#define _ZK_LINK_M(p, n) _ZK_LINK(p, n)
#endif

#ifdef ZK_SCOPE
#define _ZK_NAME(n) _ZK_LINK_M(ZK_SCOPE, n)
#else
#define _ZK_NAME(n) _ZK_LINK(zk, n)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ZKMux *ZKMux;

ZKMux _ZK_NAME(new_ZKMux)(void);
#define new_ZKMux() _ZK_NAME(new_ZKMux)()

void _ZK_NAME(ZKMux_lock)(ZKMux);
#define ZKMux_lock(ZKMux) _ZK_NAME(ZKMux_lock)(ZKMux)

void _ZK_NAME(ZKMux_unlock)(ZKMux);
#define  ZKMux_unlock(ZKMux) _ZK_NAME(ZKMux_unlock)(ZKMux)

void _ZK_NAME(ZKMux_free)(ZKMux);
#define  ZKMux_free(ZKMux) _ZK_NAME(ZKMux_free)(ZKMux)

#ifdef __cplusplus
}
#endif

#ifdef ZK_IMPL
#ifndef _ZK_MUX_BODY
#define _ZK_MUX_BODY

#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	#include <pthread.h>
#else
	#pragma message("ZKMux: not support this OS!")
#endif

ZKMux _ZK_NAME(new_ZKMux)(void) {
	#if defined(_WIN32)
		return (ZKMux)CreateMutexW(NULL, FALSE, NULL);
	#elif defined(__unix__)
		pthread_mutex_t *ptMux = calloc(1, sizeof(pthread_mutex_t));
		pthread_mutex_init(ptMux, NULL);
		return (ZKMux)ptMux;
	#else
		return NULL;
	#endif
}

void _ZK_NAME(ZKMux_lock)(ZKMux mux) {
	if (mux) {
		#if defined(_WIN32)
			WaitForSingleObject((HANDLE)mux, INFINITE);
		#elif defined(__unix__)
			pthread_mutex_lock((pthread_mutex_t *)mux);
		#endif
	}
}

void _ZK_NAME(ZKMux_unlock)(ZKMux mux) {
	if (mux) {
		#if defined(_WIN32)
			ReleaseMutex((HANDLE)mux);
		#elif defined(__unix__)
			pthread_mutex_unlock((pthread_mutex_t *)mux);
		#endif
	}
}

void _ZK_NAME(ZKMux_free)(ZKMux mux) {
	if (mux) {
		#if defined(_WIN32)
			CloseHandle((HANDLE)mux);
		#elif defined(__unix__)
			pthread_mutex_destroy((pthread_mutex_t *)mux);
			free(mux);
		#endif
	}
}

#endif // !_ZK_MUX_BODY
#endif // !ZK_IMPL

#endif // !_ZK_MUX_H
