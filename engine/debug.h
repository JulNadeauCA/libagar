/*	$Csoft: debug.h,v 1.5 2002/02/28 12:50:14 vedge Exp $	*/

#ifdef DEBUG
extern int engine_debug;

# ifdef __GNUC__
#  define dprintf(fmt, args...)						\
	do {								\
		if (engine_debug)					\
			printf("%s: " fmt, __FUNCTION__ , ##args);	\
	} while (0)
#  define fatal(fmt, args...)					\
	do {							\
		printf("%s: " fmt, __FUNCTION__ , ##args);	\
		abort();					\
	} while (0)
# else /* !__GNUC__ */
#  define dprintf	printf
#  define fatal 	printf
# endif /* __GNUC__ */

# define dperror(s)		\
	do {			\
		perror(s);	\
		abort();	\
	} while (0)

#else /* !DEBUG */

# ifdef __GNUC__
#  define dprintf(arg...)	((void)0)
#  define fatal(fmt, args...)	printf("%s: " fmt, __FUNCTION__ , ##args)
# else
#  define dprintf		((void)0)
#  define fatal			printf
# endif /* __GNUC__ */

# define dperror(s)	perror(s)

#endif /* DEBUG */

#ifdef __GNUC__
#define warning(fmt, args...)	printf("%s: " fmt, __FUNCTION__ , ##args)
#else
#define warning			printf
#endif

#ifdef LOCKDEBUG

#include <stdlib.h>

#define pthread_mutex_lock(mutex)					\
	do {								\
		if (pthread_mutex_lock((mutex)) != 0) {			\
			abort();					\
		}							\
	} while (0)
#define pthread_mutex_unlock(mutex) 					\
	do {								\
		if (pthread_mutex_unlock((mutex)) != 0) {		\
			abort();					\
		}							\
	} while (/*CONSTCOND*/0)
#endif

