/*	$Csoft: debug.h,v 1.19 2002/11/12 02:30:51 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ENGINE_DEBUG_H_
#define _AGAR_ENGINE_DEBUG_H_

#ifdef DEBUG
extern int engine_debug;
# ifdef __GNUC__
#  define dprintf(fmt, args...)						\
	do {								\
		if (engine_debug)					\
			printf("%s: " fmt, __FUNCTION__ , ##args);	\
	} while (/*CONSTCOND*/0)
#  define deprintf(fmt, args...)					\
	do {								\
		if (engine_debug)					\
			fprintf(stderr, fmt , ##args);			\
	} while (/*CONSTCOND*/0)
# else	/* !__GNUC__ */
#  define dprintf	printf
#  define deprintf	printf
# endif	/* __GNUC__ */
# define dprintrect(name, rect)						\
	dprintf("%s: %dx%d at [%d,%d]\n", (name),			\
	    (rect)->w, (rect)->h, (rect)->x, (rect)->y)
# define dprintnode(m, x, y, str)					\
	printf("%s:[%d,%d]: %s\n", OBJECT((m))->name, (x), (y), (str))
#else	/* !DEBUG */
# if defined(__GNUC__)
#  define dprintf(arg...) ((void)0)
#  define deprintf(arg...) ((void)0)
# else
#  define dprintf	  printf
#  define deprintf	  printf
# endif
# define dprintrect(name, rect)
# define dprintnode(m, x, y, str)
#endif	/* DEBUG */

#ifdef LOCKDEBUG
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#if 0
#include <semaphore.h>
#endif

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

#if 0
#define Sem_init(wsem, pshared, value) do {				\
	int _rv;							\
	if ((_rv = sem_init((wsem), (pshared), (value))) != 0) {	\
		fatal("sem_init(%p): %s\n", strerror(_rv));		\
	}								\
} while (/*CONSTCOND*/0)
#define Sem_destroy(wsem) do {						\
	int _rv;							\
	if ((_rv = sem_destroy((wsem))) != 0) {				\
		fatal("sem_destroy(%p): %s\n", (wsem), strerror(_rv));	\
	}								\
} while (/*CONSTCOND*/0)
#define Sem_wait(wsem) do {						\
	int _rv;							\
	if ((_rv = sem_wait((wsem))) != 0) {				\
		fatal("sem_wait(%p): %s\n", (wsem), strerror(_rv));	\
	}								\
} while (/*CONSTCOND*/0)
#define Sem_post(wsem) do {						\
	int _rv;							\
	if ((_rv = sem_post((wsem))) != 0) {				\
		fatal("sem_post(%p): %s\n", (wsem), strerror(_rv));	\
	}								\
} while (/*CONSTCOND*/0)
#endif

#endif	/* LOCKDEBUG */

#define Pthread_create(thread, attr, func, arg) do {			\
	int _rv;							\
	if ((_rv = pthread_create((thread), (attr), (func), (arg)))	\
	    != 0) {							\
		fatal("pthread_create: %s\n", strerror(_rv));		\
	}								\
} while (/*CONSTCOND*/0)
#define Pthread_join(thread, valptr) do {				\
	int _rv;							\
	if ((_rv = pthread_join((thread), (valptr))) != 0) {		\
		fatal("pthread_join: %s\n", strerror(_rv));		\
	}								\
} while (/*CONSTCOND*/0)

#endif	/* _AGAR_ENGINE_DEBUG_H_ */
