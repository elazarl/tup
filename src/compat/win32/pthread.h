#ifndef tup_compat_win32_pthread_h
#define tup_compat_win32_pthread_h

#include <windows.h>

typedef struct {
	void*		h;
	void*		data;
} pthread_t;

typedef CRITICAL_SECTION pthread_mutex_t;

typedef void pthread_attr_t;
typedef void pthread_mutexattr_t;

typedef void* (*pthread_start_t)(void*);

int pthread_create(
		pthread_t*		thread,
		const pthread_attr_t*	attr,
		pthread_start_t		func,
		void*			arg);

int pthread_join(
		pthread_t		thread,
		void**			retarg);

int pthread_mutex_init(
		pthread_mutex_t*	mutex,
		const pthread_mutexattr_t* attr);

int pthread_mutex_destroy(pthread_mutex_t *mutex);

#define pthread_mutex_lock(plock)	EnterCriticalSection(plock)
#define pthread_mutex_unlock(plock)	LeaveCriticalSection(plock)

/* Condition variable code is from:
 * http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
 */
typedef struct {
	HANDLE event;
} pthread_cond_t;

typedef void pthread_condattr_t;

int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

#endif
