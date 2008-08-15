/*	Public domain	*/

#ifndef _AGAR_CORE_THREADS_H_
#define _AGAR_CORE_THREADS_H_

#ifdef THREADS

#ifdef _AGAR_INTERNAL
#include <config/have_pthreads.h>
#else
#include <agar/config/have_pthreads.h>
#endif

#ifdef HAVE_PTHREADS
#include <pthread.h>
#include <signal.h>
#else
# error "THREADS option currently requires POSIX threads"
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

#if defined(__cplusplus)
extern "C" {
#endif

extern pthread_mutexattr_t agRecursiveMutexAttr;

#if defined(__cplusplus)
}
#endif

#ifdef DEBUG

# define AG_MutexInit(m) if (pthread_mutex_init((m),NULL)!=0) abort()
# define AG_MutexInitRecursive(m) if (pthread_mutex_init((m),&agRecursiveMutexAttr)!=0) abort()
# define AG_MutexDestroy(m) if (pthread_mutex_destroy(m)!=0) abort()
# define AG_MutexLock(m) if (pthread_mutex_lock(m)!=0) abort()
# define AG_MutexUnlock(m) if (pthread_mutex_unlock(m)!=0) abort()

# define AG_CondInit(cd) if (pthread_cond_init(cd,NULL)!=0) abort()
# define AG_CondDestroy(cd) if (pthread_cond_destroy(cd)!=0) abort()
# define AG_CondBroadcast(cd) if (pthread_cond_broadcast(cd)!=0) abort()
# define AG_CondSignal(cd) if (pthread_cond_signal(cd)!=0) abort()
# define AG_CondWait(cd,m) if (pthread_cond_wait(cd,m)!=0) abort()
# define AG_CondTimedWait(cd,m,t) if (pthread_cond_timedwait(cd,m,t)!=0) abort()

# define AG_ThreadCancel(t) if (pthread_cancel(t)!=0) abort();

#else /* !DEBUG */

# define AG_MutexInit(m) pthread_mutex_init((m),NULL)
# define AG_MutexInitRecursive(m) pthread_mutex_init((m),&agRecursiveMutexAttr)
# define AG_MutexDestroy(m) pthread_mutex_destroy(m)
# define AG_MutexLock(m) pthread_mutex_lock(m)
# define AG_MutexUnlock(m) pthread_mutex_unlock(m)

# define AG_CondInit(cd) pthread_cond_init(cd,NULL)
# define AG_CondDestroy(cd) pthread_cond_destroy(cd)
# define AG_CondBroadcast(cd) pthread_cond_broadcast(cd)
# define AG_CondSignal(cd) pthread_cond_signal(cd)
# define AG_CondWait(cd,m) pthread_cond_wait(cd,m)
# define AG_CondTimedWait(cd,m,t) pthread_cond_timedwait(cd,m,t)

# define AG_ThreadCancel(t) pthread_cancel(t)

#endif /* DEBUG */

#define AG_ThreadCreate(t,f,arg) pthread_create((t),NULL,(f),(arg))
#define AG_ThreadJoin(t,vp) pthread_join((t),(vp))
#define AG_ThreadExit(p) pthread_exit(p)
#define AG_MutexTrylock(m) pthread_mutex_trylock(m)
#define AG_ThreadKeyCreate(k) pthread_key_create(k,NULL)
#define AG_ThreadKeyDelete(k) pthread_key_delete(k)
#define AG_ThreadKeyGet(k) pthread_getspecific(k)
#define AG_ThreadKeySet(k,v) pthread_setspecific((k),(v))
#define AG_ThreadSigMask(how,n,o) pthread_sigmask((how),(n),(o))
#define AG_ThreadKill(thread,signo) pthread_kill((thread),(signo))

#else /* !THREADS */

typedef int AG_Mutex;
typedef int AG_Thread;
typedef int AG_Cond;

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

#define AG_ThreadCreate(thread,func,arg) AG_FatalError("No THREADS")
#define AG_ThreadJoin(thread,valptr) AG_FatalError("No THREADS")
#define AG_ThreadExit(p)
#define AG_MutexTrylock(m)
#define AG_ThreadKeyCreate(k)
#define AG_ThreadKeyDelete(k)
#define AG_ThreadKeyGet(k)
#define AG_ThreadKeySet(k,v)
#define AG_ThreadSigMask(how,newmask,oldmask)
#define AG_ThreadKill(thread,signo)

#undef HAVE_PTHREADS
#endif /* THREADS */

#endif /* _AGAR_CORE_THREADS_H_ */
