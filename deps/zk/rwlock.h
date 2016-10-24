#ifndef _ZK_RWLOCK_H
#define _ZK_RWLOCK_H

typedef struct zk_rwlock *zk_rwlock_t;

#include <stdlib.h>
#include <stdbool.h>

#if defined(_WIN32)

	#include <windows.h>
	#include <string.h>
	#include "mutex.h"

	typedef VOID (WINAPI *SRWLockMethod_t)(PSRWLOCK SRWLock);

	struct zk_rwlock {
		struct _RTL_SRWLOCK owner;
		SRWLockMethod_t InitializeSRWLock;
		SRWLockMethod_t AcquireSRWLockExclusive;
		SRWLockMethod_t ReleaseSRWLockExclusive;
		SRWLockMethod_t AcquireSRWLockShared;
		SRWLockMethod_t ReleaseSRWLockShared;

		zk_mutex_t spare;
	};

	static struct zk_rwlock _zk_rwlock_prototype;

#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	#include <pthread.h>
#else
	#pragma message("ZK.rwlock: not support this OS!")
#endif

static inline zk_rwlock_t zk_new_rwlock(void) {
	#if defined(_WIN32)

		zk_rwlock_t lock = calloc(1, sizeof(struct zk_rwlock));

		if (_zk_rwlock_prototype.ReleaseSRWLockShared == NULL) {
			HMODULE k32 = LoadLibraryW(L"kernel32");

			#define _LOAD_METHOD(m) lock->m = (SRWLockMethod_t)GetProcAddress(k32, #m)

			_LOAD_METHOD(InitializeSRWLock);
			if (!lock->InitializeSRWLock) {
				lock->spare = zk_new_mutex();
				return lock;
			}
			_LOAD_METHOD(AcquireSRWLockExclusive);
			_LOAD_METHOD(ReleaseSRWLockExclusive);
			_LOAD_METHOD(AcquireSRWLockShared);
			_LOAD_METHOD(ReleaseSRWLockShared);

			#undef _LOAD_METHOD

			FreeLibrary(k32);

			lock->owner = (struct _RTL_SRWLOCK)RTL_SRWLOCK_INIT;

			if (_zk_rwlock_prototype.ReleaseSRWLockShared == NULL) {
				memcpy(&_zk_rwlock_prototype, lock, sizeof(struct zk_rwlock));
			}
		} else {
			memcpy(lock, &_zk_rwlock_prototype, sizeof(struct zk_rwlock));
		}

		lock->InitializeSRWLock(&lock->owner);

		return lock;

	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_rwlock_t *ntv_lock = calloc(1, sizeof(pthread_rwlock_t));
		int result = pthread_rwlock_init(ntv_lock, NULL);
		if (result) {
			puts("ZK.rwlock: POSIX.pthread_rwlock_init error!");
			exit(1);
		}
		return (zk_rwlock_t)ntv_lock;
	#endif
}

static inline void zk_rwlock_reading(zk_rwlock_t lock) {
	#if defined(_WIN32)
		if (lock->spare) {
			zk_mutex_lock(lock->spare);
		} else {
			lock->AcquireSRWLockShared(&lock->owner);
		}
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_rwlock_rdlock((pthread_rwlock_t *)lock);
	#endif
}

static inline void zk_rwlock_red(zk_rwlock_t lock) {
	#if defined(_WIN32)
		if (lock->spare) {
			zk_mutex_unlock(lock->spare);
		} else {
			lock->ReleaseSRWLockShared(&lock->owner);
		}
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_rwlock_unlock((pthread_rwlock_t *)lock);
	#endif
}

static inline void zk_rwlock_writing(zk_rwlock_t lock) {
	#if defined(_WIN32)
		if (lock->spare) {
			zk_mutex_lock(lock->spare);
		} else {
			lock->AcquireSRWLockExclusive(&lock->owner);
		}
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_rwlock_wrlock((pthread_rwlock_t *)lock);
	#endif
}

static inline void zk_rwlock_written(zk_rwlock_t lock) {
	#if defined(_WIN32)
		if (lock->spare) {
			zk_mutex_unlock(lock->spare);
		} else {
			lock->ReleaseSRWLockExclusive(&lock->owner);
		}
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_rwlock_unlock((pthread_rwlock_t *)lock);
	#endif
}

static inline void zk_rwlock_free(zk_rwlock_t lock) {
	#if defined(_WIN32)
		if (lock->spare) {
			zk_mutex_free(lock->spare);
		}
		free(lock);
	#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
		pthread_rwlock_destroy((pthread_rwlock_t *)lock);
		free(lock);
	#endif
}

#endif // !_ZK_RWLOCK_H
