/*	$Csoft: error.h,v 1.18 2003/04/14 08:56:20 vedge Exp $	*/
/*	Public domain	*/

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUC__
# define warning(fmt, args...) \
	printf("%s: " fmt, __FUNCTION__ , ##args)
# define fatal(fmt, args...)						\
	do {								\
		fprintf(stderr, "%s: " fmt, __FUNCTION__ , ##args);	\
		fprintf(stderr, "\n");					\
		abort();						\
	} while (0)
#else
# define warning	printf
# define fatal		error_fatal
#endif

#define Free(p)	if ((p) != NULL) free((p))

#define Vasprintf(msg, fmt, args) do {				\
	va_start((args), (fmt));				\
	if (vasprintf((msg), (fmt), (args)) == -1) 		\
		fatal("vasprintf: %s", strerror(errno));	\
	va_end((args));						\
} while (0)

#include "begin_code.h"

__BEGIN_DECLS
extern DECLSPEC char		*Strdup(const char *);
extern DECLSPEC void		*Malloc(size_t);
extern DECLSPEC void		*Realloc(void *, size_t);
extern DECLSPEC ssize_t		 Write(int, const void *, size_t);
extern DECLSPEC ssize_t		 Read(int, void *, size_t);
extern DECLSPEC const char	*error_get(void);
extern DECLSPEC void		 error_set(const char *, ...);
extern DECLSPEC void		 error_fatal(const char *, ...);
extern DECLSPEC void		 Asprintf(char **, const char *, ...);
__END_DECLS

#include "close_code.h"
