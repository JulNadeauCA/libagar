/*	$Csoft: debug.h,v 1.9 2002/05/06 02:16:47 vedge Exp $	*/

#ifndef _AGAR_ENGINE_DEBUG_H_
#define _AGAR_ENGINE_DEBUG_H_

#include <errno.h>

/*
 * Generic debugging
 */
#ifdef DEBUG
extern int engine_debug;

# ifdef __GNUC__
#  define dprintf(fmt, args...)						\
	do {								\
		if (engine_debug)					\
			printf("%s: " fmt, __FUNCTION__ , ##args);	\
	} while (/*CONSTCOND*/0)
# else /* !__GNUC__ */
#  define dprintf	printf
# endif /* __GNUC__ */
# define dperror(s)		\
	do {			\
		perror(s);	\
		abort();	\
	} while (/*CONSTCOND*/0)
# define dprintrect(name, rect)					\
	dprintf("%s: %dx%d at [%d,%d]\n", (name),		\
	    (rect)->w, (rect)->h, (rect)->x, (rect)->y)

#else	/* !DEBUG */

# define dprintf(arg...)	((void)0)
# define dperror(s)		perror(s)
# define dprintrect(name, rect)	((void)0)

#endif	/* DEBUG */

#define dprintnode(m, x, y, str)					\
	printf("%s:[%d,%d]: %s\n", OBJECT((m))->name, (x), (y), (str))

/*
 * Lock debugging
 */

#ifdef LOCKDEBUG

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

extern pthread_key_t lockassert_key;	/* engine.c */

#define LOCKASSERT_NOOP		(void *)0x0
#define LOCKASSERT_CHECK	(void *)0x1
#define LOCKASSERT_FAILED	(void *)0x2

/* Mutexes */
#define pthread_mutex_lock(mutex)					\
	do {								\
		if (pthread_mutex_lock((mutex)) != 0) {			\
			if (pthread_getspecific(lockassert_key) ==	\
			    LOCKASSERT_CHECK) {				\
				pthread_setspecific(lockassert_key,	\
				    LOCKASSERT_FAILED);			\
			} else {					\
				fatal("mutex(%p): %s\n", mutex,		\
				    strerror(errno));			\
			}						\
		}							\
	} while (/*CONSTCOND*/0)
#define pthread_mutex_unlock(mutex) 					\
	do {								\
		if (pthread_mutex_unlock((mutex)) != 0) {		\
			fatal("mutex(%p): %s\n", mutex, strerror(errno)); \
		}							\
	} while (/*CONSTCOND*/0)
#if 0
#define pthread_mutex_assert(mutex)					\
	do {								\
		pthread_setspecific(lockassert_key, LOCKASSERT_CHECK);	\
		pthread_mutex_lock((mutex));				\
		if (pthread_getspecific(lockassert_key) !=		\
		    LOCKASSERT_FAILED) {				\
			fatal("pthread_mutex_assert: %p is not held\n",	\
			    (mutex));					\
		}							\
		pthread_setspecific(lockassert_key, LOCKASSERT_NOOP);	\
	} while (/*CONSTCOND*/0)
#else
#define pthread_mutex_assert(mutex) ((void)(0))
#endif
#define pthread_mutex_init(mutex, attr)	do {				\
		if (pthread_mutex_init((mutex), (attr)) != 0) {		\
			fatal("pthread_mutex_init: %s\n",		\
			    strerror(errno));				\
		}							\
	} while (/*CONSTCOND*/0)
#define pthread_mutex_destroy(mutex) do {				\
		if (pthread_mutex_destroy((mutex)) != 0) {		\
			fatal("pthread_mutex_destroy: %s\n",		\
			    strerror(errno));				\
		}							\
	} while (/*CONSTCOND*/0)

/* Read/write locks */

#define pthread_rwlock_init(rwlock, attr) do {				\
		if (pthread_rwlock_init((rwlock), (attr)) != 0) {	\
			fatal("pthread_rwlock_init: %s\n",		\
			    strerror(errno));				\
		}							\
	} while (/*CONSTCOND*/0)
#define pthread_rwlock_destroy(rwlock) do {				\
		if (pthread_rwlock_destroy((rwlock)) != 0) {		\
			fatal("pthread_rwlock_destroy: %s\n",		\
			    strerror(errno));				\
		}							\
	} while (/*CONSTCOND*/0)

/* Threads */

#define pthread_create(thread, attr, func, arg) do {			\
		if (pthread_create((thread), (attr), (func), (arg))	\
		    != 0) {						\
			fatal("pthread_create: %s\n", strerror(errno));	\
		}							\
	} while (/*CONSTCOND*/0)

#define pthread_join(thread, valptr) do {				\
		if (pthread_join((thread), (valptr)) != 0) {		\
			fatal("pthread_join: %s\n", strerror(errno));	\
		}							\
	} while (/*CONSTCOND*/0)

#else	/* !LOCKDEBUG */

#define pthread_mutex_assert(mutex)	((void)0)

#endif	/* LOCKDEBUG */

/*
 * Generic fatal() and warning() macros
 */

#ifdef __GNUC__
# define warning(fmt, args...) \
	printf("%s: " fmt, __FUNCTION__ , ##args)
# define fatal(fmt, args...)					\
	do {							\
		printf("%s: " fmt, __FUNCTION__ , ##args);	\
		abort();					\
	} while (/*CONSTCOND*/0)
#else
# define warning	printf
# define fatal		printf
#endif

#endif	/* _AGAR_ENGINE_DEBUG_H_ */
