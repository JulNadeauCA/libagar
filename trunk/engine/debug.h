/*	$Csoft$	*/

#ifdef DEBUG
extern int engine_debug;
#define	dprintf(fmt, args...)					\
	if (engine_debug)					\
		printf("%s: " fmt, __FUNCTION__ , ##args) 
#define	dprintfa(fmt, args...)					\
	if (engine_debug)					\
		printf(fmt, ##args) 
#define	dprintfn(n, arg)					\
	if (engine_debug > (n))					\
n		printf("%s: " fmt, __FUNCTION__ , ##args) 

#define	fatal(fmt, args...)					\
	do {							\
		printf("%s: " fmt, __FUNCTION__ , ##args);	\
		abort();					\
	} while (0)

#else /* !DEBUG */

#define	dprintf(arg...)		((void)0)
#define	dprintfa(arg...)	((void)0)
#define dprintfn(n, arg...)	((void)0)
#define	fatal(fmt, args...)					\
	printf("%s: " fmt, __FUNCTION__ , ##args)
#endif /* DEBUG */

