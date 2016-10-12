#ifndef _MUTEX_H
#define _MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mutex *Mutex;
Mutex new_Mutex(void);
void Mutex_lock(Mutex);
void Mutex_unlock(Mutex);
void Mutex_free(Mutex);

#ifdef __cplusplus
}
#endif

#endif // !_MUTEX_H
