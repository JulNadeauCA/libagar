/*	$Csoft: debug.h,v 1.7 2002/03/13 07:37:33 vedge Exp $	*/

#ifdef DEBUG
extern int engine_debug;

# ifdef __GNUC__
#  define dprintf(fmt, args...)						\
	do {								\
		if (engine_debug)					\
			printf("%s: " fmt, __FUNCTION__ , ##args);	\
	} while (/*CONSTCOND*/0)
#  define fatal(fmt, args...)					\
	do {							\
		printf("%s: " fmt, __FUNCTION__ , ##args);	\
		abort();					\
	} while (/*CONSTCOND*/0)
# else /* !__GNUC__ */
#  define dprintf	printf
#  define fatal 	printf
# endif /* __GNUC__ */
# define dperror(s)		\
	do {			\
		perror(s);	\
		abort();	\
	} while (/*CONSTCOND*/0)
# define dprintrect(name, rect)					\
	dprintf("%s: %dx%d at [%d,%d]\n", (name),		\
	    (rect)->w, (rect)->h, (rect)->x, (rect)->y)

#else /* !DEBUG */

# define dprintf(arg...)	((void)0)
# ifdef __GNUC__
#  define fatal(fmt, args...)	printf("%s: " fmt, __FUNCTION__ , ##args)
# else
#  define fatal			printf
# endif /* __GNUC__ */

# define dperror(s)		perror(s)
# define dprintrect(name, rect)	((void)0)

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
			perror("mutex");				\
			abort();					\
		}							\
	} while (/*CONSTCOND*/0)
#define pthread_mutex_unlock(mutex) 					\
	do {								\
		if (pthread_mutex_unlock((mutex)) != 0) {		\
			perror("mutex");				\
			abort();					\
		}							\
	} while (/*CONSTCOND*/0)
#endif

