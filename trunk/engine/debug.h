/*	$Csoft: debug.h,v 1.2 2002/01/26 06:37:18 vedge Exp $	*/

#ifdef DEBUG
extern int engine_debug;

# ifdef __GNUC__
#  define dprintf(fmt, args...)					\
	if (engine_debug)					\
		printf("%s: " fmt, __FUNCTION__ , ##args) 
#  define fatal(fmt, args...)					\
	do {							\
		printf("%s: " fmt, __FUNCTION__ , ##args);	\
		abort();					\
	} while (0)
# else /* !__GNUC__ */
#  define dprintf	printf
#  define fatal 	printf
# endif /* __GNUC__ */

#else /* !DEBUG */

# ifdef __GNUC__
#  define dprintf(arg...)		((void)0)
#  define fatal(fmt, args...)	printf("%s: " fmt, __FUNCTION__ , ##args)
# else
#  define dprintf			((void)0)
#  define fatal			printf
# endif /* __GNUC__ */

#endif /* DEBUG */

#ifdef __GNUC__
#define warning(fmt, args...)	printf("%s: " fmt, __FUNCTION__ , ##args)
#else
#define warning			printf
#endif

