/*	$Csoft: debug.h,v 1.20 2002/11/13 01:13:03 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ENGINE_DEBUG_H_
#define _AGAR_ENGINE_DEBUG_H_

void	_dprintf(const char *, ...);
void	_debug(int, const char *, ...);
void	_debug_n(int, const char *, ...);
void	_dprintf_noop(const char *, ...);
void	_debug_noop(int, const char *, ...);
void	_debug_n_noop(int, const char *, ...);

#ifdef DEBUG
extern int engine_debug;
# ifdef __GNUC__
#  define dprintf(fmt, args...)						\
	do {								\
		if (engine_debug)					\
			printf("%s: " fmt, __FUNCTION__ , ##args);	\
	} while (0)
#  define deprintf(fmt, args...)					\
	do {								\
		if (engine_debug)					\
			fprintf(stderr, fmt, ##args);			\
	} while (0)
#  define debug(mask, fmt, args...)					\
	do {								\
		if (engine_debug & (mask))				\
			printf("%s: " fmt , __FUNCTION__ , ##args);	\
	} while (0)
#  define debug_n(mask, fmt, args...)					\
	do {								\
		if (engine_debug & (mask))				\
			fprintf(stderr, fmt, ##args);			\
	} while (0)
# else	/* !__GNUC__ */
#  define dprintf	_dprintf
#  define deprintf	_dprintf
#  define debug		_debug
#  define debug_n	_debug_n
# endif	/* __GNUC__ */
#else	/* !DEBUG */
# if defined(__GNUC__)
#  define dprintf(arg...)		((void)0)
#  define deprintf(arg...)		((void)0)
#  define debug(level, arg...)		((void)0)
#  define debug_n(level, arg...)	((void)0)
# else
#  define dprintf	_dprintf_noop
#  define deprintf	_dprintf_noop
#  define debug		_debug_noop
#  define debug_n	_debug_n_noop
# endif
#endif	/* DEBUG */

#if defined(LOCKDEBUG) && defined(SERIALIZATION)
#include <stdlib.h>
#include <string.h>

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

#endif	/* LOCKDEBUG and SERIALIZATION */

#ifdef SERIALIZATION
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
#endif

#endif	/* !_AGAR_ENGINE_DEBUG_H_ */
