/*	Public domain	*/

#ifndef _AGAR_CORE_THREADS_H_
#define _AGAR_CORE_THREADS_H_

#ifdef AG_THREADS

#include <agar/config/have_pthreads.h>
#ifdef HAVE_PTHREADS
#include <pthread.h>
#include <signal.h>
#else
# error "AG_THREADS option requires POSIX threads"
#endif

typedef pthread_mutex_t AG_Mutex;
typedef pthread_mutexattr_t AG_MutexAttr;
typedef pthread_t AG_Thread;
typedef pthread_cond_t AG_Cond;
typedef pthread_key_t AG_ThreadKey;

#ifdef _SGI_SOURCE
#define AG_MUTEX_INITIALIZER {{0}}
#else
#define AG_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#endif

#include <agar/core/begin.h>
__BEGIN_DECLS
extern pthread_mutexattr_t agRecursiveMutexAttr;
__END_DECLS
#include <agar/core/close.h>

#define AG_ThreadSelf()			pthread_self()
#define AG_ThreadJoin(t,vp)		pthread_join((t),(vp))
#define AG_ThreadExit(p)		pthread_exit(p)
#define AG_ThreadKeyCreate(k)		pthread_key_create(k,NULL)
#define AG_ThreadKeyDelete(k)		pthread_key_delete(k)
#define AG_ThreadKeyGet(k)		pthread_getspecific(k)
#define AG_ThreadKeySet(k,v)		pthread_setspecific((k),(v))
#define AG_ThreadSigMask(how,n,o)	pthread_sigmask((how),(n),(o))
#define AG_ThreadKill(thread,signo)	pthread_kill((thread),(signo))
#define AG_MutexTryLock(m)		pthread_mutex_trylock(m)
#define AG_CondWait(cd,m)		pthread_cond_wait(cd,m)
#define AG_CondTimedWait(cd,m,t)	pthread_cond_timedwait(cd,m,t)

/*
 * Thread interface
 */
static __inline__ void
AG_ThreadCreate(AG_Thread *th, void *(*fn)(void *), void *arg)
{
	int rv;
	if ((rv = pthread_create(th, NULL, fn, arg)) != 0)
		AG_FatalError("AG_ThreadCreate (%d)", rv);
}
static __inline__ int
AG_ThreadTryCreate(AG_Thread *th, void *(*fn)(void *), void *arg)
{
	int rv;
	if ((rv = pthread_create(th, NULL, fn, arg)) != 0) {
		AG_SetError("pthread_create failed (%d)", rv);
		return (-1);
	}
	return (0);
}
static __inline__ void
AG_ThreadCancel(AG_Thread th)
{
	if (pthread_cancel(th) != 0)
		AG_FatalError("pthread_cancel failed");
}

/*
 * Mutex interface
 */
static __inline__ void
AG_MutexInit(AG_Mutex *m)
{
	if (pthread_mutex_init(m, NULL) != 0)
		AG_FatalError("AG_MutexInit");
}
static __inline__ void
AG_MutexInitRecursive(AG_Mutex *m)
{
	if (pthread_mutex_init(m, &agRecursiveMutexAttr) != 0)
		AG_FatalError("AG_MutexInitRecursive");
}
static __inline__ int
AG_MutexTryInit(AG_Mutex *m)
{
	int rv;
	if ((rv = pthread_mutex_init(m, NULL)) != 0) {
		AG_SetError("pthread_mutex_init failed (%d)", rv);
		return (-1);
	}
	return (0);
}
static __inline__ int
AG_MutexTryInitRecursive(AG_Mutex *m)
{
	int rv;
	if ((rv = pthread_mutex_init(m, &agRecursiveMutexAttr)) != 0) {
		AG_SetError("pthread_mutex_init failed (%d)", rv);
		return (-1);
	}
	return (0);
}
static __inline__ void
AG_MutexLock(AG_Mutex *m)
{
	if (pthread_mutex_lock(m) != 0)
		AG_FatalError("AG_MutexLock");
}
static __inline__ void
AG_MutexUnlock(AG_Mutex *m)
{
	if (pthread_mutex_unlock(m) != 0)
		AG_FatalError("AG_MutexUnlock");
}
static __inline__ void
AG_MutexDestroy(AG_Mutex *m)
{
	if (pthread_mutex_destroy(m) != 0)
		AG_FatalError("AG_MutexDestroy");
}

/*
 * Condition variable interface
 */
static __inline__ void
AG_CondInit(AG_Cond *cd)
{
	if (pthread_cond_init(cd, NULL) != 0)
		AG_FatalError("AG_CondInit");
}
static __inline__ int
AG_CondTryInit(AG_Cond *cd)
{
	int rv;
	if ((rv = pthread_cond_init(cd, NULL)) != 0) {
		AG_SetError("pthread_cond_init failed (%d)", rv);
		return (-1);
	}
	return (0);
}
static __inline__ void
AG_CondDestroy(AG_Cond *cd)
{
	if (pthread_cond_destroy(cd) != 0)
		AG_FatalError("AG_CondDestroy");
}
static __inline__ void
AG_CondBroadcast(AG_Cond *cd)
{
	if (pthread_cond_broadcast(cd) != 0)
		AG_FatalError("AG_CondBroadcast");
}
static __inline__ void
AG_CondSignal(AG_Cond *cd)
{
	if (pthread_cond_signal(cd) != 0)
		AG_FatalError("AG_CondSignal");
}

#else /* !AG_THREADS */

typedef int AG_Mutex;
typedef int AG_Thread;
typedef int AG_Cond;
typedef int AG_MutexAttr;
typedef int AG_ThreadKey;

#define AG_MUTEX_INITIALIZER 0
#define AG_COND_INITIALIZER 0

#define AG_MutexInit(m)
#define AG_MutexInitRecursive(m)
#define AG_MutexDestroy(m)
#define AG_MutexLock(m)
#define AG_MutexUnlock(m)
#define AG_CondInit(cd)
#define AG_CondDestroy(cd)
#define AG_CondBroadcast(cd)
#define AG_CondSignal(cd)
#define AG_CondWait(cd,m)
#define AG_CondTimedWait(cd,m,t)
#define AG_ThreadCancel(thread)
#define AG_ThreadSelf(thread) AG_FatalError("No AG_THREADS")
#define AG_ThreadCreate(thread,func,arg) AG_FatalError("No AG_THREADS")
#define AG_ThreadJoin(thread,valptr) AG_FatalError("No AG_THREADS")
#define AG_ThreadExit(p)
#define AG_ThreadKeyCreate(k)
#define AG_ThreadKeyDelete(k)
#define AG_ThreadKeyGet(k)
#define AG_ThreadKeySet(k,v)
#define AG_ThreadSigMask(how,newmask,oldmask)
#define AG_ThreadKill(thread,signo)

static __inline__ int AG_MutexTryInit(AG_Mutex *mutex) { return (0); }
static __inline__ int AG_MutexTryInitRecursive(AG_Mutex *mutex) { return (0); }
static __inline__ int AG_MutexTryLock(AG_Mutex *mutex) { return (0); }

#undef HAVE_PTHREADS
#endif /* AG_THREADS */

#endif /* _AGAR_CORE_THREADS_H_ */
