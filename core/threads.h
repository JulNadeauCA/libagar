/*	Public domain	*/

#ifndef _AGAR_CORE_THREADS_H_
#define _AGAR_CORE_THREADS_H_

#ifdef AG_THREADS

#include <agar/config/have_pthreads.h>
#ifdef HAVE_PTHREADS
# include <pthread.h>
#else
# error "AG_THREADS option requires POSIX threads"
#endif

#define AG_Mutex	pthread_mutex_t
#define AG_MutexAttr	pthread_mutexattr_t
#define AG_Thread	pthread_t
#define AG_Cond		pthread_cond_t
#define AG_ThreadKey	pthread_key_t

#ifdef _SGI_SOURCE
# define AG_MUTEX_INITIALIZER {{0}}
#else
# define AG_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#endif

/*
 * Use a specific nullability attribute for pthread_mutex_t since
 * it may or may not be a pointer depending on the platform.
 */
#if defined(__GNUC__) || defined(__CC65__)
# define _Nonnull_Mutex
# define _Nonnull_Cond
# define _Nonnull_Thread
# define _Nullable_Mutex
# define _Nullable_Cond
# define _Nullable_Thread
# define _Null_unspecified_Mutex
# define _Null_unspecified_Cond
# define _Null_unspecified_Thread
#else
# include <agar/core/threads_nullability.h>
#endif /* !(__GNUC__ || __CC65__) */

#include <agar/core/begin.h>
__BEGIN_DECLS
extern pthread_mutexattr_t agRecursiveMutexAttr;
extern AG_Thread           agEventThread;

#define AG_ThreadSelf()			pthread_self()
#define AG_ThreadEqual(t1,t2)		pthread_equal((t1),(t2))
#define AG_ThreadExit(p)		pthread_exit(p)
#define AG_ThreadKeyGet(k)		pthread_getspecific(k)
#define AG_ThreadSigMask(how,n,o)	pthread_sigmask((how),(n),(o))
#define AG_ThreadKill(thread,signo)	(void)pthread_kill((thread),(signo))
#define AG_MutexTryLock(m)		pthread_mutex_trylock(m)
#define AG_CondWait(cd,m)		pthread_cond_wait(cd,m)
#define AG_CondTimedWait(cd,m,t)	pthread_cond_timedwait(cd,m,t)

/*
 * Inlinables
 */
void ag_thread_create(_Nonnull_Thread AG_Thread *_Nonnull,
                      void *_Nullable (*_Nonnull)(void *_Nullable),
		      void *_Nullable);
int  ag_thread_try_create(_Nonnull_Thread AG_Thread *_Nonnull,
                          void *_Nullable (*_Nonnull)(void *_Nullable),
			  void *_Nullable);
void ag_thread_cancel(_Nonnull_Thread AG_Thread);
int  ag_thread_try_cancel(_Nonnull_Thread AG_Thread);
void ag_thread_join(_Nonnull_Thread AG_Thread, void *_Nullable *_Nonnull);
int  ag_thread_try_join(AG_Thread, void *_Nullable *_Nonnull);
void ag_mutex_init(_Nonnull_Mutex AG_Mutex *_Nonnull);
void ag_mutex_init_recursive(_Nonnull_Mutex AG_Mutex *_Nonnull);
int  ag_mutex_try_init(_Nonnull_Mutex AG_Mutex *_Nonnull);
int  ag_mutex_try_init_recursive(_Nonnull_Mutex AG_Mutex *_Nonnull);
void ag_mutex_lock(_Nonnull_Mutex AG_Mutex *_Nonnull);
void ag_mutex_unlock(_Nonnull_Mutex AG_Mutex *_Nonnull);
void ag_mutex_destroy(_Nonnull_Mutex AG_Mutex *_Nonnull);
void ag_cond_init(_Nonnull_Cond AG_Cond *_Nonnull);
int  ag_cond_try_init(_Nonnull_Cond AG_Cond *_Nonnull);
void ag_cond_destroy(_Nonnull_Cond AG_Cond *_Nonnull);
void ag_cond_broadcast(_Nonnull_Cond AG_Cond *_Nonnull);
void ag_cond_signal(_Nonnull_Cond AG_Cond *_Nonnull);
void ag_thread_key_create(AG_ThreadKey *_Nonnull, void (*_Nullable)(void *_Nullable));
int  ag_thread_key_try_create(AG_ThreadKey *_Nonnull, void (*_Nullable)(void *_Nullable));
void ag_thread_key_delete(AG_ThreadKey);
int  ag_thread_key_try_delete(AG_ThreadKey);
void ag_thread_key_set(AG_ThreadKey, const void *_Nullable);
int  ag_thread_key_try_set(AG_ThreadKey, const void *_Nullable);

#ifdef AG_INLINE_THREADS
# define AG_INLINE_HEADER
# include <agar/core/inline_threads.h>
#else
# define AG_ThreadCreate(th,fn,a)     ag_thread_create((th),(fn),(a))
# define AG_ThreadTryCreate(th,fn,a)  ag_thread_try_create((th),(fn),(a))
# define AG_ThreadCancel(th)          ag_thread_cancel(th)
# define AG_ThreadTryCancel(th)       ag_thread_try_cancel(th)
# define AG_ThreadJoin(th,p)          ag_thread_join((th),(p))
# define AG_ThreadTryJoin(th,p)       ag_thread_try_join((th),(p))
# define AG_MutexInit(m)              ag_mutex_init(m)
# define AG_MutexInitRecursive(m)     ag_mutex_init_recursive(m)
# define AG_MutexTryInit(m)           ag_mutex_try_init(m)
# define AG_MutexTryInitRecursive(m)  ag_mutex_try_init_recursive(m)
# define AG_MutexLock(m)              ag_mutex_lock(m)
# define AG_MutexUnlock(m)            ag_mutex_unlock(m)
# define AG_MutexDestroy(m)           ag_mutex_destroy(m)
# define AG_CondInit(cd)              ag_cond_init(cd)
# define AG_CondTryInit(cd)           ag_cond_try_init(cd)
# define AG_CondDestroy(cd)           ag_cond_destroy(cd)
# define AG_CondBroadcast(cd)         ag_cond_broadcast(cd)
# define AG_CondSignal(cd)            ag_cond_signal(cd)
# define AG_ThreadKeyCreate(k,dfn)    ag_thread_key_create((k),(dfn))
# define AG_ThreadKeyTryCreate(k,dfn) ag_thread_key_try_create((k),(dfn))
# define AG_ThreadKeyDelete(k)        ag_thread_key_delete(k)
# define AG_ThreadKeyTryDelete(k)     ag_thread_key_try_delete(k)
# define AG_ThreadKeySet(k,p)         ag_thread_key_set((k),(p))
# define AG_ThreadKeyTrySet(k,p)      ag_thread_key_try_set((k),(p))
#endif

__END_DECLS
#include <agar/core/close.h>

#else /* !AG_THREADS */

# define _Nonnull_Mutex
# define _Nonnull_Cond
# define _Nonnull_Thread
# define _Nullable_Mutex
# define _Nullable_Cond
# define _Nullable_Thread
# define _Null_unspecified_Mutex
# define _Null_unspecified_Cond
# define _Null_unspecified_Thread
# define AG_MUTEX_INITIALIZER 0
# define AG_COND_INITIALIZER 0
# define AG_Mutex void *
# define AG_Thread void *
# define AG_Cond void *
# define AG_MutexAttr void *
# define AG_ThreadKey int
# define AG_MutexInit(m)
# define AG_MutexInitRecursive(m)
# define AG_MutexDestroy(m)
# define AG_MutexLock(m)
# define AG_MutexUnlock(m)
# define AG_CondInit(cd)
# define AG_CondDestroy(cd)
# define AG_CondBroadcast(cd)
# define AG_CondSignal(cd)
# define AG_CondWait(cd,m)
# define AG_CondTimedWait(cd,m,t)
# define AG_MutexTryInit(m) (0)
# define AG_MutexTryInitRecursive(m) (0)
# define AG_MutexTryLock(m) (0)
#endif /* !AG_THREADS */

#endif /* _AGAR_CORE_THREADS_H_ */
