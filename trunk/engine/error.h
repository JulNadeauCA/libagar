/*	$Csoft: error.h,v 1.21 2003/06/17 23:30:42 vedge Exp $	*/
/*	Public domain	*/

#include <sys/types.h>

#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# if defined(__cplusplus)
#  define __BEGIN_DECLS	extern "C" {
#  define __END_DECLS	}
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUC__
# define fatal(fmt, args...)						\
	do {								\
		fprintf(stderr, "%s: " fmt, __FUNCTION__ , ##args);	\
		fprintf(stderr, "\n");					\
		abort();						\
	} while (0)
#else
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
char		*Strdup(const char *);
void		*Malloc(size_t);
void		*Realloc(void *, size_t);
ssize_t		 Write(int, const void *, size_t);
ssize_t		 Read(int, void *, size_t);
const char	*error_get(void);
void		 error_set(const char *, ...);
void		 error_fatal(const char *, ...);
void		 Asprintf(char **, const char *, ...);
__END_DECLS

#include "close_code.h"
