/*	$Csoft: error.h,v 1.2 2002/06/01 02:39:25 vedge Exp $	*/
/*	Public domain	*/

#define AGAR_GetError()    ((char *)pthread_getspecific(engine_errorkey))
#define AGAR_SetError(msg) pthread_setspecific(engine_errorkey, (char *)(msg))

extern pthread_key_t engine_errorkey;	/* engine.c */

#ifdef __GNUC__
# define warning(fmt, args...) \
	printf("%s: " fmt, __FUNCTION__ , ##args)
# ifdef DEBUG
#  define fatal(fmt, args...)					\
	do {							\
		printf("%s: " fmt, __FUNCTION__ , ##args);	\
		abort();					\
	} while (/*CONSTCOND*/0)
# else
#  define fatal(fmt, args...)					\
	do {							\
		printf("%s: " fmt, __FUNCTION__ , ##args);	\
		engine_stop();					\
	} while (/*CONSTCOND*/0)
# endif
#else
# define warning	printf
# define fatal		printf
#endif

void		*emalloc(size_t);
void		*erealloc(void *, size_t);

