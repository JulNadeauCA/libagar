/*	Public domain	*/

#ifndef _AGAR_CORE_ERROR_PUB_H_
#define _AGAR_CORE_ERROR_PUB_H_

#ifdef _AGAR_INTERNAL
#include <config/free_null_is_a_noop.h>
#else
#include <agar/config/free_null_is_a_noop.h>
#endif

#include "begin_code.h"
__BEGIN_DECLS
void		 AG_InitError(void);
void		 AG_DestroyError(void);
char		*AG_Strdup(const char *);
const char	*AG_GetError(void);
void		 AG_SetError(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_FatalError(const char *, ...);
void		 AG_DebugPrintf(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_Debug(int, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
void		 AG_DebugObj(void *, const char *, ...)
		     NONNULL_ATTRIBUTE(1)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
void		 AG_DebugN(int, const char *, ...)
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
#define AG_Free(p) free(p)
#else
static __inline__ void
AG_Free(void *p)
{
	if (p != NULL)
		free(p);
}
#endif /* FREE_NULL_IS_A_NOOP */

__END_DECLS

#include "close_code.h"
#endif /* _AGAR_CORE_ERROR_PUB_H_ */
