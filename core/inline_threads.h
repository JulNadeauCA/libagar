/*
 * Threads interface
 */

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ThreadCreate(_Nonnull_Thread AG_Thread *_Nonnull th,
    void *_Nullable (*_Nonnull fn)(void *_Nullable), void *_Nullable arg)
#else
void
ag_thread_create(AG_Thread *th, void *(*fn)(void *), void *arg)
#endif
{
	int rv;

	if ((rv = pthread_create(th, NULL, fn, arg)) != 0)
		AG_FatalError("pthread_create");
}

#ifdef AG_INLINE_HEADER
int
AG_ThreadTryCreate(_Nonnull_Thread AG_Thread *_Nonnull th,
    void *_Nullable (*_Nonnull fn)(void *_Nullable), void *_Nullable arg)
#else
int
ag_thread_try_create(AG_Thread *th, void *(*fn)(void *), void *arg)
#endif
{
	int rv;

	if ((rv = pthread_create(th, NULL, fn, arg)) != 0) {
		AG_SetError("%s", AG_Strerror(rv));
		return (-1);
	}
	return (0);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ThreadCancel(_Nonnull_Thread AG_Thread th)
#else
void
ag_thread_cancel(AG_Thread th)
#endif
{
	if (pthread_cancel(th) != 0)
		AG_FatalError("pthread_cancel");
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_ThreadTryCancel(_Nonnull_Thread AG_Thread th)
#else
int
ag_thread_try_cancel(AG_Thread th)
#endif
{
	int rv;
	if ((rv = pthread_cancel(th)) != 0) {
		AG_SetError("%s", AG_Strerror(rv));
		return (-1);
	}
	return (0);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ThreadJoin(_Nonnull_Thread AG_Thread th, void *_Nullable *_Nonnull p)
#else
void
ag_thread_join(AG_Thread th, void **p)
#endif
{
	if (pthread_join(th, p) != 0)
		AG_FatalError("pthread_join");
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_ThreadTryJoin(_Nonnull_Thread AG_Thread th, void *_Nullable *_Nonnull p)
#else
int
ag_thread_try_join(AG_Thread th, void **p)
#endif
{
	int rv;

	if ((rv = pthread_join(th, p)) != 0) {
		AG_SetError("%s", AG_Strerror(rv));
		return (-1);
	}
	return (0);
}

/*
 * Mutex interface
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_MutexInit(_Nonnull_Mutex AG_Mutex *_Nonnull m)
#else
void
ag_mutex_init(AG_Mutex *m)
#endif
{
	if (pthread_mutex_init(m, NULL) != 0)
		AG_FatalError("pthread_mutex_init");
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_MutexInitRecursive(_Nonnull_Mutex AG_Mutex *_Nonnull m)
#else
void
ag_mutex_init_recursive(AG_Mutex *m)
#endif
{
	if (pthread_mutex_init(m, &agRecursiveMutexAttr) != 0)
		AG_FatalError("pthread_mutex_init(recursive)");
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_MutexTryInit(_Nonnull_Mutex AG_Mutex *_Nonnull m)
#else
int
ag_mutex_try_init(AG_Mutex *m)
#endif
{
	int rv;

	if ((rv = pthread_mutex_init(m, NULL)) != 0) {
		AG_SetError("%s", AG_Strerror(rv));
		return (-1);
	}
	return (0);
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_MutexTryInitRecursive(_Nonnull_Mutex AG_Mutex *_Nonnull m)
#else
int
ag_mutex_try_init_recursive(AG_Mutex *m)
#endif
{
	int rv;

	if ((rv = pthread_mutex_init(m, &agRecursiveMutexAttr)) != 0) {
		AG_SetError("%s", AG_Strerror(rv));
		return (-1);
	}
	return (0);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_MutexLock(_Nonnull_Mutex AG_Mutex *_Nonnull m)
#else
void
ag_mutex_lock(AG_Mutex *m)
#endif
{
	if (pthread_mutex_lock(m) != 0)
		AG_FatalError("pthread_mutex_lock");
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_MutexUnlock(_Nonnull_Mutex AG_Mutex *_Nonnull m)
#else
void
ag_mutex_unlock(AG_Mutex *m)
#endif
{
	if (pthread_mutex_unlock(m) != 0)
		AG_FatalError("pthread_mutex_unlock");
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_MutexDestroy(_Nonnull_Mutex AG_Mutex *_Nonnull m)
#else
void
ag_mutex_destroy(AG_Mutex *m)
#endif
{
	if (pthread_mutex_destroy(m) != 0)
		AG_FatalError("pthread_mutex_destroy");
}

/*
 * Condition variable interface
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_CondInit(_Nonnull_Cond AG_Cond *_Nonnull cd)
#else
void
ag_cond_init(AG_Cond *cd)
#endif
{
	if (pthread_cond_init(cd, NULL) != 0)
		AG_FatalError("pthread_cond_init");
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_CondTryInit(_Nonnull_Cond AG_Cond *_Nonnull cd)
#else
int
ag_cond_try_init(AG_Cond *cd)
#endif
{
	int rv;

	if ((rv = pthread_cond_init(cd, NULL)) != 0) {
		AG_SetError("%s", AG_Strerror(rv));
		return (-1);
	}
	return (0);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_CondDestroy(_Nonnull_Cond AG_Cond *_Nonnull cd)
#else
void
ag_cond_destroy(AG_Cond *cd)
#endif
{
	if (pthread_cond_destroy(cd) != 0)
		AG_FatalError("pthread_cond_destroy");
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_CondBroadcast(_Nonnull_Cond AG_Cond *_Nonnull cd)
#else
void
ag_cond_broadcast(AG_Cond *cd)
#endif
{
	if (pthread_cond_broadcast(cd) != 0)
		AG_FatalError("pthread_cond_broadcast");
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_CondSignal(_Nonnull_Cond AG_Cond *_Nonnull cd)
#else
void
ag_cond_signal(AG_Cond *cd)
#endif
{
	if (pthread_cond_signal(cd) != 0)
		AG_FatalError("pthread_cond_signal");
}

/*
 * Thread-local storage interface
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ThreadKeyCreate(AG_ThreadKey *_Nonnull k,
    void (*_Nullable destructorFn)(void *_Nullable))
#else
void
ag_thread_key_create(AG_ThreadKey *k, void (*destructorFn)(void *))
#endif
{
	if (pthread_key_create(k,destructorFn) != 0)
		AG_FatalError("pthread_key_create");
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_ThreadKeyTryCreate(AG_ThreadKey *_Nonnull k,
    void (*_Nullable destructorFn)(void *_Nullable))
#else
int
ag_thread_key_try_create(AG_ThreadKey *k, void (*destructorFn)(void *))
#endif
{
	int rv;

	if ((rv = pthread_key_create(k,destructorFn)) != 0) {
		AG_SetError("%s", AG_Strerror(rv));
		return (-1);
	}
	return (0);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ThreadKeyDelete(AG_ThreadKey k)
#else
void
ag_thread_key_delete(AG_ThreadKey k)
#endif
{
	if (pthread_key_delete(k) != 0)
		AG_FatalError("pthread_key_delete");
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_ThreadKeyTryDelete(AG_ThreadKey k)
#else
int
ag_thread_key_try_delete(AG_ThreadKey k)
#endif
{
	int rv;

	if ((rv = pthread_key_delete(k)) != 0) {
		AG_SetError("%s", AG_Strerror(rv));
		return (-1);
	}
	return (0);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ThreadKeySet(AG_ThreadKey k, const void *_Nullable p)
#else
void
ag_thread_key_set(AG_ThreadKey k, const void *p)
#endif
{
	if (pthread_setspecific(k, p) != 0)
		AG_FatalError("pthread_setspecific");
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_ThreadKeyTrySet(AG_ThreadKey k, const void *_Nullable p)
#else
int
ag_thread_key_try_set(AG_ThreadKey k, const void *p)
#endif
{
	int rv;

	if ((rv = pthread_setspecific(k, p)) != 0) {
		AG_SetError("%s", AG_Strerror(rv));
		return (-1);
	}
	return (0);
}
