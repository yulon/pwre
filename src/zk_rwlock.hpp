#ifndef _ZK_RWLOCK_HPP
#define _ZK_RWLOCK_HPP

#if defined(_WIN32)
	#include <windows.h>
	#include <mutex>
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	#include <pthread.h>
	#include <iostream>
#else
	#pragma message("ZK::RWLock: not support this OS!")
#endif

namespace ZK {
	#if defined(_WIN32)
		namespace {
			typedef VOID (WINAPI *_SRWLockMethod)(PSRWLOCK SRWLock);
			_SRWLockMethod _InitializeSRWLock = NULL;
			_SRWLockMethod _AcquireSRWLockExclusive;
			_SRWLockMethod _ReleaseSRWLockExclusive;
			_SRWLockMethod _AcquireSRWLockShared;
			_SRWLockMethod _ReleaseSRWLockShared;
		}
	#endif

	class RWLock {
		public:
			static void Init() {
				#if defined(_WIN32)
					HMODULE k32 = LoadLibraryW(L"kernel32");

					#define _LOAD_METHOD(m) m = (_SRWLockMethod)GetProcAddress(k32, #m)

					_InitializeSRWLock = NULL;
					_LOAD_METHOD(_InitializeSRWLock);
					if (_InitializeSRWLock) {
						_LOAD_METHOD(_AcquireSRWLockExclusive);
						_LOAD_METHOD(_ReleaseSRWLockExclusive);
						_LOAD_METHOD(_AcquireSRWLockShared);
						_LOAD_METHOD(_ReleaseSRWLockShared);
					}

					#undef _LOAD_METHOD

					FreeLibrary(k32);
				#endif
			}

			RWLock(){
				#if defined(_WIN32)
					if (!_InitializeSRWLock) {
						StdMux = new std::mutex;
						return;
					}
					NtvRWLock = (struct _RTL_SRWLOCK)RTL_SRWLOCK_INIT;
					_InitializeSRWLock(&NtvRWLock);

				#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
					NtvRWLock = new pthread_rwlock_t;
					int result = pthread_rwlock_init(NtvRWLock, NULL);
					if (result) {
						std::cout << "ZK::RWLock: POSIX.pthread_rwlock_init error!" << std::endl;
						exit(1);
					}
				#endif
			}
			~RWLock(){
				#if defined(_WIN32)
					if (StdMux) {
						delete StdMux;
					}
				#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
					pthread_rwlock_destroy(NtvRWLock);
				#endif
			}

			void Reading(){
				#if defined(_WIN32)
					if (StdMux) {
						StdMux->lock();
					} else {
						_AcquireSRWLockShared(&NtvRWLock);
					}
				#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
					pthread_rwlock_rdlock(NtvRWLock);
				#endif
			}
			void Red(){
				#if defined(_WIN32)
					if (StdMux) {
						StdMux->unlock();
					} else {
						_ReleaseSRWLockShared(&NtvRWLock);
					}
				#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
					pthread_rwlock_unlock(NtvRWLock);
				#endif
			}

			void Writing() {
				#if defined(_WIN32)
					if (StdMux) {
						StdMux->lock();
					} else {
						_AcquireSRWLockExclusive(&NtvRWLock);
					}
				#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
					pthread_rwlock_wrlock(NtvRWLock);
				#endif
			}
			void Written() {
				#if defined(_WIN32)
					if (StdMux) {
						StdMux->unlock();
					} else {
						_ReleaseSRWLockExclusive(&NtvRWLock);
					}
				#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
					pthread_rwlock_unlock(NtvRWLock);
				#endif
			}

		private:
			#if defined(_WIN32)
				struct _RTL_SRWLOCK NtvRWLock;
				std::mutex *StdMux;
			#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
				pthread_rwlock_t *NtvRWLock;
			#endif
	};
} /* ZK */

#endif // !_ZK_RWLOCK_HPP
