/*	$Csoft: debug.h,v 1.3 2002/02/03 11:08:08 vedge Exp $	*/

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

#else /* !DEBUG */

# ifdef __GNUC__
#  define dprintf(arg...)	((void)0)
#  define fatal(fmt, args...)	printf("%s: " fmt, __FUNCTION__ , ##args)
# else
#  define dprintf		((void)0)
#  define fatal			printf
# endif /* __GNUC__ */

#endif /* DEBUG */

#ifdef __GNUC__
#define warning(fmt, args...)	printf("%s: " fmt, __FUNCTION__ , ##args)
#else
#define warning			printf
#endif

