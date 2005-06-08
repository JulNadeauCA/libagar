/*	$Csoft: error.h,v 1.10 2004/06/18 03:11:26 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ERROR_ERROR_H_
#define _AGAR_ERROR_ERROR_H_

#include <compat/queue.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <config/debug.h>
#include <config/threads.h>
#include <config/have_bounded_attribute.h>
#include <config/have_format_attribute.h>
#include <config/have_nonnull_attribute.h>

#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# if defined(__cplusplus)
#  define __BEGIN_DECLS	extern "C" {
#  define __END_DECLS	}
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif

#ifdef HAVE_BOUNDED_ATTRIBUTE
#define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
#else
#define BOUNDED_ATTRIBUTE(t, a, b)
#endif

#ifdef HAVE_FORMAT_ATTRIBUTE
#define FORMAT_ATTRIBUTE(t, a, b) __attribute__((__format__ (t,a,b)))
#else
#define FORMAT_ATTRIBUTE(t, a, b)
#endif

#ifdef HAVE_NONNULL_ATTRIBUTE
#define NONNULL_ATTRIBUTE(a) __attribute__((__nonnull__ (a)))
#else
#define NONNULL_ATTRIBUTE(a)
#endif

#ifdef __GNUC__
#define fatal(fmt, args...)						\
	do {								\
		fprintf(stderr, "%s: " fmt, __FUNCTION__ , ##args);	\
		fprintf(stderr, "\n");					\
		abort();						\
	} while (0)
#else
#define fatal AG_FatalError
#endif

#define Malloc(len, t)		AG_Malloc((len), (t))
#define Realloc(p, len)		AG_Realloc((p), (len))
#ifdef DEBUG
#define Free(p, t)		AG_Free((p), (t))
#else
#define Free(p, t)		if ((p) != NULL) free((p))
#endif
#define Strdup(s)		AG_Strdup(s)
#define Vasprintf(msg, fmt, args) do {				\
	va_start((args), (fmt));				\
	if (vasprintf((msg), (fmt), (args)) == -1) 		\
		fatal("vasprintf");				\
	va_end((args));						\
} while (0)

#include <engine/error/malloc.h>

#include "begin_code.h"

#ifdef DEBUG
extern int agDebugLvl;
#endif

__BEGIN_DECLS
void		 AG_InitError(void);
void		 AG_DestroyError(void);
__inline__ char	*AG_Strdup(const char *);
const char	*AG_GetError(void);
void		 AG_SetError(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_FatalError(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_DebugPrintf(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_DebugPrintfNop(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_Debug(int, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
void		 AG_DebugNop(int, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
void		 AG_DebugN(int, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
__END_DECLS

#include "close_code.h"

#ifdef DEBUG

#ifdef __GNUC__

# define dprintf(fmt, args...) \
	printf("%s: " fmt, __FUNCTION__ , ##args)

# define deprintf(fmt, args...) \
	fprintf(stderr, fmt, ##args)

# define debug(mask, fmt, args...)				\
	if (agDebugLvl & (mask)) 				\
		printf("%s: " fmt , __FUNCTION__ , ##args)

# define debug_n(mask, fmt, args...)				\
	if (agDebugLvl & (mask))				\
		fprintf(stderr, fmt, ##args)

#else
# define dprintf	AG_DebugPrintf
# define deprintf	AG_DebugPrintf
# define debug		AG_Debug
# define debug_n	AG_DebugN
#endif /* __GNUC__ */

#else /* !DEBUG */

#if defined(__GNUC__)
# define dprintf(arg...)	((void)0)
# define deprintf(arg...)	((void)0)
# define debug(level, arg...)	((void)0)
# define debug_n(level, arg...)	((void)0)
#else
# define dprintf	AG_DebugPrintfNop
# define deprintf	AG_DebugPrintfNop
# define debug		AG_DebugNop
# define debug_n	AG_DebugNop
#endif /* __GNUC__ */

#endif	/* DEBUG */

#ifdef THREADS
# ifdef DEBUG
#  define pthread_mutex_lock(mutex) \
	if (pthread_mutex_lock((mutex)) != 0) \
		abort()
#  define pthread_mutex_unlock(mutex) \
	if (pthread_mutex_unlock((mutex)) != 0) \
		abort()
#  define pthread_mutex_init(mutex, attr) \
	if (pthread_mutex_init((mutex), (attr)) != 0) \
		abort()
#  define pthread_mutex_destroy(mutex) \
	if (pthread_mutex_destroy((mutex)) != 0) \
		abort()
# endif /* DEBUG */
# define Pthread_create(thread, attr, func, arg) \
	if (pthread_create((thread), (attr), (func), (arg)) != 0) \
		abort()
# define Pthread_join(thread, valptr) \
	if (pthread_join((thread), (valptr)) != 0) \
		abort()
#endif /* THREADS */

#endif /* _AGAR_ERROR_ERROR_H_ */
