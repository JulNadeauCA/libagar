/*	$Csoft: debug.h,v 1.14 2002/06/09 10:08:04 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ENGINE_DEBUG_H_
#define _AGAR_ENGINE_DEBUG_H_

#include <errno.h>

#ifdef DEBUG
extern int engine_debug;
# ifdef __GNUC__
#  define dprintf(fmt, args...)						\
	do {								\
		if (engine_debug)					\
			printf("%s: " fmt, __FUNCTION__ , ##args);	\
	} while (/*CONSTCOND*/0)
# else	/* !__GNUC__ */
#  define dprintf	printf
# endif	/* __GNUC__ */
# define dprintrect(name, rect)						\
	dprintf("%s: %dx%d at [%d,%d]\n", (name),			\
	    (rect)->w, (rect)->h, (rect)->x, (rect)->y)
# define dprintnode(m, x, y, str)					\
	printf("%s:[%d,%d]: %s\n", OBJECT((m))->name, (x), (y), (str))
#else	/* !DEBUG */
# if defined(__GNUC__)
#  define dprintf(arg...) ((void)0)
# else
#  define dprintf	  printf
# endif
# define dprintrect(name, rect)
# define dprintnode(m, x, y, str)
#endif	/* DEBUG */

#ifdef LOCKDEBUG
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define pthread_mutex_lock(mutex) do {					\
	int _rv;							\
	if ((_rv = pthread_mutex_lock((mutex))) != 0) {			\
		fatal("mutex_lock(%p): %s\n", (mutex), strerror(_rv));	\
	}								\
} while (/*CONSTCOND*/0)
#define pthread_mutex_unlock(mutex) do {				\
	int _rv;							\
	if ((_rv = pthread_mutex_unlock((mutex))) != 0) {		\
		fatal("mutex_unlock(%p): %s\n", (mutex), strerror(_rv));\
	}								\
} while (/*CONSTCOND*/0)
#define pthread_mutex_init(mutex, attr)	do {				\
	int _rv;							\
	if ((_rv = pthread_mutex_init((mutex), (attr))) != 0) {		\
		fatal("pthread_mutex_init: %s\n", strerror(_rv));	\
	}								\
} while (/*CONSTCOND*/0)
#define pthread_mutex_destroy(mutex) do {				\
	int _rv;							\
	if ((_rv = pthread_mutex_destroy((mutex))) != 0) {		\
		fatal("pthread_mutex_destroy: %s\n", strerror(_rv));	\
	}								\
} while (/*CONSTCOND*/0)

#define sem_init(wsem, pshared, value) do {				\
	int _rv;							\
	if ((_rv = sem_init((wsem), (pshared), (value))) != 0) {	\
		fatal("sem_init(%p): %s\n", strerror(_rv));		\
	}								\
} while (/*CONSTCOND*/0)
#define sem_destroy(wsem) do {						\
	int _rv;							\
	if ((_rv = sem_destroy((wsem))) != 0) {				\
		fatal("sem_destroy(%p): %s\n", (wsem), strerror(_rv));	\
	}								\
} while (/*CONSTCOND*/0)
#define sem_wait(wsem) do {						\
	int _rv;							\
	if ((_rv = sem_wait((wsem))) != 0) {				\
		fatal("sem_wait(%p): %s\n", (wsem), strerror(_rv));	\
	}								\
} while (/*CONSTCOND*/0)
#define sem_post(wsem) do {						\
	int _rv;							\
	if ((_rv = sem_post((wsem))) != 0) {				\
		fatal("sem_post(%p): %s\n", (wsem), strerror(_rv));	\
	}								\
} while (/*CONSTCOND*/0)

#define pthread_create(thread, attr, func, arg) do {			\
	int _rv;							\
	if ((_rv = pthread_create((thread), (attr), (func), (arg)))	\
	    != 0) {							\
		fatal("pthread_create: %s\n", strerror(_rv));		\
	}								\
} while (/*CONSTCOND*/0)
#define pthread_join(thread, valptr) do {				\
	int _rv;							\
	if ((_rv = pthread_join((thread), (valptr))) != 0) {		\
		fatal("pthread_join: %s\n", strerror(_rv));		\
	}								\
} while (/*CONSTCOND*/0)

#endif	/* LOCKDEBUG */
#endif	/* _AGAR_ENGINE_DEBUG_H_ */
