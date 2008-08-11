/*	Public domain	*/

#ifndef _AGAR_CORE_ERROR_H_
#define _AGAR_CORE_ERROR_H_

#ifdef _AGAR_INTERNAL
#include <config/free_null_is_a_noop.h>
#else
#include <agar/config/free_null_is_a_noop.h>
#endif

#include <stdlib.h>

#include "begin_code.h"

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_STD)
# define Malloc(len) AG_Malloc(len)
# define Free(p) AG_Free(p)
# define Realloc(p,len) AG_Realloc((p),(len))
# define Snprintf AG_Snprintf
# define Vsnprintf AG_Vsnprintf
# define Vasprintf(msg, fmt, args) do {				\
	if (AG_Vasprintf((msg),(fmt),(args)) == -1) 		\
		AG_FatalError("Out of memory (vasprintf)");	\
} while (0)
# define Verbose AG_Verbose
# ifdef DEBUG
#  define Debug AG_Debug
# else
#  ifdef __GNUC__
#   define Debug(obj, arg...) ((void)0)
#  else
#   define Debug AG_Debug
#  endif
# endif /* DEBUG */
#endif /* _AGAR_INTERNAL or _USE_AGAR_STD */

__BEGIN_DECLS
extern int agDebugLvl;

void		 AG_InitError(void);
void		 AG_DestroyError(void);
const char	*AG_GetError(void);
void		 AG_SetError(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_FatalError(const char *, ...);
void		 AG_Debug(void *, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
void		 AG_Verbose(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		*AG_PtrMismatch(void);
void		*AG_ObjectMismatch(const char *, const char *);
int		 AG_IntMismatch(void);
float		 AG_FloatMismatch(void);


static __inline__ void *
AG_Malloc(size_t len)
{
	void *p;
	if ((p = malloc(len)) == NULL) { AG_FatalError("malloc"); }
	return (p);
}

static __inline__ void *
AG_Realloc(void *pOld, size_t len)
{
	void *pNew;
	/* XXX redundant on some systems */
	if (pOld == NULL) {
		if ((pNew = malloc(len)) == NULL)
			AG_FatalError("malloc");
	} else {
		if ((pNew = realloc(pOld, len)) == NULL)
			AG_FatalError("realloc");
	}
	return (pNew);
}

#ifdef FREE_NULL_IS_A_NOOP
# define AG_Free(p) free(p)
# undef FREE_NULL_IS_A_NOOP
#else
static __inline__ void AG_Free(void *p) { if (p != NULL) free(p); }
#endif
__END_DECLS

#include "close_code.h"

#endif /* _AGAR_CORE_ERROR_H_ */
