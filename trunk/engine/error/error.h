/*	$Csoft: error.h,v 1.2 2003/06/21 02:52:43 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ENGINE_ERROR_ERROR_H_
#define _AGAR_ENGINE_ERROR_ERROR_H_

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <config/debug.h>
#include <config/lockdebug.h>
#include <config/threads.h>

#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# if defined(__cplusplus)
#  define __BEGIN_DECLS	extern "C" {
#  define __END_DECLS	}
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif

#ifdef __GNUC__
#define fatal(fmt, args...)						\
	do {								\
		fprintf(stderr, "%s: " fmt, __FUNCTION__ , ##args);	\
		fprintf(stderr, "\n");					\
		abort();						\
	} while (0)
#else
#define fatal error_fatal
#endif

#define Malloc(len)	error_malloc(len)
#define Realloc(p, len)	error_realloc((p), (len))
#define Free(p)		if ((p) != NULL) free((p))
#define Strdup(s)	error_strdup(s)
#define Vasprintf(msg, fmt, args) do {				\
	va_start((args), (fmt));				\
	if (vasprintf((msg), (fmt), (args)) == -1) 		\
		fatal("vasprintf");				\
	va_end((args));						\
} while (0)

#include "begin_code.h"

#ifdef DEBUG
extern int engine_debug;
#endif

__BEGIN_DECLS
void		 error_init(void);
void		 error_destroy(void);

__inline__ void	*error_malloc(size_t);
__inline__ void	*error_realloc(void *, size_t);
__inline__ char	*error_strdup(const char *);
const char	*error_get(void);
void		 error_set(const char *, ...);
void		 error_fatal(const char *, ...);
void		 Asprintf(char **, const char *, ...);
void		 error_dprintf(const char *, ...);
void		 error_dprintf_nop(const char *, ...);
void		 error_debug(int, const char *, ...);
void		 error_debug_nop(int, const char *, ...);
void		 error_debug_n(int, const char *, ...);
__END_DECLS

#include "close_code.h"

#ifdef DEBUG

#ifdef __GNUC__

# define dprintf(fmt, args...) \
	printf("%s: " fmt, __FUNCTION__ , ##args)

# define deprintf(fmt, args...) \
	fprintf(stderr, fmt, ##args)

# define debug(mask, fmt, args...)				\
	if (engine_debug & (mask)) 				\
		printf("%s: " fmt , __FUNCTION__ , ##args)

# define debug_n(mask, fmt, args...)				\
	if (engine_debug & (mask))				\
		fprintf(stderr, fmt, ##args)

#else
# define dprintf	error_dprintf
# define deprintf	error_dprintf
# define debug		error_debug
# define debug_n	error_debug_n
#endif /* __GNUC__ */

#else /* !DEBUG */

#if defined(__GNUC__)
# define dprintf(arg...)	((void)0)
# define deprintf(arg...)	((void)0)
# define debug(level, arg...)	((void)0)
# define debug_n(level, arg...)	((void)0)
#else
# define dprintf	error_dprintf_nop
# define deprintf	error_dprintf_nop
# define debug		error_debug_nop
# define debug_n	error_debug_nop
#endif /* __GNUC__ */

#endif	/* DEBUG */

#ifdef THREADS
# ifdef LOCKDEBUG
#  define pthread_mutex_lock(mutex) \
	if (pthread_mutex_lock(mutex) != 0) \
		fatal("lock")
#  define pthread_mutex_unlock(mutex) \
	if (pthread_mutex_unlock(mutex) != 0) \
		fatal("unlock")
#  define pthread_mutex_init(mutex, attr) \
	if (pthread_mutex_init((mutex), (attr)) != 0) \
		fatal("mutex init")
#  define pthread_mutex_destroy(mutex) do {			\
	int _rv;						\
	if ((_rv = pthread_mutex_destroy(mutex)) != 0)		\
		fatal("mutex destroy: %s", strerror(_rv));	\
 } while (0)
# endif /* LOCKDEBUG */
# define Pthread_create(thread, attr, func, arg) \
	if (pthread_create((thread), (attr), (func), (arg)) != 0) \
		fatal("thread create")
# define Pthread_join(thread, valptr) \
	if (pthread_join((thread), (valptr)) != 0) \
		fatal("thread join")
#endif /* THREADS */

#endif /* _AGAR_ENGINE_ERROR_ERROR_H_ */
